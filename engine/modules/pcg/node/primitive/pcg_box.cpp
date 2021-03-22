#include "pcg_box.h"
#include "engine/core/render/base/mesh/mesh.h"

namespace Echo
{
	PCGBox::PCGBox()
	{

	}

	PCGBox::~PCGBox()
	{

	}

	void PCGBox::bindMethods()
	{
		CLASS_BIND_METHOD(PCGBox, getSize, DEF_METHOD("getSize"));
		CLASS_BIND_METHOD(PCGBox, setSize, DEF_METHOD("setSize"));

		CLASS_REGISTER_PROPERTY(PCGBox, "Size", Variant::Type::Vector3, "getSize", "setSize");
	}

	void PCGBox::setSize(const Vector3& size)
	{
		m_size = size;
		m_dirtyFlag = true;
	}

	//void PCGBox::play(PCGData& data)
	//{
	//	for (i32 i = 0; i < 3; i++)
	//	{
	//		addPlane(data, i, false);
	//		addPlane(data, i, true);
	//	}

	//	m_dirtyFlag = false;
	//}

	//void PCGBox::addPlane(PCGData& data, i32 axis, bool negative)
	//{
	//	Vector3 normal = Vector3::ZERO;
	//	normal[axis] = negative ? -1.f : 1.f;

	//	Vector2 uvs[4] =
	//	{
	//		{-1.f, 1.f},
	//		{ 1.f, 1.f},
	//		{ 1.f, -1.f},
	//		{-1.f, -1.f}
	//	};

	//	Vector3 positions[4];
	//	for (i32 i = 0; i < 4; i++)
	//	{
	//		positions[i][axis] = negative ? -1.f : 1.f;

	//		i32 k = 0;
	//		for (i32 j = 0; j < 3; j++)
	//		{
	//			if (j != axis)
	//			{
	//				positions[i][j] = uvs[i][k];
	//				k++;
	//			}
	//		}
	//	}

	//	vector<PGPoint*>::type points;
	//	for (int i = 0; i < 4; ++i)
	//	{
	//		PGPoint* point = data.addPoint();

	//		point->m_position = positions[i] * m_size * 0.5f;
	//		point->m_normal = normal;
	//		point->m_uv = (uvs[i] + Vector2(1.f, 1.f)) * 0.5f;

	//		points.push_back(point);
	//	}

	//	PCGPrimitive* prim0 = data.addPrimitive();
	//	prim0->addPoint(points[0]);
	//	prim0->addPoint(points[1]);
	//	prim0->addPoint(points[2]);

	//	PCGPrimitive* prim1 = data.addPrimitive();
	//	prim1->addPoint(points[0]);
	//	prim1->addPoint(points[2]);
	//	prim1->addPoint(points[3]);
	//}
}
