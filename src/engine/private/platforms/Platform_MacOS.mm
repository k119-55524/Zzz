#include <foundation.h>

#include "Platform.h"
#include "../../engine.h"
#import <Cocoa/Cocoa.h>
#import <iostream>

using namespace zzz::engine;

static std::unique_ptr<Engine> g_Engine;

@interface EngineAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation EngineAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    std::cerr << "[EngineAppDelegate] Application did finish launching. Initializing engine..." << std::endl;
    g_Engine = std::make_unique<Engine>("GameMacOS_ZzzEngine");
    
    auto initResult = g_Engine->Initialize();
    if (initResult) {
        std::cerr << "[EngineAppDelegate] Engine initialized successfully. Starting Run..." << std::endl;
        auto runResult = g_Engine->Run();
        if (!runResult) {
            std::cerr << "[EngineAppDelegate] Engine Run failed with error: " << runResult.error() << std::endl;
        } else {
            std::cerr << "[EngineAppDelegate] Engine Run succeeded." << std::endl;
        }
    } else {
        std::cerr << "[EngineAppDelegate] Engine initialization failed with error: " << initResult.error() << std::endl;
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    std::cerr << "[EngineAppDelegate] Application did resign active." << std::endl;
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
