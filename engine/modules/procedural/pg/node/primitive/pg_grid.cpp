#include "pg_grid.h"

namespace Echo
{
	PGGrid::PGGrid()
	{

	}

	PGGrid::~PGGrid()
	{

	}

	void PGGrid::bindMethods()
	{

	}

	void PGGrid::play(PCGData& data)
	{
		vector<PGPoint*>::type points;

		i32 columns = 11;
		i32 rows = 11;
		if (columns > 0 && rows > 0)
		{
			Vector3 basePosition(-0.5f * (rows - 1), 0.f, -0.5f * (columns - 1));

			// vertex buffer
			for (i32 row = 0; row < rows; row++)
			{
				for (i32 column = 0; column < columns; column++)
				{
					PGPoint* point = data.addPoint();

					point->m_position = Vector3(row, 0.f, column) + basePosition;
					point->m_uv = Vector2(row / (rows - 1), column / (columns - 1));
					point->m_normal = Vector3::UNIT_Y;

					points.emplace_back(point);
				}
			}

			// index buffer
			i32 rowLength = rows - 1;
			i32 columnLength = columns - 1;
			for (i32 row = 0; row < rowLength; row++)
			{
				for (i32 column = 0; column < columnLength; column++)
				{
					i32 indexLeftTop = row * columns + column;
					i32 indexRightTop = indexLeftTop + 1;
					i32 indexLeftBottom = indexLeftTop + columns;
					i32 indexRightBottom = indexRightTop + columns;

					PGPrimitive* prim0 = data.addPrimitive();
					prim0->addPoint(points[indexLeftTop]);
					prim0->addPoint(points[indexRightBottom]);
					prim0->addPoint(points[indexRightTop]);

					PGPrimitive* prim1 = data.addPrimitive();
					prim1->addPoint(points[indexLeftTop]);
					prim1->addPoint(points[indexLeftBottom]);
					prim1->addPoint(points[indexRightBottom]);
				}
			}
		}

		m_dirtyFlag = false;
	}
}