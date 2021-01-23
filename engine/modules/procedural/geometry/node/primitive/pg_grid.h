#pragma once

#include "../../../procedural_geometry.h"

namespace Echo
{
	class PGGrid : public PGNode
	{
		ECHO_CLASS(PGGrid, PGNode)

	public:
		PGGrid();
		virtual ~PGGrid();

		// calculate
		virtual void play(PCGData& data) override;

		// rows
		i32 getRows() const { return m_rows; }
		void setRows(i32 rows);

		// columns
		i32 getColumns() const { return m_columns; }
		void setColumns(i32 columns);

	protected:
		i32		m_rows = 11;
		i32		m_columns = 11;
	};
}
