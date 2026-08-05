#if Z_MACOS

#include "InputMacOS.h"
#import "../../window/MacOSView.h"
#include <engine/EngineIncludes.h>

using namespace zzz::engine;

@implementation MacOSView

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)mouseDown:(NSEvent *)event {
    if (self.inputEngine) {
        NativeMsg msg;
        msg.msg = event;
        self.inputEngine->ProcessMessage(msg);
    }
}

// Additional input methods can be added here (e.g. mouseUp, keyDown, etc.)

@end

std::expected<void, std::string> InputMacOS::Initialize()
{
	return {};
}

bool InputMacOS::ProcessMessage(const NativeMsg& nativeMsg)
{
    // NSEvent* event = (NSEvent*)nativeMsg.msg;
    // Handle the event...
	return false;
}

#endif // Z_MACOS
