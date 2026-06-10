#import <Cocoa/Cocoa.h>

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        NSApplication* app = [NSApplication sharedApplication];
        id delegate = [[NSClassFromString(@"EngineAppDelegate") alloc] init];
        [app setDelegate:delegate];

        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app activateIgnoringOtherApps:YES];

        [app run];
    }

    return 0;
}
