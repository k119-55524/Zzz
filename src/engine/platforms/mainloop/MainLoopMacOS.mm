#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#include "MainLoopMacOS.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

using namespace zzz::engine;

@interface DisplayLinkTarget_MacOS : NSObject
@property (nonatomic, assign) MainLoopMacOS* loop;
- (void)update:(NSTimer *)timer;
@end

@implementation DisplayLinkTarget_MacOS
- (void)update:(NSTimer *)timer {
    if (self.loop) {
        self.loop->OnUpdate();
    }
}
@end

static NSTimer* g_DisplayTimer = nil;
static DisplayLinkTarget_MacOS* g_DisplayTarget = nil;

MainLoopMacOS::MainLoopMacOS(const Platform& platform, std::function<void()> onUpdate) :
	MainLoopBase(platform, std::move(onUpdate))
{
}

void MainLoopMacOS::Run()
{
    g_DisplayTarget = [[DisplayLinkTarget_MacOS alloc] init];
    g_DisplayTarget.loop = this;
    
    // Using NSTimer targeting 60 FPS (1/60th of a second)
    g_DisplayTimer = [NSTimer scheduledTimerWithTimeInterval:(1.0 / 60.0)
                                                      target:g_DisplayTarget
                                                    selector:@selector(update:)
                                                    userInfo:nil
                                                     repeats:YES];
}

#endif // defined(Z_MACOS)
