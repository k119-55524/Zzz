#pragma once

// Зонтик над core/utils/macros/ - чтобы старые #include "core/utils/Macroses.h" не переписывать.
// LogMacros.h - DOut*, GAPILogMacros.h - DOut*GAPI, MiscMacros.h - THROW_RUNTIME/Z_NO_COPY*/CRT_LEAK_CHECK*.
#include "core/utils/macros/LogMacros.h"
#include "core/utils/macros/GAPILogMacros.h"
#include "core/utils/macros/MiscMacros.h"
