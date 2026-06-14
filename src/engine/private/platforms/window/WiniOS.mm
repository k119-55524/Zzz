#include "WiniOS.h"
#import "iOSView.h"
#include <common/common.h>

using namespace zzz::engine;

@interface EngineViewController : UIViewController
@property (nonatomic, assign) WiniOS* winEngine;
@property (nonatomic, assign) zzz::engine::InputiOS* inputEngine;
@end

@implementation EngineViewController
- (void)viewDidLoad {
    [super viewDidLoad];
    
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    iOSView* metalView = [[iOSView alloc] initWithFrame:self.view.bounds device:device];
    metalView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    metalView.inputEngine = self.inputEngine;
    [self.view addSubview:metalView];
}
@end

WiniOS::WiniOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks))
{
}

WiniOS::~WiniOS()
{
}

std::expected<void, std::string> WiniOS::Initialize(const std::string_view appName)
{
    UIWindow* window = [(id)[[UIApplication sharedApplication] delegate] window];
    if (window != nil) {
        EngineViewController* viewController = [[EngineViewController alloc] init];
        viewController.winEngine = this;
        viewController.inputEngine = m_Input.get();
        window.rootViewController = viewController;
    } else {
        return UNEXPECTED("UIWindow is not initialized on iOS.");
    }
	return {};
}
