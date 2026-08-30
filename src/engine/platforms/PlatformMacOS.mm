#include "engine/EngineIncludes.h"
#include "engine/Engine.h"
#include "Platform.h"
#import <Cocoa/Cocoa.h>

using namespace zzz::engine;

static std::unique_ptr<Engine> g_Engine;

@interface EngineAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation EngineAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    DOut("[EngineAppDelegate] Приложение завершило запуск. Инициализация движка...");
    g_Engine = safe_make_unique<Engine>();

    DOut("[EngineAppDelegate] Движок инициализирован успешно. Запуск Run...");
    auto runResult = g_Engine->Run();
    if (!runResult) {
        DOutError("[EngineAppDelegate] Run движка завершился ошибкой: {}.", runResult.error());
    } else {
        DOut("[EngineAppDelegate] Run движка выполнен успешно.");
    }
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    return YES;
}

- (void)applicationDidResignActive:(NSNotification *)notification {
    DOut("[EngineAppDelegate] Приложение потеряло активность.");
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

PlatformHardwareState Platform::GatherHardwareState() const
{
	PlatformHardwareState state{};
	state.cpu.architecture = "ARM64";
	return state;
}
