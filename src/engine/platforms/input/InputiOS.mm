#if Z_IOS

#include "InputiOS.h"
#import "../../window/iOSView.h"
#include <core/utils/Types.h>
#include <core/utils/Ensure.h>
#include <core/utils/Defines.h>
#include <core/utils/Macroses.h>
#include <core/utils/MemoryUtils.h>
#include <core/utils/ThrowWrappers.h>

using namespace zzz::engine;

@implementation iOSView

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    if (self.inputEngine) {
        NativeMsg msg;
        msg.msg = event;
        self.inputEngine->ProcessMessage(msg);
    }
}

// Additional input methods can be added here (e.g. touchesMoved, touchesEnded, etc.)

@end

std::expected<void, std::string> InputiOS::Initialize()
{
	return {};
}

bool InputiOS::ProcessMessage(const NativeMsg& nativeMsg)
{
    // UIEvent* event = (UIEvent*)nativeMsg.msg;
    // Handle the event...
	return false;
}

#endif // Z_IOS
