#include "VertexBuffer.h"

#include "Engine/Core/Assert.h"

#include "Platform/OpenGL4/Buffers/OGL4VertexBuffer.h"

namespace LM
{

    Ref<VertexBuffer> VertexBuffer::Create(uint32_t _Size)
    {
        return CreateRef<OGL4VertexBuffer>(_Size);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<VertexBuffer> VertexBuffer::Create(const void* _Data, uint32_t _Size)
    {
        return CreateRef<OGL4VertexBuffer>(_Data, _Size);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}    // namespace LM
