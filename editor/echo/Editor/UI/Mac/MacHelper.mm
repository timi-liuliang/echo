#include <QWidget>
#include <AppKit/AppKit.h>

extern "C" void macChangeTitleBarColor(WId winId, double red, double green, double blue)
{
    if (winId != 0)
    {
        NSView* view = (NSView*)winId;
        NSWindow* window = [view window];
        window.titlebarAppearsTransparent = true;
        [window setBackgroundColor:[NSColor colorWithSRGBRed:red green:green blue:blue alpha:1.0]];
    }
}
