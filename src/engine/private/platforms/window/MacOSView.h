#pragma once

#ifdef __OBJC__

#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>

namespace zzz::engine {
    class InputMacOS;
}

@interface MacOSView : MTKView
@property (nonatomic, assign) zzz::engine::InputMacOS* inputEngine;
@end

#endif
