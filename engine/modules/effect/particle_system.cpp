#include "engine/core/log/Log.h"
#include "particle_system.h"
#include "engine/core/render/base/Renderer.h"
#include <algorithm>
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/util/magic_enum.hpp"
#include "engine/core/io/IO.h"
#include "engine/core/util/XmlBinary.h"

namespace Echo
{
    ParticleSystem::ParticleSystem()
    {
    }

    ParticleSystem::~ParticleSystem()
    {
    }

    void ParticleSystem::bindMethods()
    {

    }

    Res* ParticleSystem::load(const ResourcePath& path)
    {
        if (!path.isEmpty())
        {
        }

        return nullptr;
    }

    void ParticleSystem::save()
    {
        XmlBinaryWriter writer;
    }
}
