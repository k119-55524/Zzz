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

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    if (_engine)
        _engine->OnPlatformApplicationDidBecomeActive();
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    if (_engine)
        _engine->OnPlatformApplicationWillResignActive();
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    if (_engine)
        _engine->OnPlatformApplicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    if (_engine)
        _engine->OnPlatformApplicationWillEnterForeground();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application
{
    if (_engine)
        _engine->OnPlatformApplicationDidReceiveMemoryWarning();
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    _engine.reset();
}

@end
