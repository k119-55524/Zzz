#include "pch.h"
#include "Platform.h"

using namespace Zzz;
using namespace Zzz::Platforms;

Platform::Platform(unique_ptr<ISysMB> mb) :
	messageBox{move(mb)}
{
#ifdef _DEBUG
	assert(messageBox.get() != nullptr);
#endif	// _DEBUG
}
