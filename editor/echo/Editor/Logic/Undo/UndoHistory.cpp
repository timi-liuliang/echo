#include "UndoHistory.h"

namespace Studio
{
    UndoHistory* UndoHistory::instance()
    {
        static UndoHistory inst;
        return &inst;
    }

    void UndoHistory::beginSnapshot()
    {
        
    }

    void UndoHistory::snapShot(Echo::Object* obj)
    {
        
    }

    void UndoHistory::endSnapShot()
    {
        
    }

    void UndoHistory::undo()
    {
        
    }

    void UndoHistory::redo()
    {
        
    }

    void UndoHistory::clear()
    {
        
    }
}
