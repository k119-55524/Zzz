# Этап 25. GPU-буферы, асинхронная загрузка в GPU и полноценный GpuMesh

**Статус:** ⏳ В процессе (активный этап разработки)

---

## 🎯 Цель этапа

Довести цепочку `mapped archive → CpuMesh → GpuMesh` до реального размещения vertex/index данных в видеопамяти. `GpuMesh` становится готовым только после завершения GPU copy-команд, подтверждённого fence. Произвольные воркеры `TaskDispatcher` не обращаются напрямую к DX12/Vulkan.

Целевые backend’ы этапа — **DirectX 12 и Vulkan на Windows**. Metal сохраняет компилируемую заглушку до этапа соответствующей платформы. Draw/Pipeline State, шейдеры и материалы не входят в этап 25.

## Исходное состояние

- `CpuMesh` уже содержит vertex/index bytes, stride, count и `eIndexFormat`.
- `GpuMesh_DX` и `GpuMesh_VK` хранят только GUID, имя и количества элементов; настоящих GPU-буферов нет.
- `GpuResourceManager` получает `GAPI`, но не использует его при создании GPU-ресурса.
- `CreateGpuResourceAndUploadFromCpu` синхронно создаёт C++-заглушку, после чего `ResourceTable` немедленно разрешается как Ready.
- DX12 уже имеет direct queue и fence; Vulkan имеет graphics queue с централизованной внешней синхронизацией `QueueSubmit`.

## Архитектурные контракты

1. **Истинный GpuReady.** `ResourceTable<GpuMesh>::Resolve(success)` вызывается только после подтверждённого выполнения copy-команд.
2. **Один владелец upload-контекста.** Command allocator/list DX12 и command pool/buffer Vulkan не разделяются между произвольными worker-потоками. Ими управляет `GpuUploadScheduler`.
3. **Асинхронность для вызывающего кода.** CPU-десериализация остаётся в `TaskDispatcher`; GPU upload ставится в специализированную очередь и не блокирует игровой поток.
4. **Внешняя синхронизация очередей.** Все Vulkan submit проходят через синхронизированный API; DX12 submission и fence-value выдаются централизованно.
5. **Время жизни данных.** `CpuMesh` и staging-ресурсы удерживаются до безопасного завершения upload. После fence CPU bulk data освобождаются, если других ссылок на `CpuMesh` нет.
6. **No-Hang.** Ошибка создания buffer/staging, записи команд, submit или shutdown обязательно разрешает ожидающий запрос через `std::unexpected`.
7. **Детерминированный shutdown.** Новые upload-запросы прекращаются, in-flight submissions завершаются либо переводятся в ошибку, затем освобождаются staging и device-local ресурсы.
8. **Границы этапа.** Hot reload, device-loss restore, общий GPU allocator, defragmentation, textures, descriptors и draw-команды не входят в этот этап.

## 🛠️ План работ

### 1. Общие контракты GPU-буферов

- [ ] Ввести типизированное назначение buffer (`Vertex`, `Index`, `Staging`) и проверяемый размер в байтах.
- [ ] Добавить платформенно выбираемый RAII `GPUBuffer` с backend-реализациями DX12/Vulkan и безопасным освобождением native handle/memory.
- [ ] Зафиксировать неизменяемость vertex/index buffer после завершения upload.
- [ ] Добавить в Vulkan debug-object mapping тип `VkBuffer`; назначать debug names обоим backend’ам.

### 2. GpuUploadScheduler

- [ ] Создать специализированный `GpuUploadScheduler`, принимающий upload-запросы независимо от `TaskDispatcher`.
- [ ] Определить пакет запроса, который удерживает источник данных и completion callback до завершения fence.
- [ ] Реализовать состояния `Accepting → Stopping → Stopped`, запрет новых запросов после начала shutdown и гарантированное разрешение каждого принятого запроса.
- [ ] Ограничить объём in-flight staging memory; не создавать неограниченную очередь больших CPU-копий.
- [ ] Не исполнять пользовательский callback под внутренними mutex scheduler’а.

### 3. DirectX 12 backend

- [ ] Создать default-heap vertex/index buffers и upload-heap staging resources.
- [ ] Записывать `CopyBufferRegion` и переходы `COPY_DEST → VERTEX_AND_CONSTANT_BUFFER` / `INDEX_BUFFER`.
- [ ] Централизовать command allocator/list и submission; не использовать один allocator одновременно несколькими потоками.
- [ ] Удерживать upload resources до достижения fence value и освобождать их после completion.
- [ ] Сформировать валидные `D3D12_VERTEX_BUFFER_VIEW` и `D3D12_INDEX_BUFFER_VIEW` в `GpuMesh_DX`.

### 4. Vulkan backend

- [ ] Создать device-local vertex/index `VkBuffer` и выделить/bind memory подходящего memory type.
- [ ] Создать host-visible staging buffer, скопировать CPU bytes и записать `vkCmdCopyBuffer`.
- [ ] Добавить buffer memory barriers до `VERTEX_ATTRIBUTE_READ` / `INDEX_READ`.
- [ ] Управлять command pool/buffer с корректной внешней синхронизацией и выполнять submit через `VulkanAPI::QueueSubmit`.
- [ ] Удерживать staging buffer/memory до fence completion и затем освобождать.

### 5. Интеграция ресурсов

- [ ] Заменить синхронный `CreateGpuResourceAndUploadFromCpu` на асинхронный контракт через `GpuUploadScheduler`.
- [ ] Передать scheduler/GAPI в путь создания `GpuMesh`, не использовать глобальные device/queue.
- [ ] Хранить в `GpuMesh_DX` / `GpuMesh_VK` реальные vertex/index buffers, stride, count и index format.
- [ ] Разрешать `ResourceTable<GpuMesh>` только из completion upload; ошибки пробрасывать без создания готовой заглушки.
- [ ] Убедиться, что последняя ссылка на `CpuMesh` освобождается после завершения upload, а не до копирования данных.
- [ ] Оставить Material/Texture/Shader на существующих заглушках до следующих этапов, не выдавая их за реализованный GPU upload.

### 6. Жизненный цикл и верификация

- [ ] Встроить остановку `GpuUploadScheduler` в `Engine::Shutdown` перед `GAPI::WaitForGpu` и уничтожением GAPI.
- [ ] Проверить shutdown при queued и in-flight upload без зависания, use-after-free и callback после уничтожения владельца.
- [ ] Собрать Windows DX12 и Windows Vulkan конфигурации с включёнными debug/validation layers.
- [ ] Проверить загрузку непустого mesh: размеры buffer, stride, index format/count и достижение completion fence.
- [ ] Проверить ошибочные входы: пустые vertex/index bytes, переполнение размеров, отказ выделения и ошибка submit.
- [ ] Обновить `docs/ARCHITECTURE.md` и `general_plan.md` по фактически реализованной схеме.

## Критерии приёмки

1. `GpuMesh` содержит реальные native vertex/index buffers и корректные метаданные для будущего DrawIndexed.
2. Callback успешной загрузки не вызывается до fence completion.
3. Ни один worker `TaskDispatcher` не использует несинхронизированные command allocator/list, command pool/buffer или queue.
4. Staging и `CpuMesh` живут достаточно долго и освобождаются после завершения upload.
5. Любой принятый запрос завершается успехом или `std::unexpected`, включая shutdown и ошибки GAPI.
6. DX12 и Vulkan validation/debug layers не сообщают об ошибках времени жизни, барьеров или внешней синхронизации.

