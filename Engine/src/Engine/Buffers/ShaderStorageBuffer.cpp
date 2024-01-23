#include "ShaderStorageBuffer.h"

#include "Engine/Core/Assert.h"

#include "Platform/OpenGL4/Buffers/OGL4ShaderStorageBuffer.h"

namespace LM
{

    Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(uint32_t _Size)
    {
        return CreateRef<OGL4ShaderStorageBuffer>(_Size);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

    Ref<ShaderStorageBuffer> ShaderStorageBuffer::Create(const void* _Data, uint32_t _Size)
    {
        return CreateRef<OGL4ShaderStorageBuffer>(_Data, _Size);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}    // namespace LM
