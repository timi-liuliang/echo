#pragma once

#include "engine/core/base/object.h"
#include "engine/core/util/StringUtil.h"
#include "build_listener.h"

namespace Echo
{
    class BuildSettings : public Object
    {
        ECHO_VIRTUAL_CLASS(BuildSettings, Object)
        
    public:
        BuildSettings() {}
        virtual ~BuildSettings() {}

        // platform name
        virtual const char* getPlatformName() const { return "Unknown"; }

        // platform thumbnail
        virtual ImagePtr getPlatformThumbnail() const { return nullptr; }
        
        // set
        virtual void setOutputDir(const String& outputDir) {}
        
        // build
        virtual void build() {}
        
        // get final result path
        virtual String getFinalResultPath() { return StringUtil::BLANK; }

        // package root folders
        virtual void packageRes(const String& rootFolder);
        
    public:
        // log
        void setListener(BuildListener* listener) { m_listener = listener; }
        void log(const char* formats, ...);

    protected:
        // write line
		static void writeLine(String& str, const String& line) { str += line + "\n"; }
        
    protected:
        BuildListener*    m_listener = nullptr;
    };
}
