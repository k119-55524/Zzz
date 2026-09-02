#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#ifdef __OBJC__

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

namespace zzz::engine {
    class InputMacOS;
}

@interface MacOSView : MTKView
@property (nonatomic, assign) zzz::engine::InputMacOS* inputEngine;
@end

#endif // __OBJC__

#endif // defined(Z_MACOS)

