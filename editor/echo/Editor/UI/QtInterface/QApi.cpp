#include "Studio.h"
#include "QCheckBox.cx"
#include "QComboBox.cx"
#include "QCursor.cx"
#include "QEvent.cx"

#define BIND_EDITOR_FUN(Fun) this->Fun = Echo::Fun

namespace Studio
{
    // init functions
    void AStudio::initFunctions()
    {
        BIND_EDITOR_FUN(qCheckBoxNew);
        
        // QComboBox->addItem(const QIcon &icon, const QString &text)
        BIND_EDITOR_FUN(qComboBoxAddItem);
        BIND_EDITOR_FUN(qComboBoxCurrentIndex);
        BIND_EDITOR_FUN(qComboBoxCurrentText);
        BIND_EDITOR_FUN(qComboBoxSetCurrentIndex);
        BIND_EDITOR_FUN(qComboBoxSetCurrentText);
        BIND_EDITOR_FUN(qComboBoxSetItemText);
        BIND_EDITOR_FUN(qComboBoxClear);
        
        // QCursor
        BIND_EDITOR_FUN(qCursorPos);
        
        // QEvent
        BIND_EDITOR_FUN(qObjectGetEventAll);
        BIND_EDITOR_FUN(qGraphicsItemGetEventAll);
    }
}
