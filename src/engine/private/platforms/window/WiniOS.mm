#include "WiniOS.h"
#import "iOSView.h"

using namespace zzz::engine;

@interface EngineViewController : UIViewController
@property (nonatomic, assign) WiniOS* winEngine;
@end

@implementation EngineViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    iOSView* metalView = [[iOSView alloc] initWithFrame:self.view.bounds device:device];
    metalView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:metalView];
}
@end

WiniOS::WiniOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input) :
	WindowBase(platform, input)
{
}

WiniOS::~WiniOS()
{
}

std::expected<void, std::string> WiniOS::Initialize(const std::string_view appName)
{
    // The UIWindow should be created by AppDelegate, but we can also manage the rootViewController here.
    // Wait, on iOS, the UIWindow is typically bound to the screen. 
    // Usually AppDelegate passes the UIWindow through NativeAppData.
    // Let's assume NativeAppData holds the UIWindow, or we just rely on AppDelegate to set the rootViewController.
    
    // For now, we just return success. EngineViewController will be used by Platform_iOS.mm.
	return {};
}
