#include "GpuResourceManager.h"
#include "engine/resources/cpu/CpuResourceManager.h"
#include "engine/gapi/GAPI.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/Ensure.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	GpuResourceManager::GpuResourceManager(
		std::shared_ptr<GAPI> gapi,
		std::shared_ptr<CpuResourceManager> cpuResourceManager)
		: m_GAPI(std::move(gapi))
		, m_CpuManager(std::move(cpuResourceManager))
	{
		ensure(m_CpuManager != nullptr, "CpuResourceManager не должен быть null в GpuResourceManager.");
	}

	void GpuResourceManager::Update()
	{
		m_MainThreadQueue.ExecuteAll();
	}

	std::shared_ptr<GpuMesh> GpuResourceManager::CreateGpuMesh(std::shared_ptr<CpuMesh> cpuMesh)
	{
		if (!cpuMesh) return nullptr;
		// На этапе 22 - сохранение ссылки на CPU-меш. На этапе 23 - создание аппаратных Vertex/Index буферов GAPI.
		return safe_make_shared<GpuMesh>(cpuMesh->GetGuid(), std::string(cpuMesh->GetName()), std::move(cpuMesh));
	}

	std::shared_ptr<GpuMaterial> GpuResourceManager::CreateGpuMaterial(std::shared_ptr<CpuMaterial> cpuMaterial)
	{
		if (!cpuMaterial) return nullptr;
		return safe_make_shared<GpuMaterial>(cpuMaterial->GetGuid(), std::string(cpuMaterial->GetName()), std::move(cpuMaterial));
	}

	std::shared_ptr<GpuTexture2D> GpuResourceManager::CreateGpuTexture(std::shared_ptr<CpuTexture2D> cpuTexture)
	{
		if (!cpuTexture) return nullptr;
		return safe_make_shared<GpuTexture2D>(cpuTexture->GetGuid(), std::string(cpuTexture->GetName()), std::move(cpuTexture));
	}

	std::shared_ptr<GpuShader> GpuResourceManager::CreateGpuShader(std::shared_ptr<CpuShader> cpuShader)
	{
		if (!cpuShader) return nullptr;
		return safe_make_shared<GpuShader>(cpuShader->GetGuid(), std::string(cpuShader->GetName()), std::move(cpuShader));
	}

	void GpuResourceManager::RequestCpuMesh(
		const Guid& guid,
		std::weak_ptr<void> ctx,
		std::function<void(std::expected<std::shared_ptr<CpuMesh>, std::string>)> cb)
	{
		m_CpuManager->LoadMeshAsync(guid, std::move(ctx), std::move(cb));
	}

	void GpuResourceManager::RequestCpuMaterial(
		const Guid& guid,
		std::weak_ptr<void> ctx,
		std::function<void(std::expected<std::shared_ptr<CpuMaterial>, std::string>)> cb)
	{
		m_CpuManager->LoadMaterialAsync(guid, std::move(ctx), std::move(cb));
	}

	void GpuResourceManager::RequestCpuTexture(
		const Guid& guid,
		std::weak_ptr<void> ctx,
		std::function<void(std::expected<std::shared_ptr<CpuTexture2D>, std::string>)> cb)
	{
		m_CpuManager->LoadTextureAsync(guid, std::move(ctx), std::move(cb));
	}

	void GpuResourceManager::RequestCpuShader(
		const Guid& guid,
		std::weak_ptr<void> ctx,
		std::function<void(std::expected<std::shared_ptr<CpuShader>, std::string>)> cb)
	{
		m_CpuManager->LoadShaderAsync(guid, std::move(ctx), std::move(cb));
	}

}
