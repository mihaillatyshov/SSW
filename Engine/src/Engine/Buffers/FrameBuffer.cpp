#include "FrameBuffer.h"

#include "Engine/Core/Assert.h"

#include "Platform/OpenGL4/Buffers/OGL4FrameBuffer.h"

namespace LM
{

    Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferProps& _Props)
    {
        return CreateRef<OGL4FrameBuffer>(_Props);

        CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}    // namespace LM
