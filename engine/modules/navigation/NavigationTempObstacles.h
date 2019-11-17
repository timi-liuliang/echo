#pragma once

#include "Navigation.h"

class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;	
struct rcPolyMeshDetail;
struct rcChunkyTriMesh;

namespace Echo
{
	class DataStream;
	class InputGeometryData;
	class BuildContext;

	/**
	 * 寻路接口封装
	 */
	class NavigationTempObstacles : public Navigation
	{
	public:
		NavigationTempObstacles();
		~NavigationTempObstacles();

		// 构建导航图
		virtual bool build( float agentRadius, float agentHeight, float stepHeight);

		// 每帧更新
		virtual void update(float delta);

		// 加载
		virtual void load(const String& filePath);

		// 保存
		virtual void save( const char* savePath);

	public:
		// 添加圆柱障碍物
		void addTempObstacleCylinder(const Vector3& pos, float radius, float height);

		// 添加盒子障碍物
		void addTempObstacleBox(const Vector3& vMin, const Vector3& vMax);

		// 添加轴向包围盒障碍物
		void addTempObstacleObb(const Vector3& p0, const Vector3& p1, float width, float height);

		// 移除障碍物
		void removeTempObstacle(const float* sp, const float* sq);

		// 清空所有障碍物
		void clearAllTempObstacles();

	protected:
		// 分解三角形为方块集合
		int rasterizeTileLayers(const rcChunkyTriMesh* chunkyMesh, const int tx, const int ty, const rcConfig& cfg, struct TileCacheData* tiles, const int maxTiles);

		// 清空导航图
		virtual void cleanup();

	private:
		struct LinearAllocator*		m_talloc;
		struct FastLZCompressor*	m_tcomp;
		struct MeshProcess*			m_tmproc;
		class dtTileCache*			m_tileCache;
		int							m_maxTiles;
		int							m_maxPolysPerTile;
		float						m_tileSize;

		int							m_cacheCompressedSize = 0;
		int							m_cacheRawSize;
		int							m_cacheLayerCount = 0;
		int							m_cacheBuildMemUsage = 0;

		bool						m_filterLowHangingObstacles;
		bool						m_filterLedgeSpans = true;
		bool						m_filterWalkableLowHeightSpans;

		struct dtNavMeshData*		m_navMeshData = nullptr;
		struct dtTileCacheData*		m_tileCacheData = nullptr;
	};
}
