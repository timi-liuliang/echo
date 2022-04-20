#include "light.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	static std::unordered_map<i32, Light*> g_lights;

	Light::Light()
	{
		g_lights[m_id] = this;
	}

	Light::Light(Type type)
		: Node()
		, m_lightType(type)
	{
		g_lights[m_id] = this;
	}

	Light::~Light()
	{
		auto it = g_lights.find(m_id);
		if (it != g_lights.end())
		{
			g_lights.erase(it);
		}
		else
		{
			EchoLogError("Light isn't exist. destruct failed.");
		}
	}

	void Light::bindMethods()
	{
		CLASS_BIND_METHOD(Light, is2d);
		CLASS_BIND_METHOD(Light, set2d);

		CLASS_REGISTER_PROPERTY(Light, "Is2D", Variant::Type::Bool, is2d, set2d);
	}

	vector<Light*>::type Light::gatherLights(i32 types)
	{
		vector<Light*>::type result;
		for (auto it : g_lights)
		{
			if (it.second->isType(Type::Direction))
			{
				result.emplace_back(it.second);
			}
		}

		return result;
	}
}
