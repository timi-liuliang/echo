#include "font_texture.h"
#include "engine/core/util/Buffer.h"
#include "engine/core/resource/Res.h"
#include "engine/core/render/interface/Texture.h"

#define INVALID -1

namespace Echo
{
	FontTexture::Node::Node()
		: m_id(INVALID)
	{
		m_child[0] = INVALID;
		m_child[1] = INVALID;
	}

	bool FontTexture::Node::IsLeaf() const
	{
		return m_child[0] == INVALID && m_child[1] == INVALID;
	}

	FontTexture::FontTexture(int width, int height)
		: m_width(width)
		, m_height(height)
		, m_textureData(NULL)
	{
		Node rootNode;
		rootNode.m_rc = IRect(0, 0, m_width, m_height);
		m_nodes.reserve(256);
		m_nodes.push_back(rootNode);
	}

	FontTexture::~FontTexture()
	{
		EchoSafeFree(m_textureData);
	}

	const Vector4 FontTexture::getViewport(int nodeIdx) const
	{
		Vector4	result;
		const IRect& tRc = m_nodes[nodeIdx].m_rc;
		result.x = static_cast<float>(tRc.left) / static_cast<float>(m_width);
		result.y = static_cast<float>(tRc.top) / static_cast<float>(m_height);
		result.z = static_cast<float>(tRc.width) / static_cast<float>(m_width);
		result.w = static_cast<float>(tRc.height) / static_cast<float>(m_height);

		return result;
	}

	int FontTexture::insert(Color* data, int width, int height)
	{
		if (!data)
			return INVALID;

		int nodeIdx = insert(0, data, width, height);

		return overWrite(nodeIdx, data, width, height);
	}

	int FontTexture::overWrite(int nodeIdx, Color* data, int width, int height)
	{
		if (nodeIdx != INVALID)
		{
			m_nodes[nodeIdx].m_id = nodeIdx;
			size_t pixelsize = PixelUtil::GetPixelSize(m_format);

			// copy pixels over from texture to pNode->m_rc part of texture
			if (!m_textureData)
			{
				m_textureData = EchoMalloc(m_width*m_height*pixelsize);
				memset(m_textureData, 0, m_width*m_height*pixelsize);
			}
			const IRect& rc = m_nodes[nodeIdx].m_rc;

			Dword* pDestData = (Dword*)m_textureData;
			for (int h = 0; h < height; h++)
			{
				for (int w = 0; w < width; w++)
				{
					int destIdx = (rc.top + h) * m_width + w + rc.left;
					int srcIdx = h * width + w;
					Color color = data[srcIdx];
					// Determine the largest color component
					float maxCeil = std::max<float>(std::max<float>(std::max<float>(color.r, color.g), color.b), 0.1f);
					color.a = maxCeil / 8.0f;
					color.r /= maxCeil;
					color.g /= maxCeil;
					color.b /= maxCeil;

					pDestData[destIdx] = color.getABGR();
				}
			}

			return nodeIdx;
		}

		return INVALID;
	}

	void FontTexture::refreshTexture()
	{
		size_t pixelsize = PixelUtil::GetPixelSize(m_format);
		Buffer buffer(m_width*m_height*pixelsize, m_textureData, false);
		if (m_texture)
		{
            m_texture->updateTexture2D(PF_BGRA8_UNORM, Texture::TU_GPU_READ, m_width, m_height, buffer.getData(), buffer.getSize());
		}
		else
		{
            m_texture = Texture::createTexture2D(PF_BGRA8_UNORM, Texture::TU_GPU_READ, m_width, m_height, buffer.getData(), buffer.getSize());
		}
	}

	int FontTexture::insert(int nodeIdx, Color* data, int width, int height)
	{
		if (nodeIdx == INVALID)
			return INVALID;

		// if we're not a leaf then
		if (!m_nodes[nodeIdx].IsLeaf())
		{
			int child0Idx = m_nodes[nodeIdx].m_child[0];
			int child1Idx = m_nodes[nodeIdx].m_child[1];
			if (child0Idx != INVALID && child1Idx != INVALID)
			{
				if (m_nodes[child0Idx].m_rc.getArea() > m_nodes[child1Idx].m_rc.getArea())
					std::swap(child0Idx, child1Idx);
			}

			// try inserting into first child
			int newIdx = insert(child0Idx, data, width, height);
			if (newIdx != INVALID)
				return newIdx;

			// no room, insert into second
			return insert(child1Idx, data, width, height);
		}
		else
		{
			// if there is already a lightmap here, return
			if (m_nodes[nodeIdx].m_id != INVALID)
				return INVALID;

			int texWidth = static_cast<int>(width);
			int texHeight = static_cast<int>(height);

			// if we're too small, return
			if (m_nodes[nodeIdx].m_rc.width < texWidth || m_nodes[nodeIdx].m_rc.height < texHeight)
				return INVALID;

			// if we'rs just right accept, "no surplus space"
			if (m_nodes[nodeIdx].m_rc.width == texWidth && m_nodes[nodeIdx].m_rc.height == texHeight)
				return nodeIdx;

			// otherwise, gotta split this node and create some kids
			Node node0, node1;

			// deside split pattern
			int dw = m_nodes[nodeIdx].m_rc.width - texWidth;
			int dh = m_nodes[nodeIdx].m_rc.height - texHeight;

			const IRect& nRect = m_nodes[nodeIdx].m_rc;
			if (dw > dh)
			{
				node0.m_rc = IRect(nRect.left + texWidth, nRect.top, nRect.width - texWidth, nRect.height);
				node1.m_rc = IRect(nRect.left, nRect.top + texHeight, texWidth, nRect.height - texHeight);
			}
			else
			{
				node0.m_rc = IRect(nRect.left + texWidth, nRect.top, nRect.width - texWidth, texHeight);
				node1.m_rc = IRect(nRect.left, nRect.top + texHeight, nRect.width, nRect.height - texHeight);
			}

			// add to subnode with space
			if (node0.m_rc.getArea() > 0)
			{
				m_nodes[nodeIdx].m_child[0] = m_nodes.size();
				m_nodes.push_back(node0);
			}
			if (node1.m_rc.getArea() > 0)
			{
				m_nodes[nodeIdx].m_child[1] = m_nodes.size();
				m_nodes.push_back(node1);
			}

			m_nodes[nodeIdx].m_rc.width = texWidth;
			m_nodes[nodeIdx].m_rc.height = texHeight;

			return nodeIdx;
		}
	}
}
