#include "MainLoop_MacOS.h"
#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>

using namespace zzz::engine;

@interface DisplayLinkTarget_MacOS : NSObject
@property (nonatomic, assign) MainLoop_MacOS* loop;
- (void)update:(NSTimer *)timer;
@end

@implementation DisplayLinkTarget_MacOS
- (void)update:(NSTimer *)timer {
    if (self.loop) {
        self.loop->onUpdateSystem();
    }
}
@end

static NSTimer* g_DisplayTimer = nil;
static DisplayLinkTarget_MacOS* g_DisplayTarget = nil;

MainLoop_MacOS::MainLoop_MacOS(const std::shared_ptr<Platform> platform) :
	MainLoopBase(platform)
{
}

void MainLoop_MacOS::Run()
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
