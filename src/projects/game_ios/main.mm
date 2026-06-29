#define Z_PRINT_DEFINES
#ifdef Z_PRINT_DEFINES
#endif
#include <common/defines.h>

#import <UIKit/UIKit.h>

int main(int argc, char* argv[])
{
    @autoreleasepool
    {
        extern void RegisterAllScripts();
        RegisterAllScripts();

        return UIApplicationMain(argc, argv, nil, @"EngineAppDelegate");
    }
}
