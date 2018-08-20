#include "LuaScript.h"
#include "engine/core/io/IO.h"

static const char* luaScriptTemplate = 
R"(local template ={}

-- start
function template:start()
end

-- update
function template:update()
end

return template
)";

namespace Echo
{
	LuaScript::LuaScript()
	{
		//EchoAssert(false);
	}

	LuaScript::LuaScript(const ResourcePath& path)
		: Res( path)
	{

	}
	
	LuaScript::~LuaScript()
	{

	}

	void LuaScript::bindMethods()
	{

	}

	Res* LuaScript::load(const ResourcePath& path)
	{
		MemoryReader reader(path.getPath());
		if (reader.getSize())
		{
			LuaScript* res = EchoNew(LuaScript(path));
			res->m_srcData = reader.getData<const char*>();

			return res;
		}

		return nullptr;
	}

	void LuaScript::save()
	{
		const char* content = m_srcData.empty() ? luaScriptTemplate : m_srcData.data();
		if (content)
		{
			String fullPath = IO::instance()->getFullPath(m_path.getPath());
			std::ofstream f(fullPath.c_str());
			
			unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
			f.write((char*)bom, sizeof(bom));

			f << content;

			f.flush();
			f.close();
		}
	}
}