#include "SimpleRenderable2D.h"

#include "GL/glew.h"

namespace LM
{

    void SimpleRenderable2D::Create(const std::vector<glm::vec4>& _Vertices, const std::vector<uint32_t>& _Indices)
    {
        BufferLayout VerticesLayout({
            {ShaderDataType::Float4, "a_Position"},
        });

        m_Vertices = VertexBuffer::Create(_Vertices.data(), _Vertices.size() * VerticesLayout.GetStride());
        m_Vertices->SetLayout(VerticesLayout);

        m_Indices = IndexBuffer::Create(_Indices.data(), _Indices.size());

        m_VertexArray = VertexArray::Create();
        m_VertexArray->AddVertexBuffer(m_Vertices);
        m_VertexArray->SetIndexBuffer(m_Indices);
    }

    void SimpleRenderable2D::Draw() const
    {
        m_VertexArray->Bind();
        glDrawElements(GL_TRIANGLES, m_Indices->GetCount(), GL_UNSIGNED_INT, NULL);
    }

}
