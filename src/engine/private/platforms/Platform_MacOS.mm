#include <foundation.h>

#include "Platform.h"
#include "../../Engine.h"
#import <Cocoa/Cocoa.h>

using namespace zzz::engine;

static std::unique_ptr<Engine> g_Engine;

@interface EngineAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation EngineAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    g_Engine = std::make_unique<Engine>("GameMacOS_ZzzEngine");
    if (g_Engine->Initialize()) {
        g_Engine->Run();
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    if (g_Engine) {
        g_Engine->OnPlatformApplicationWillResignActive();
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    g_Engine.reset();
}

@end

void Platform::InitializePlatformSpecific()
{
}

void Platform::ShutdownPlatformSpecific()
{
}
