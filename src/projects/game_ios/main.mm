#define Z_PRINT_DEFINES
#ifdef Z_PRINT_DEFINES
#endif
#include <core/Core.h>

#import <UIKit/UIKit.h>

int main(int argc, char* argv[])
{
    @autoreleasepool
    {
        // RegisterAllScripts is called automatically by Engine::Run(*m_ScriptRegistrar)

        return UIApplicationMain(argc, argv, nil, @"EngineAppDelegate");
    }
}
