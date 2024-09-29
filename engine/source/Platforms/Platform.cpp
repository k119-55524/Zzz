#include "pch.h"
#include "Platform.h"

using namespace Zzz;
using namespace Zzz::Platforms;

Platform::Platform(unique_ptr<MessageBoxBase> mb) :
	messageBox{move(mb)}
{
#if defined(_DEBUG)
	assert(messageBox.get() != nullptr);
#endif
}
