#import "ViewController.h"

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@implementation ViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    id<MTLDevice> device = MTLCreateSystemDefaultDevice();

    MTKView* metalView =
        [[MTKView alloc]
            initWithFrame:self.view.bounds
            device:device];

    metalView.autoresizingMask =
        UIViewAutoresizingFlexibleWidth |
        UIViewAutoresizingFlexibleHeight;

    [self.view addSubview:metalView];
}

@end