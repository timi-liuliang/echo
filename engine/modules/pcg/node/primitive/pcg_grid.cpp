#include "pcg_grid.h"

namespace Echo
{
	PCGGrid::PCGGrid()
	{

	}

	PCGGrid::~PCGGrid()
	{

	}

	void PCGGrid::bindMethods()
	{
		CLASS_BIND_METHOD(PCGGrid, getRows);
		CLASS_BIND_METHOD(PCGGrid, setRows);
		CLASS_BIND_METHOD(PCGGrid, getColumns);
		CLASS_BIND_METHOD(PCGGrid, setColumns);

		CLASS_REGISTER_PROPERTY(PCGGrid, "Rows", Variant::Type::Int, getRows, setRows);
		CLASS_REGISTER_PROPERTY(PCGGrid, "Columns", Variant::Type::Int, getColumns, setColumns);
	}

	void PCGGrid::setRows(i32 rows)
	{
		m_rows = Math::Clamp(rows, 2, 65535);
		m_dirtyFlag = true;
	}

	void PCGGrid::setColumns(i32 columns)
	{
		m_columns = Math::Clamp(columns, 2, 65535);
		m_dirtyFlag = true;
	}

	//void PCGGrid::play()
	//{
		//vector<PCGPoint*>::type points;

		//if (m_columns > 0 && m_rows > 0)
		//{
		//	Vector3 basePosition(-0.5f * (m_rows - 1), 0.f, -0.5f * (m_columns - 1));

		//	// vertex buffer
		//	for (i32 row = 0; row < m_rows; row++)
		//	{
		//		for (i32 column = 0; column < m_columns; column++)
		//		{
		//			PGPoint* point = data.addPoint();

		//			point->m_position = Vector3(row, 0.f, column) + basePosition;
		//			point->m_uv = Vector2(row / (m_rows - 1), column / (m_columns - 1));
		//			point->m_normal = Vector3::UNIT_Y;

		//			points.emplace_back(point);
		//		}
		//	}

		//	// index buffer
		//	i32 rowLength = m_rows - 1;
		//	i32 columnLength = m_columns - 1;
		//	for (i32 row = 0; row < rowLength; row++)
		//	{
		//		for (i32 column = 0; column < columnLength; column++)
		//		{
		//			i32 indexLeftTop = row * m_columns + column;
		//			i32 indexRightTop = indexLeftTop + 1;
		//			i32 indexLeftBottom = indexLeftTop + m_columns;
		//			i32 indexRightBottom = indexRightTop + m_columns;

		//			PCGPrimitive* prim0 = data.addPrimitive();
		//			prim0->addPoint(points[indexLeftTop]);
		//			prim0->addPoint(points[indexRightBottom]);
		//			prim0->addPoint(points[indexRightTop]);

		//			PCGPrimitive* prim1 = data.addPrimitive();
		//			prim1->addPoint(points[indexLeftTop]);
		//			prim1->addPoint(points[indexLeftBottom]);
		//			prim1->addPoint(points[indexRightBottom]);
		//		}
		//	}
		//}

		//m_dirtyFlag = false;
	//}
}