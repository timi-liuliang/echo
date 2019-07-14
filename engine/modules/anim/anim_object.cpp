#include "anim_object.h"

namespace Echo
{
    AnimObject::~AnimObject()
    {
        EchoSafeDeleteContainer(m_properties, AnimProperty);
    }
}
