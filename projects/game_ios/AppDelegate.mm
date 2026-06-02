#import "AppDelegate.h"
#import "ViewController.h"

#include <memory>
#include <iostream>

#include <engine.h>
#include <logger.h>
#include <foundation.h>

@implementation AppDelegate
{
    std::unique_ptr<zzz::engine::Engine> _engine;
}

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    _engine = std::make_unique<zzz::engine::Engine>("GameiOS_ZzzEngine");
    auto initResult = _engine->Initialize();
    if (!initResult.has_value())
    {
        return NO;
    }

    self.window =
        [[UIWindow alloc]
            initWithFrame:[UIScreen mainScreen].bounds];

    ViewController* viewController =
        [[ViewController alloc] init];

    self.window.rootViewController = viewController;

    [self.window makeKeyAndVisible];

    return YES;
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    if (_engine)
    {
        auto res = _engine->OnAppMinimize();
        if (!res)
        {
            DOutFatal("Failed to save config on entering background: {}.", res.error());
        }
    }
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    if (_engine)
    {
        auto res = _engine->OnAppMinimize();
        if (!res)
        {
            DOutFatal("Failed to save config on termination: {}.", res.error());
        }
    }
}

@end
