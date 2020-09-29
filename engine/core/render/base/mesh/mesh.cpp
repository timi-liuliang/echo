#include "engine/core/log/Log.h"
#include "mesh.h"
#include "engine/core/render/base/renderer.h"
#include <algorithm>
#include <thirdparty/pugixml/pugixml.hpp>
#include "engine/core/util/magic_enum.hpp"
#include "engine/core/io/IO.h"
#include "engine/core/util/XmlBinary.h"

namespace Echo
{
	Mesh* Mesh::create(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer)
	{
		return EchoNew(Mesh(isDynamicVertexBuffer, isDynamicIndicesBuffer));
	}

	Mesh::Mesh(bool isDynamicVertexBuffer, bool isDynamicIndicesBuffer)
		: m_topologyType(TT_TRIANGLELIST)
		, m_isDynamicVertexBuffer(isDynamicVertexBuffer)
		, m_isDynamicIndicesBuffer(isDynamicIndicesBuffer)
	{
	}

	Mesh::~Mesh()
	{
		clear();
	}

	void Mesh::bindMethods()
	{

	}

	GPUBuffer* Mesh::getVertexBuffer() const
	{
		return m_vertexBuffer;
	}

	GPUBuffer* Mesh::getIndexBuffer() const
	{
		return m_indexBuffer;
	}

	void Mesh::buildTangentData()
	{
		ui32 faceCount = getFaceCount();

		// build tangent data by position and uv
		vector<Vector3>::type tangentDatas;  tangentDatas.resize(m_vertData.getVertexCount(), Vector3::ZERO);
		vector<Vector3>::type binormalDatas;  binormalDatas.resize(m_vertData.getVertexCount(), Vector3::ZERO);
		for (ui32 i = 0; i < faceCount; i++)
		{
			Word* indices = (Word*)m_indices.data();

			// only support trang list topology
			Word baseIdx = i * 3;
			Word vertIdx0 = indices[baseIdx + 0];
			Word vertIdx1 = indices[baseIdx + 1];
			Word vertIdx2 = indices[baseIdx + 2];

			const Vector3& pos0 = m_vertData.getPosition(vertIdx0);
			const Vector3& pos1 = m_vertData.getPosition(vertIdx1);
			const Vector3& pos2 = m_vertData.getPosition(vertIdx2);

			const Vector2& uv0 = m_vertData.getUV0(vertIdx0);
			const Vector2& uv1 = m_vertData.getUV0(vertIdx1);
			const Vector2& uv2 = m_vertData.getUV0(vertIdx2);

			Vector3 deltaPos1 = pos1 - pos0;
			Vector3 deltaPos2 = pos2 - pos0;

			Vector2 deltauv1 = uv1 - uv0;
			Vector2 deltauv2 = uv2 - uv0;

			float r = 1.f / (deltauv1.x*deltauv2.y - deltauv1.y*deltauv2.x);
			Vector3 tangent = (deltaPos1*deltauv2.y - deltaPos2*deltauv1.y) * r;
			Vector3 binormal = (deltaPos2*deltauv1.x - deltaPos1*deltauv2.x) * r;

			tangentDatas[vertIdx0] += tangent;
			tangentDatas[vertIdx1] += tangent;
			tangentDatas[vertIdx2] += tangent;

			binormalDatas[vertIdx0] += binormal;
			binormalDatas[vertIdx1] += binormal;
			binormalDatas[vertIdx2] += binormal;
		}

		// normalize
		for (size_t i = 0; i < tangentDatas.size(); i++)
		{
			tangentDatas[i].normalize();
			binormalDatas[i].normalize();
		}
	}

	void Mesh::clear()
	{
		m_indices.clear();
		m_indices.shrink_to_fit();

		EchoSafeDelete(m_vertexBuffer, GPUBuffer);
		EchoSafeDelete(m_indexBuffer, GPUBuffer);

		m_vertData.reset();
	}

	ui32 Mesh::getIndexCount() const
	{
		return m_idxCount;
	}

	ui32 Mesh::getFaceCount() const
	{
		ui32 count = m_indexBuffer ? m_idxCount : getVertexCount();
		switch (m_topologyType)
		{
		case TT_POINTLIST:		return count;
		case TT_LINELIST:		return count / 2;
		case TT_LINESTRIP:		return count - 1;
		case TT_TRIANGLELIST:	return count / 3;
		case TT_TRIANGLESTRIP:	return count - 2;
		default:				return 0;
		}
	}

	ui32 Mesh::getIndexStride() const
	{
		return m_idxStride;
	}

	Word* Mesh::getIndices() const
	{
		return (Word*)m_indices.data();
	}

	ui32 Mesh::getMemeoryUsage() const
	{
		return m_vertData.getVertexStride()*m_vertData.getVertexCount() + m_idxCount*m_idxStride;
	}

	void Mesh::generateTangentData(bool useNormalMap)
	{
		if (useNormalMap)
		{
			buildTangentData();
		}
		else
		{
			//m_vertData.m_isUseTangentBinormal = false;
			//removeVertexData(RenderInput::VS_TANGENT);
			//removeVertexData(RenderInput::VS_BINORMAL);
		}
	}

	const VertexElementList& Mesh::getVertexElements() const
	{
		return m_vertData.getFormat().m_vertexElements;
	}

	bool Mesh::buildBuffer()
	{
		buildIndexBuffer();
		buildVertexBuffer();

		return true;
	}

	void Mesh::buildIndexBuffer()
	{
		Buffer indexBuff(m_idxCount*m_idxStride, m_indices.data());
		if (m_isDynamicIndicesBuffer)
		{
			if (!m_indexBuffer)
				m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_DYNAMIC, indexBuff);
			else
				m_indexBuffer->updateData(indexBuff);
		}
		else
		{
			if(!m_indexBuffer)
				m_indexBuffer = Renderer::instance()->createIndexBuffer(GPUBuffer::GBU_GPU_READ, indexBuff);
			else
				EchoLogError("Cannot modify static mesh index buffer");
		}	
	}

	void Mesh::buildVertexBuffer()
	{
		Buffer vertBuff(m_vertData.getByteSize(), m_vertData.getVertices());
		if (m_isDynamicVertexBuffer)
		{
			if (!m_vertexBuffer)
				m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_DYNAMIC, vertBuff);
			else
				m_vertexBuffer->updateData(vertBuff);
		}
		else
		{
			if (!m_vertexBuffer)
				m_vertexBuffer = Renderer::instance()->createVertexBuffer(GPUBuffer::GBU_GPU_READ, vertBuff);
			else
				EchoLogError("Cannot modify static mesh vertex buffer");
		}	
	}

	void Mesh::updateIndices(ui32 indicesCount, ui32 indicesStride, const void* indices)
	{
		// load indices
		m_idxCount = indicesCount;
		m_idxStride = indicesStride;
		if (m_idxCount)
		{
			const Byte* indicesInByte = (const Byte*)indices;
			ui32 idxBuffSize = m_idxCount * m_idxStride;
			m_indices.assign(indicesInByte, indicesInByte +idxBuffSize);

			buildIndexBuffer();
		}
	}

	void Mesh::updateVertexs(const MeshVertexFormat& format, ui32 vertCount, const Byte* vertices)
	{
		m_vertData.set(format, vertCount);
		if (vertCount)
		{
			// copy data
			memcpy(m_vertData.getVertices(), vertices, vertCount * m_vertData.getVertexStride());

			// calculate local aabb
			m_box.reset();
			for (i32 i = 0; i < m_vertData.getVertexCount(); i++)
			{
				m_box.addPoint(m_vertData.getPosition(i));
			}

			// update vertex buffer
			buildVertexBuffer();
		}
	}

	void Mesh::updateVertexs(const MeshVertexData& vertexData)
	{
		m_vertData = vertexData;

		// calculate local aabb
		m_box.reset();
		for (i32 i = 0; i < m_vertData.getVertexCount(); i++)
		{
			m_box.addPoint(m_vertData.getPosition(i));
		}

		buildVertexBuffer();
	}

	Res* Mesh::load(const ResourcePath& path)
	{
		if (!path.isEmpty())
		{
			Mesh* res = EchoNew(Mesh);
			if (res)
			{
				XmlBinaryReader reader;
				if (reader.load(path.getPath().c_str()))
				{
					// root node
					pugi::xml_node root = reader.getRoot();
					String topology = root.attribute("topology").as_string();
					res->setTopologyType(magic_enum::enum_cast<Mesh::TopologyType>(topology.c_str()).value_or(Mesh::TT_TRIANGLELIST));

					// indices
					pugi::xml_node indices = root.child("indices");
					i32 indicesCount = indices.attribute("count").as_int();
					i32 indicesStride = indices.attribute("stride").as_int();

					// indices data
					XmlBinaryReader::Data indicesData;
					reader.getData("Indices", indicesData);

					// parse vertex
					XmlBinaryReader::Data positionData, normalData, colorData, uv0Data, blendingData;
					reader.getData("Position", positionData);
					reader.getData("Normal", normalData);
					reader.getData("UV0", uv0Data);

					MeshVertexFormat vertFormat;
					vertFormat.m_isUseNormal = !normalData.isEmpty();
					vertFormat.m_isUseUV = !uv0Data.isEmpty();
					vertFormat.m_isUseVertexColor = !colorData.isEmpty();
					vertFormat.m_isUseBlendingData = !blendingData.isEmpty();

					// vertex
					pugi::xml_node vertex = root.child("vertex");
					i32 vertCount = vertex.attribute("count").as_int();

					// init vertex data
					MeshVertexData vertexData;
					vertexData.set(vertFormat, vertCount);

					if (!positionData.isEmpty() && positionData.m_type == "Vector3")
					{
						Vector3* positions = (Vector3*)(positionData.m_data.data());
						for (int i = 0; i < vertCount; i++)
						{
							vertexData.setPosition(i, positions[i]);
						}
					}

					if (!normalData.isEmpty() && normalData.m_type == "Byte3")
					{
						Byte* normals = (Byte*)(normalData.m_data.data());
						for (int i = 0; i < vertCount; i++)
						{
							Vector3 normal;
							normal.x = normals[i * 3 + 0] / 255.f * 2.f - 1.f;
							normal.y = normals[i * 3 + 1] / 255.f * 2.f - 1.f;
							normal.z = normals[i * 3 + 2] / 255.f * 2.f - 1.f;
							vertexData.setNormal(i, normal);
						}
					}

					if (!uv0Data.isEmpty() && uv0Data.m_type == "Vector2")
					{
						Vector2* uvs = (Vector2*)(uv0Data.m_data.data());
						for (int i = 0; i < vertCount; i++)
						{
							vertexData.setUV0(i, uvs[i]);
						}
					}

					// set indices data
					if (!indicesData.isEmpty())
					{
						res->updateIndices(indicesCount, indicesStride, indicesData.m_data.data());
					}

					// set vertex data
					res->updateVertexs(vertexData);

					return res;
				}
			}
		}

		return nullptr;
	}

	void Mesh::save()
	{
		XmlBinaryWriter writer;

		// root node
		pugi::xml_node root = writer.getRoot();
		root.append_attribute("topology").set_value(std::string(magic_enum::enum_name(m_topologyType)).c_str());

		// indices
		pugi::xml_node indices = root.append_child("indices");
		indices.append_attribute("count").set_value(getIndexCount());
		indices.append_attribute("stride").set_value(getIndexStride());
		writer.addData("Indices", StringUtil::Format("Byte%d", getIndexStride()).c_str(), getIndices(), getIndexCount() * getIndexStride());

		// vertex
		pugi::xml_node vertex = root.append_child("vertex");
		vertex.append_attribute("count").set_value(m_vertData.getVertexCount());

		// positions
		{
			ByteArray positions = m_vertData.getPositions();
			writer.addData("Position", "Vector3", positions.data(), positions.size());
		}

		// normal
		if (m_vertData.getFormat().m_isUseNormal)
		{
			ByteArray normals = m_vertData.getNormals();
			writer.addData("Normal", "Byte3", normals.data(), normals.size());
		}

		// color
		if (m_vertData.getFormat().m_isUseVertexColor)
		{

		}

		// uv
		if (m_vertData.getFormat().m_isUseUV)
		{
			ByteArray uvs = m_vertData.getUV0s();
			writer.addData("UV0", "Vector2", uvs.data(), uvs.size());
		}

		// uv1
		if (m_vertData.getFormat().m_isUseLightmapUV)
		{

		}

		// blending data
		if (m_vertData.getFormat().m_isUseBlendingData)
		{

		}

		// tangent binormal
		if (m_vertData.getFormat().m_isUseTangentBinormal)
		{

		}

		// write
		writer.save(m_path.getPath().c_str());
	}
}
