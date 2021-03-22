#pragma once

#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	class PCGGrid : public PCGNode
	{
		ECHO_CLASS(PCGGrid, PCGNode)

	public:
		PCGGrid();
		virtual ~PCGGrid();

		// rows
		i32 getRows() const { return m_rows; }
		void setRows(i32 rows);

		// columns
		i32 getColumns() const { return m_columns; }
		void setColumns(i32 columns);

		// calculate
		virtual void run() override;

	protected:
		i32		m_rows = 11;
		i32		m_columns = 11;
	};
}
