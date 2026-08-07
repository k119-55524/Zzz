#include "WinMacOS.h"
#import "MacOSView.h"

using namespace zzz::engine;

@interface EngineWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) WinMacOS* winEngine;
@end

@implementation EngineWindowDelegate
- (BOOL)windowShouldClose:(NSWindow *)sender {
    if (self.winEngine) {
        // [macOS] Вызывается при нажатии на красную кнопку закрытия окна.
        // Окно запрашивает у нас разрешение на закрытие. Передаем сигнал в движок.
        VERIFY_AND_CALL(self.winEngine->m_Callbacks.OnClose);
    }
    return YES;
}
@end

WinMacOS::WinMacOS(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks) :
	WindowBase(platform, input, std::move(callbacks))
{
}

WinMacOS::~WinMacOS()
{
}

std::expected<void, std::string> WinMacOS::Initialize(const StartViewPlatformData& settings)
{
    NSRect frame = NSMakeRect(0, 0, settings.GetSize().width, settings.GetSize().height);

    NSUInteger style =
        NSWindowStyleMaskTitled |
        NSWindowStyleMaskClosable |
        NSWindowStyleMaskResizable;

    NSWindow* window = [[NSWindow alloc]
        initWithContentRect:frame
        styleMask:style
        backing:NSBackingStoreBuffered
        defer:NO];

    [window setTitle:[NSString stringWithUTF8String:settings.GetTitle().c_str()]];
    [window setReleasedWhenClosed:NO];

    EngineWindowDelegate* delegate = [[EngineWindowDelegate alloc] init];
    delegate.winEngine = this;
    [window setDelegate:delegate];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    MacOSView* metalView = [[MacOSView alloc] initWithFrame:frame device:device];
    
    // We would assign inputEngine here if we had access to InputMacOS natively,
    // but Input is stored as std::shared_ptr<Input> m_Input in WindowBase.
    // For now, assume InputMacOS can be casted.
    // metalView.inputEngine = static_cast<InputMacOS*>(m_Input.get());

    [window setContentView:metalView];
    [window makeKeyAndOrderFront:nil];

	return {};
}
