#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "main.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation AppDelegate

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender
{
    DOut(L">>>>> MacOS game END.");
    
    return YES;
}

@end

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        zzz::engine::Engine engine;

        auto initResult = engine.Initialize();

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
