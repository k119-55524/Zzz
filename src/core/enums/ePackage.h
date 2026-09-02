#pragma once

#include <string_view>
#include "core/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	enum class ePackage : zU32
	{
		ProjectManifest = 1,
		Scene           = 2,
		PrimaryView     = 3,
		ChildView       = 4,
		IndependentView = 5,
		Prefab          = 6,
		BinaryAsset     = 7
	};

	constexpr std::string_view ToString(ePackage type)
	{
		switch (type)
		{
		case ePackage::ProjectManifest: return "ProjectManifest";
		case ePackage::Scene:           return "Scene";
		case ePackage::PrimaryView:     return "PrimaryView";
		case ePackage::ChildView:       return "ChildView";
		case ePackage::IndependentView: return "IndependentView";
		case ePackage::Prefab:          return "Prefab";
		case ePackage::BinaryAsset:     return "BinaryAsset";
		}
		THROW_RUNTIME("Необработанный ePackage");
	}
}
