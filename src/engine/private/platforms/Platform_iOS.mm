#include <foundation.h>

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
    NSLog(@"[EngineAppDelegate] Application did finish launching. Initializing engine...");
    // UIWindow must be created by AppDelegate in iOS
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    
    // Engine creation
    g_Engine = std::make_unique<Engine>("GameiOS_ZzzEngine");
    auto initResult = g_Engine->Initialize();
    if (initResult) {
        NSLog(@"[EngineAppDelegate] Engine initialized successfully. Starting Run...");
        // Here we could inject the self.window into NativeAppData or pass it to WiniOS
        // WiniOS::Initialize will create the ViewController and View. 
        // Then we can set the rootViewController.
        // Wait, WiniOS does not expose ViewController. We can just set a dummy rootViewController for now,
        // or assume WiniOS sets the self.window.rootViewController.
        // Actually, let's just make WiniOS set up its own ViewController and we can fetch it, 
        // or WiniOS sets it to key window. For now, just make key window.
        [self.window makeKeyAndVisible];
        
        auto runResult = g_Engine->Run();
        if (!runResult) {
            NSLog(@"[EngineAppDelegate] Engine Run failed with error: %s", runResult.error().c_str());
        } else {
            NSLog(@"[EngineAppDelegate] Engine Run succeeded.");
        }
    } else {
        NSLog(@"[EngineAppDelegate] Engine initialization failed with error: %s", initResult.error().c_str());
    }
    
    return YES;
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    if (g_Engine) g_Engine->OnPlatformApplicationDidBecomeActive();
}

- (void)applicationWillResignActive:(UIApplication *)application {
    if (g_Engine) g_Engine->OnPlatformApplicationWillResignActive();
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    if (g_Engine) g_Engine->OnPlatformApplicationDidEnterBackground();
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    if (g_Engine) g_Engine->OnPlatformApplicationWillEnterForeground();
}

- (void)applicationDidReceiveMemoryWarning:(UIApplication *)application {
    if (g_Engine) g_Engine->OnPlatformApplicationDidReceiveMemoryWarning();
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
