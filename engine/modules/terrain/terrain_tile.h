#pragma once

namespace Echo
{
	class Terrain;
    class TerrainTile
    {
    public:
		TerrainTile();
		~TerrainTile();

	private:
		Terrain*	m_terrain;
    };
	typedef map<String, TerrainTile*> TerrainTiles;
}
