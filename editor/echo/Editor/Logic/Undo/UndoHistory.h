#pragma once

#include <engine/core/util/StringUtil.h>
#include <engine/core/base/object.h>

namespace Studio
{
	class UndoHistory
	{
	public:
        // instance
        static UndoHistory* instance();
        
        // snap shot
        void beginSnapshot();
        void snapShot(Echo::Object* obj);
        void endSnapShot();
        
        // undo
        void undo();
        
        // redo
        void redo();
        
        // clear
        void clear();
	};
}
