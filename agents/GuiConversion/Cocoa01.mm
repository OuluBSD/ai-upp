#ifdef HAVE_COCOA
#import <Cocoa/Cocoa.h>

@interface HelloAppDelegate : NSObject <NSApplicationDelegate>
@end

@implementation HelloAppDelegate
- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender {
    return YES;
}
@end

void Cocoa01(int test_num) {
    if (test_num == 0 || test_num < 0) {
        @autoreleasepool {
            [NSApplication sharedApplication];
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

            NSRect frame = NSMakeRect(100, 100, 320, 240);
            NSWindow *window = [[NSWindow alloc] initWithContentRect:frame
                                                            styleMask:(NSWindowStyleMaskTitled |
                                                                       NSWindowStyleMaskClosable |
                                                                       NSWindowStyleMaskMiniaturizable)
                                                              backing:NSBackingStoreBuffered
                                                                defer:NO];
            [window setTitle:@"Hello World program"];

            NSTextField *label = [[NSTextField alloc] initWithFrame:[[window contentView] bounds]];
            [label setStringValue:@"Hello world!"];
            [label setBezeled:NO];
            [label setDrawsBackground:NO];
            [label setEditable:NO];
            [label setSelectable:NO];
            [label setAlignment:NSTextAlignmentCenter];
            [label setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
            [[window contentView] addSubview:label];

            [window center];
            [window makeKeyAndOrderFront:nil];

            HelloAppDelegate *delegate = [[HelloAppDelegate alloc] init];
            [NSApp setDelegate:delegate];
            [NSApp activateIgnoringOtherApps:YES];

            [NSApp run];

            // keep objects alive until after the run loop ends
            (void)delegate; (void)window; (void)label;
        }
    }
}
#else
void Cocoa01(int) {}
#endif
