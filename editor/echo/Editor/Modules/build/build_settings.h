#pragma once

#include "engine/core/base/object.h"
#include "engine/core/util/StringUtil.h"
#include "build_log.h"

namespace Echo
{
    class BuildSettings : public Object
    {
        ECHO_VIRTUAL_CLASS(BuildSettings, Object)
        
    public:
        BuildSettings() {}
        virtual ~BuildSettings() {}
        
        // build
        virtual void build() {}
        
        // get final result path
        virtual String getFinalResultPath() { return StringUtil::BLANK; }
        
    public:
        // log
        void setListener(BuildLog* log) { m_listener = log; }
        void log(const char* formats, ...);
        
    protected:
        BuildLog*    m_listener = nullptr;
    };
}
