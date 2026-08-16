#define Z_PRINT_DEFINES
#ifdef Z_PRINT_DEFINES
#endif
#include <core/Core.h>

#import <Cocoa/Cocoa.h>

int main(int argc, const char* argv[])
{
    @autoreleasepool
    {
        // RegisterAllScripts is called automatically by Engine::Run(*m_ScriptRegistrar)

        NSApplication* app = [NSApplication sharedApplication];
        id delegate = [[NSClassFromString(@"EngineAppDelegate") alloc] init];
        [app setDelegate:delegate];

        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app activateIgnoringOtherApps:YES];

        [app run];
    }

    return 0;
}
