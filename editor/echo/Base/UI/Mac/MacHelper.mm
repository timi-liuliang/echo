#include <QWidget>
#include <AppKit/AppKit.h>

extern "C" void macChangeTitleBarColor(WId winId, double red, double green, double blue)
{
    if (winId != 0)
    {
        NSView* view = (NSView*)winId;
        NSWindow* window = [view window];
        window.titlebarAppearsTransparent = YES;
        window.backgroundColor = [NSColor colorWithRed:red green:green blue:blue alpha:1.];
    }
}
