#pragma once

#include "core/utils/Defines.h"

#if defined(Z_IOS)

#ifdef __OBJC__

#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>

namespace zzz::engine {
    class InputiOS;
}

@interface iOSView : MTKView
@property (nonatomic, assign) zzz::engine::InputiOS* inputEngine;
@end

#endif // __OBJC__

#endif // defined(Z_IOS)

