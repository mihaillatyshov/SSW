#include "OGL4Texture2D.h"

#include "Engine/Textures/TextureLoader.h"
#include "Platform/OpenGL/Textures/OGLCalcTextureParameters.h"

#include <GL/glew.h>

namespace LM
{

    OGLTexture2D::OGLTexture2D(std::string_view _FileName, Texture2D::MASK _Mask)
        : m_FileName(_FileName),
          m_Mask(_Mask),
          m_Width(0),
          m_Height(0)
    { }

    OGLTexture2D::~OGLTexture2D() { }

    void OGLTexture2D::Bind(uint32_t _SlotId) { glBindTextureUnit(_SlotId, m_TextureID); }

    void OGLTexture2D::Unbind() { }

    void OGLTexture2D::OnAttach()
    {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_TextureID);

        TextureLoader textureData(m_FileName);
        if (!textureData.IsOk())
        {
            LoadOnFileReadError();
        }
        else
        {
            m_Width = textureData.GetWidth();
            m_Height = textureData.GetHeight();

            glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, OGLCalcTextureParameters::WrapS(m_Mask));
            glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, OGLCalcTextureParameters::WrapT(m_Mask));
            glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, OGLCalcTextureParameters::MinFilter(m_Mask));
            glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, OGLCalcTextureParameters::MagFilter(m_Mask));

            glTextureStorage2D(m_TextureID, 1, GL_RGBA8, m_Width, m_Height);
            glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE,
                                textureData.GetData());

            if (OGLCalcTextureParameters::HasMipmap(m_Mask))
            {
                glGenerateTextureMipmap(m_TextureID);
            }
        }

        m_TextureHandle = glGetTextureHandleARB(m_TextureID);
        glMakeTextureHandleResidentARB(m_TextureHandle);
    }

    void OGLTexture2D::OnDetach() { glDeleteTextures(1, &m_TextureID); }

    void OGLTexture2D::LoadOnFileReadError()
    {
        m_Width = 2;
        m_Height = 2;

        uint32_t Data[] = { 0xffff00ff, 0xff000000, 0xff000000, 0xffff00ff };

        glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(m_TextureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureParameteri(m_TextureID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(m_TextureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureStorage2D(m_TextureID, 1, GL_RGBA8, m_Width, m_Height);
        glTextureSubImage2D(m_TextureID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, Data);
    }

}    // namespace LM
