#include <engine/header.h>

#include "Platform.h"
#include "../engine.h"
#import <UIKit/UIKit.h>

using namespace zzz::engine;

static std::unique_ptr<Engine> g_Engine;

@interface EngineAppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation EngineAppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    DOut("[EngineAppDelegate] Приложение завершило запуск. Инициализация движка...");
    // UIWindow на iOS должен создаваться в AppDelegate
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];

    // Создание движка
    g_Engine = safe_make_unique<Engine>("GameiOS_ZzzEngine");
    DOut("[EngineAppDelegate] Движок инициализирован успешно. Запуск Run...");
    // Здесь можно было бы прокинуть self.window в NativeAppData или в WiniOS.
    // WiniOS::Initialize создаст ViewController и View, после чего можно выставить rootViewController.
    // Но WiniOS не экспонирует ViewController - пока просто делаем окно key-окном,
    // а WiniOS сам выставит self.window.rootViewController.
    [self.window makeKeyAndVisible];

    auto runResult = g_Engine->Run();
    if (!runResult) {
        DOutError("[EngineAppDelegate] Run движка завершился ошибкой: {}.", runResult.error());
    } else {
        DOut("[EngineAppDelegate] Run движка выполнен успешно.");
    }

    return YES;
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    DOut("[EngineAppDelegate] Приложение стало активным.");
}

- (void)applicationWillResignActive:(UIApplication *)application {
    DOut("[EngineAppDelegate] Приложение теряет активность.");
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    DOut("[EngineAppDelegate] Приложение перешло в фон.");
    if (g_Engine) {
        auto res = g_Engine->GetPlatform()->GetUserSettingsManager()->SaveConfig();
        if (!res) {
            DOutError("[EngineAppDelegate] Не удалось сохранить конфигурацию при переходе в фон: {}.", res.error());
        }
    }
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    DOut("[EngineAppDelegate] Приложение возвращается на передний план.");
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    DOut("[EngineAppDelegate] Приложение получило предупреждение о нехватке памяти.");
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
