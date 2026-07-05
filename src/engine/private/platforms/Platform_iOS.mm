#include <common/common.h>

#include "Platform.h"
#include "../../engine.h"
#import <UIKit/UIKit.h>

using namespace zzz::engine;

static std::unique_ptr<Engine> g_Engine;

@interface EngineAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation EngineAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    DOut("[EngineAppDelegate] Application did finish launching. Initializing engine...");
    // UIWindow на iOS должен создаваться в AppDelegate
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];

    // Создание движка
    g_Engine = safe_make_unique<Engine>("GameiOS_ZzzEngine");
    DOut("[EngineAppDelegate] Engine initialized successfully. Starting Run...");
    // Здесь можно было бы прокинуть self.window в NativeAppData или в WiniOS.
    // WiniOS::Initialize создаст ViewController и View, после чего можно выставить rootViewController.
    // Но WiniOS не экспонирует ViewController - пока просто делаем окно key-окном,
    // а WiniOS сам выставит self.window.rootViewController.
    [self.window makeKeyAndVisible];

    auto runResult = g_Engine->Run();
    if (!runResult) {
        DOutError("[EngineAppDelegate] Engine Run failed with error: {}.", runResult.error());
    } else {
        DOut("[EngineAppDelegate] Engine Run succeeded.");
    }

    return YES;
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    DOut("[EngineAppDelegate] Application did become active.");
}

- (void)applicationWillResignActive:(UIApplication *)application {
    DOut("[EngineAppDelegate] Application will resign active.");
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    DOut("[EngineAppDelegate] Application did enter background.");
    if (g_Engine) {
        auto res = g_Engine->GetPlatform()->GetConfigManager()->SaveConfig();
        if (!res) {
            DOutError("[EngineAppDelegate] Failed to save config on entering background: {}.", res.error());
        }
    }
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    DOut("[EngineAppDelegate] Application will enter foreground.");
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    DOut("[EngineAppDelegate] Application did receive memory warning.");
}

- (void)applicationWillTerminate:(UIApplication *)application {
    g_Engine.reset();
}

@end

void Platform::InitializePlatformSpecific()
{
}

void Platform::ShutdownPlatformSpecific()
{
}
