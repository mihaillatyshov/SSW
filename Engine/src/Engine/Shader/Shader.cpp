#include "Shader.h"

#include "Engine/Core/Assert.h"

#include "Platform/OpenGL/Shader/OGLShader.h"

namespace LM
{

    //	Ref<Shader> Shader::Create(std::string_view _VertPath, std::string_view _FragPath)
    //	{
    // #ifdef OPENGL
    //		return CreateRef<OGLShader>(_VertPath, _FragPath);
    // #endif
    //	}

    Ref<Shader> Shader::Create(const ShaderLayout& _Layout)
    {
        return CreateRef<OGLShader>(_Layout);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}    // namespace LM
