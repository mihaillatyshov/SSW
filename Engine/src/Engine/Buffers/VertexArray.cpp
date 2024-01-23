#include "VertexArray.h"

#include "Engine/Core/Assert.h"

#include "Platform/OpenGL4/Buffers/OGL4VertexArray.h"

namespace LM
{

    Ref<VertexArray> VertexArray::Create()
    {
        return CreateRef<OGL4VertexArray>();

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}    // namespace LM
