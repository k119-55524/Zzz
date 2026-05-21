#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

int main(int argc, const char* argv[])
{
	@autoreleasepool
	{
		NSApplication * app = [NSApplication sharedApplication];

		NSRect frame = NSMakeRect(0, 0, 1280, 720);

		NSUInteger style =
			NSWindowStyleMaskTitled |
			NSWindowStyleMaskClosable |
			NSWindowStyleMaskResizable;

		NSWindow* window = [[NSWindow alloc]
			initWithContentRect:frame
			styleMask : style
			backing : NSBackingStoreBuffered
			defer : NO];

		[window setTitle:@"Zzz"] ;
		[window makeKeyAndOrderFront:nil] ;

		id<MTLDevice> device = MTLCreateSystemDefaultDevice();

		MTKView* metalView =
			[[MTKView alloc]initWithFrame:frame device : device];

		[window setContentView:metalView] ;

		[app run] ;
	}

	return 0;
}