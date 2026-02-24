#pragma once

#ifdef _DEBUG

#define CRT_LEAK_CHECK_BEGIN(...) \
	debug::crt::LeakContext _crtLeakCtx{}; \
	debug::crt::Begin(_crtLeakCtx, __VA_ARGS__)

#define CRT_LEAK_CHECK_END() \
	debug::crt::End(_crtLeakCtx)

#else

#define CRT_LEAK_CHECK_BEGIN(...)
#define CRT_LEAK_CHECK_END() 0

#endif