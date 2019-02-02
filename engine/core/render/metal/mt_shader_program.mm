#include "mt_shader_program.h"
#include "mt_renderer.h"

namespace Echo
{
    // create shader library
    bool MTShaderProgram::createShaderProgram(const String& vsContent, const String& psContent)
    {
        id<MTLDevice> device = MTRenderer::instance()->getMetalDevice();
        if(device)
        {
            //m_metalLibrary = [device newLibraryWithSource:<#(nonnull NSString *)#> options:<#(nullable MTLCompileOptions *)#> error:<#(NSError * _Nullable * _Nullable)#>];
        }
    }
}
