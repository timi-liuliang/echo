#pragma once

namespace Echo {
namespace Ui {
    
    // Vertex Format
    struct VertexFormat
    {
        Vector3        m_position;
        Vector2        m_uv;
        
        VertexFormat(const Vector3& pos, const Vector2& uv)
        : m_position(pos), m_uv(uv)
        {}
    };
    typedef vector<VertexFormat>::type  VertexArray;
    typedef vector<Word>::type          IndiceArray;
}}
