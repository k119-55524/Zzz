#include "MainLoop_iOS.h"
#import <QuartzCore/QuartzCore.h>
#import <Foundation/Foundation.h>

using namespace zzz::engine;

@interface DisplayLinkTarget_iOS : NSObject
@property (nonatomic, assign) MainLoop_iOS* loop;
- (void)update:(CADisplayLink *)displayLink;
@end

@implementation DisplayLinkTarget_iOS
- (void)update:(CADisplayLink *)displayLink {
    if (self.loop) {
        self.loop->onUpdateSystem();
    }
}
@end

static CADisplayLink* g_DisplayLink = nil;
static DisplayLinkTarget_iOS* g_DisplayTarget = nil;

MainLoop_iOS::MainLoop_iOS(const std::shared_ptr<Platform> platform) :
	MainLoopBase(platform)
{
}

void MainLoop_iOS::Run()
{
    g_DisplayTarget = [[DisplayLinkTarget_iOS alloc] init];
    g_DisplayTarget.loop = this;
    
    g_DisplayLink = [CADisplayLink displayLinkWithTarget:g_DisplayTarget selector:@selector(update:)];
    [g_DisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}
