#pragma once

#include "core/CoreIncludes.h"

#include "core/utils/Types.h"
#include "core/utils/Export.h"
#include "core/serialize/Serializer.h"
#include "core/io/FileHeader.h"
#include "core/utils/Constants.h"
#include "core/utils/Defines.h"
#include "core/utils/Ensure.h"
#include "core/utils/Guid.h"
#include "core/utils/Macroses.h"
#include "core/utils/MemoryUtils.h"
#include "core/utils/Version.h"
#include "core/utils/Converters.h"

#include "core/enums/ePackage.h"
#include "core/enums/eTargetPlatform.h"
#include "core/enums/eGAPIType.h"
#include "core/enums/eWindowState.h"
#include "core/enums/eLogMessageType.h"
#include "core/utils/LogCategory.h"
#include "core/enums/eEnumToString.h"
#include "core/enums/eWinResize.h"
#include "core/enums/platforms/ConverterMSWinTypes.h"
#include "core/enums/platforms/ConverterAndroidTypes.h"
#include "core/enums/platforms/ConverteriOSTypes.h"
#include "core/enums/platforms/ConverterLinuxTypes.h"
#include "core/enums/platforms/ConverterMacOSTypes.h"


#include "core/templates/DoubleBufferedVector.h"
#include "core/templates/QueueArray.h"
#include "core/templates/ThreadSafeQueueArray.h"
#include "core/templates/ThreadSafeSwapBuffer.h"
#include "core/templates/ThreadPool.h"

#include "core/time/Time.h"
#include "core/events/Event.h"
#include "core/events/EventBus.h"

#include "core/userscripts/ScriptStorage.h"
#include "core/userscripts/ScriptFactory.h"
#include "core/userscripts/ScriptRegistry.h"
#include "core/userscripts/base_script/BaseScript.h"
#include "core/userscripts/base_script/Script.h"
#include "core/userscripts/base_script/GameScript.h"
#include "core/userscripts/base_script/SceneScript.h"
#include "core/userscripts/base_script/ViewScript.h"

#include "core/io/Path.h"
// Узкие типы данных пакета (PackageHeader, PrimaryViewData, ChildViewData и т.д.) и hardware/*
// намеренно НЕ включаются сюда: они используются только в единичных местах (PackageManager,
// UserSettingsManager, ViewManager, Platform и т.п.), а не по всему проекту. Раньше их бланкетное
// подключение здесь заставляло core_lib/engine_lib пересобираться целиком при правке любого из ~30
// файлов, хотя большинство потребителей Core.h эти типы вообще не используют. Конкретные потребители
// подключают нужные заголовки самостоятельно (Include What You Use).
#include "core/io/package/platforms/start_view/ViewPlatformConfig.h"
#include "core/io/package/platforms/project/ProjectPlatformConfig.h"

using namespace zzz::math;
using namespace zzz::core;

