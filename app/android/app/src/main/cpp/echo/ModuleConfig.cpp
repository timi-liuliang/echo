#include <engine/core/main/module.h>

namespace Echo
{
    // implement by application or dll
    void registerModules()
    {
        REGISTER_MODULE(GeomModule)
    }
}
