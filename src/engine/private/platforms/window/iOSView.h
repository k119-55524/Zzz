#pragma once

#ifdef __OBJC__

#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>

namespace zzz::engine {
    class InputiOS;
}

@interface iOSView : MTKView
@property (nonatomic, assign) zzz::engine::InputiOS* inputEngine;
@end

#endif
