# common_lib

Эта библиотека предоставляет базовые типы данных, макросы, утилиты для управления памятью и проверки утверждений (assertions).

## Использование

Для использования библиотеки добавьте её в зависимости вашего модуля в `CMakeLists.txt`:
```cmake
target_link_libraries(your_target PUBLIC common_lib)
```

В коде подключайте нужные заголовочные файлы с префиксом `common/`:
```cpp
#include <common/types.h>
#include <common/ensure.h>
#include <common/core_macros.h>
```

Базовые сущности находятся в пространстве имён `zzz::common`.
