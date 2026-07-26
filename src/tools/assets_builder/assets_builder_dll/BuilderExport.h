#pragma once

#if defined(_WIN32)
	#if defined(ASSETS_BUILDER_EXPORTS)
		#define BUILDER_API __declspec(dllexport)
	#else
		#define BUILDER_API __declspec(dllimport)
	#endif
#else
	#define BUILDER_API
#endif
