#include <QWidget>
#include <AppKit/AppKit.h>
#include "MacHelper.h"

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

// ======================================================
// Adjust specific Widget children according to O/S
// => Set Buttons height
// => Set labels font size
// ======================================================
void adjustWidgetSizeByOS(QWidget *rootWidget)
{
    if (rootWidget == NULL)
        return;
    
    QObject *child = NULL;
    QObjectList Containers;
    QObject *container  = NULL;
    QStringList DoNotAffect;
    
    // Make an exception list (Objects not to be affected)
    DoNotAffect.append("aboutTitleLabel");     // about Dialog
    DoNotAffect.append("aboutVersionLabel");   // about Dialog
    DoNotAffect.append("aboutCopyrightLabel"); // about Dialog
    DoNotAffect.append("aboutUrlLabel");       // about Dialog
    DoNotAffect.append("aboutLicenseLabel");   // about Dialog
    
    // Set sizes according to OS:
#ifdef Q_OS_MAC
    int ButtonHeight = 32;
    int LabelsFontSize = 12;
#else // Win XP/7
    int ButtonHeight = 22;
    int LabelsFontSize = 8;
#endif
    
    // Append root to containers
    Containers.append(rootWidget);
    while (!Containers.isEmpty())
    {
        container = Containers.takeFirst();
        if (container != NULL)
        {
            for (int ChIdx=0; ChIdx < container->children().size(); ChIdx++)
            {
                child = container->children()[ChIdx];
                if (!child->isWidgetType() || DoNotAffect.contains(child->objectName()))
                    continue;
                // Append containers to Stack for recursion
                if (child->children().size() > 0)
                    Containers.append(child);
                else
                {
                    // Cast child object to button and label
                    // (if the object is not of the correct type, it will be NULL)
                    QPushButton *button = qobject_cast<QPushButton *>(child);
                    QLabel *label = qobject_cast<QLabel *>(child);
                    if (button != NULL)
                    {
                        button->setMinimumHeight(ButtonHeight); // Win
                        button->setMaximumHeight(ButtonHeight); // Win
                        button->setSizePolicy(QSizePolicy::Fixed,
                                              button->sizePolicy().horizontalPolicy());
                    }
                    else if (label != NULL)
                    {
                        QFont aFont = label->font();
                        aFont.setPointSize(LabelsFontSize);
                        label->setFont(aFont);
                    }
                }
            }
        }
    }
}
