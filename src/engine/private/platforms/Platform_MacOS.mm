#include <common/common.h>

#include "Platform.h"
#include "../../engine.h"
#import <Cocoa/Cocoa.h>

using namespace zzz::engine;

static std::unique_ptr<Engine> g_Engine;

@interface EngineAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation EngineAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    DOut("[EngineAppDelegate] Application did finish launching. Initializing engine...");
    g_Engine = safe_make_unique<Engine>("GameMacOS_ZzzEngine");

    DOut("[EngineAppDelegate] Engine initialized successfully. Starting Run...");
    auto runResult = g_Engine->Run();
    if (!runResult) {
        DOutError("[EngineAppDelegate] Engine Run failed with error: {}.", runResult.error());
    } else {
        DOut("[EngineAppDelegate] Engine Run succeeded.");
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    DOut("[EngineAppDelegate] Application did resign active.");
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
