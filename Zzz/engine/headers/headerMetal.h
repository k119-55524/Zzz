#pragma once

#if defined(ZRENDER_API_METAL)
	#if defined(__APPLE__)
		#include <TargetConditionals.h>
		#if defined(__OBJC__) && TARGET_OS_OSX || TARGET_OS_IOS
			#import <Metal/Metal.h>
		#endif // defined(__OBJC__) && TARGET_OS_OSX || TARGET_OS_IOS
	#endif // defined(__APPLE__)
#endif // ZRENDER_API_METAL
