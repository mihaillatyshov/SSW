#include "IndexBuffer.h"

#include "Engine/Core/Assert.h"

#include "Platform/OpenGL4/Buffers/OGL4IndexBuffer.h"

namespace LM
{

    Ref<IndexBuffer> IndexBuffer::Create(uint32_t _Count)
    {
        return CreateRef<OGL4IndexBuffer>(_Count);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<IndexBuffer> IndexBuffer::Create(const uint32_t* _Data, uint32_t _Count)
    {
        return CreateRef<OGL4IndexBuffer>(_Data, _Count);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}    // namespace LM
