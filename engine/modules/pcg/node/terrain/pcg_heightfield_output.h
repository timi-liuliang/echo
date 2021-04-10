#pragma once

#include "engine/modules/pcg/node/pcg_node.h"

namespace Echo
{
	class PCGHeightfieldOutput : public PCGNode
	{
		ECHO_CLASS(PCGHeightfieldOutput, PCGNode)

	public:
		PCGHeightfieldOutput();
		virtual ~PCGHeightfieldOutput();

		// Category
		virtual String getCategory() const override { return "Terrain"; }

		// Target terrain
		const NodePath& getTerrainPath() { return m_terrainPath; }
		void setTerrainPath(const NodePath& terrainPath);

		// Auto create if not exist
		bool isAutoCreate() const { return m_autoCreate; }
		void setAutoCreate(bool autoCreate) { m_autoCreate = autoCreate; }

		// Run
		virtual void run() override;

	protected:
		NodePath		m_terrainPath = "Terrain";
		bool			m_autoCreate = true;
	};
}