#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "main.h"

static std::unique_ptr<zzz::engine::Engine> g_Engine;

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    DOut(">>>>> MacOS game END.");
    
    return YES;
}

- (void)applicationDidResignActive:(NSNotification *)notification
{
    if (g_Engine)
    {
        auto res = g_Engine->OnAppMinimize();
        if (!res)
        {
            DOutCritical("Failed to save config on deactivation: {}.", res.error());
        }
    }
}

- (void)applicationWillTerminate:(NSNotification *)notification
{
    g_Engine.reset();
}

@end

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        g_Engine = std::make_unique<zzz::engine::Engine>("GameMacOS_ZzzEngine");
        auto initResult = g_Engine->Initialize();
        if (!initResult.has_value())
        {
            return -1;
        }

        NSApplication* app = [NSApplication sharedApplication];

        AppDelegate* delegate = [[AppDelegate alloc] init];
        [app setDelegate:delegate];

        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app activateIgnoringOtherApps:YES];

        NSRect frame = NSMakeRect(0, 0, 1280, 720);

        NSUInteger style =
            NSWindowStyleMaskTitled |
            NSWindowStyleMaskClosable |
            NSWindowStyleMaskResizable;

        NSWindow* window =
            [[NSWindow alloc]
                initWithContentRect:frame
                styleMask:style
                backing:NSBackingStoreBuffered
                defer:NO];

        [window setTitle:@"Zzz"];
        [window setReleasedWhenClosed:NO];
        [window makeKeyAndOrderFront:nil];

        id<MTLDevice> device = MTLCreateSystemDefaultDevice();

        MTKView* metalView =
            [[MTKView alloc]
                initWithFrame:frame
                device:device];

        [window setContentView:metalView];

        [app run];
    }

    return 0;
}
