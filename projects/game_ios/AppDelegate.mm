#import "AppDelegate.h"
#import "ViewController.h"

#include "main.h"

@implementation AppDelegate
{
    std::unique_ptr<zzz::engine::Engine> _engine;
}

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions
{
    _engine = std::make_unique<zzz::engine::Engine>();

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

@end