#pragma once

#include <Recast/Recast/Recast.h>
#include <Recast/Recast/DetourNavMesh.h>
#include <Recast/Recast/DetourNavMeshQuery.h>
#include <Recast/Recast/DetourCrowd.h>
#include <engine/core/Math/Math.h>

class dtNavMesh;
class dtNavMeshQuery;
class dtCrowd;
class dtObstacleAvoidanceDebugData;
struct rcHeightfield;
struct rcCompactHeightfield;
struct rcContourSet;
struct rcPolyMesh;
struct rcPolyMeshDetail;

namespace Echo
{
	class DataStream;
	class InputGeometryData;

	/// These are just sample areas to use consistent values across the samples.
	/// The use should specify these base on his needs.
	enum SamplePolyAreas
	{
		SAMPLE_POLYAREA_GROUND,
		SAMPLE_POLYAREA_WATER,
		SAMPLE_POLYAREA_ROAD,
		SAMPLE_POLYAREA_DOOR,
		SAMPLE_POLYAREA_GRASS,
		SAMPLE_POLYAREA_JUMP,
		SAMPLE_POLYAREA_TYPE_0,
		SAMPLE_POLYAREA_TYPE_1,
		SAMPLE_POLYAREA_TYPE_2,
		SAMPLE_POLYAREA_TYPE_3,
		SAMPLE_POLYAREA_TYPE_4,
		SAMPLE_POLYAREA_TYPE_5,
		SAMPLE_POLYAREA_TYPE_6,
		SAMPLE_POLYAREA_TYPE_7,
		SAMPLE_POLYAREA_TYPE_8,
		SAMPLE_POLYAREA_TYPE_9,
		SAMPLE_POLYAREA_TYPE_10,
	};
	enum SamplePolyFlags
	{
		SAMPLE_POLYFLAGS_WALK = 0x01,		// Ability to walk (ground, grass, road)
		SAMPLE_POLYFLAGS_SWIM = 0x02,		// Ability to swim (water).
		SAMPLE_POLYFLAGS_DOOR = 0x04,		// Ability to move through doors.
		SAMPLE_POLYFLAGS_JUMP = 0x08,		// Ability to jump.
		SAMPLE_POLYFLAGS_TYPE_0 = 0x10,
		SAMPLE_POLYFLAGS_TYPE_1 = 0x20,
		SAMPLE_POLYFLAGS_TYPE_2 = 0x40,
		SAMPLE_POLYFLAGS_TYPE_3 = 0x80,
		SAMPLE_POLYFLAGS_TYPE_4 = 0x100,
		SAMPLE_POLYFLAGS_TYPE_5 = 0x200,
		SAMPLE_POLYFLAGS_TYPE_6 = 0x400,
		SAMPLE_POLYFLAGS_TYPE_7 = 0x800,
		SAMPLE_POLYFLAGS_TYPE_8 = 0x1000,
		SAMPLE_POLYFLAGS_TYPE_9 = 0x2000,
		SAMPLE_POLYFLAGS_TYPE_10 = 0x4000,

		SAMPLE_POLYFLAGS_DISABLED = 0x8000,	// Disabled polygon
		SAMPLE_POLYFLAGS_ALL = 0xffff	// All abilities.
	};


	/**
	* 寻路接口封装
	*/
	class Navigation
	{
	public:
		virtual ~Navigation();

		// 每帧更新
		virtual void update(float delta) {}

		// 获取导航网格
		const dtNavMesh* getNavMesh() const { return m_navMesh; }

		// 获取寻路查询器(基于NavMesh寻路)
		const dtNavMeshQuery* getNavMeshQuery() const { return m_navQuery; }

	public:
		// 寻路
		void findPath(const Vector3& startPos, const Vector3& endPos, float*& path, int& pathCount, int type = 0);
		void findPath(const float spos[], const float espos[], float*& smoothPath, int & nsmoothPath, int include_flags);
		void findStraightPath(const Vector3& startPos, const Vector3& endPos, float*& path, int& pathCount);

		// 查找离某位置最近的多边形
		bool findNearestPoly(const Vector3& position, const Vector3& ext, Vector3& nearest);

		//由目标点向起始点出发，找一个两个点之间距离目标点最近的有效点，距离不要太大
		bool findNearestPolyBetween(const Vector3& start, const Vector3& end, Vector3& nearest, float deltaY = 10.f, float step = 2.f);

		// 设置可行走的区域类型
		void setIncludeFlag(unsigned int nFlag);

		// 设置不可行走的区域类型
		void setExcludeFlag(unsigned int nFlag);

		void setCrowdIncludeFlag(int type, unsigned int nFlag);

		void setAgentIncludeFlagType(int agentId, unsigned char type);

		// 设置区域消耗
		void setAreaCost(int i, float cost);

		// 查询相关接口
	public:
		bool rayCast(const Vector3& startPos, const Vector3& endPos, float& oLength, Vector3& hitPosition);

		/**
		* Casts a 'walkability' ray along the surface of the navigation mesh from
		* the start position toward the end position.
		*/
		bool rayCast(const Vector3& startPos, const Vector3& dir, float& distance, Vector3& endPos, int include_flags);

		/**
		* 射线检测(求射线与导航模型相交最近点)
		* 参数[in]		startPos	射线起点
		* 参数[in]		dir			射线朝向(务必单位化)
		* 参数[in out]  oLenght		射线长度(探测长度越短，效率越高)
		* 参数[out]     oHitPosition相交结果(最近点)
		*/
		bool rayDetect(const Vector3& startPos, const Vector3& dir, float& oLength, Vector3& oHitPosition) const;

		/**
		* 射线检测(求射线与与某多边形相交点)
		* 参数[in]		startPos	射线起点
		* 参数[in]		dir			射线朝向(务必单位化)
		* 参数[in out]  oLenght		射线长度(探测长度越短，效率越高)
		* 参数[in]		polyRef		多边形ID
		* 参数[out]     oHitPosition相交结果
		* 返回值	 true==相交，false==不相交
		*/
		bool rayDetectPoly(const Vector3& startPos, const Echo::Vector3& dir, float& oLength, dtPolyRef polyRef, Vector3& oHitPosition) const;

		// 群体寻路相关接口
	public:
		void crowdInit(float agentRadius = 0.6f);

		void setQueryExtents(float agentRadius = 0.6f);

		// 终止代理移动
		void crowdStopAgentMove(int nAgentIndex = 0);

		// 添加群体寻路代理
		int	crowdAddAgent(const Vector3& position, float weight = 0.f, float speed = 3.5f, float radius = 0.f, float maxAcceleration = 10000.f);

		void setDoCollide(bool docollide);
		// 代理是否处于活跃状态
		bool crowdIsAgentActive(const int agentId);

		// 移除代理
		void crowdRemoveAgent(int nAgentIndex);

		// 移除所有群体寻路代理
		void crowdRemoveAllAgents();

		// 默认是主角
		void crowdMoveAgentToword(const Vector3& dir, int nAgentIndex = 0);

		// 获取代理位置
		Vector3 crowdGetAgentPosition(int nAgentIndex = 0);

		// 重置代理位置
		void crowdResetAgentPosition(int nAgentIndex, Echo::Vector3 pos);

		// 更新所有代理
		void crowdUpdateAllAgents(ui32 nElapes);

		// 更新指定代理
		void crowdUpdateAgentOnly(ui32 nElapes, int idx);

		// 更新群体寻路目标点
		void crowdUpdateAgentTarget(const float* p);

		// 设置代理的移动目标
		void crowdSetAgentTarget(int aIndex);

		// 移除代理目标点
		void crowdRemoveAgentTarget(int aIndex);

	public:
		// 设置几何体
		void setGeometry(InputGeometryData* inputData) { m_geom = inputData; }

		// 构建导航网格
		virtual bool build(float agentRadius, float agentHeight, float stepHeight) = 0;

		// 清空导航网格
		virtual void cleanup() {}

		// build navigation mesh from imort data
		virtual void load(const String& filePath) = 0;

		// 保存
		virtual void save(const char* savePath) = 0;

		// 是否已加载
		bool isLoaded() const { return m_isLoaded; }

	protected:
		Navigation();

		// 重置通用设置
		void resetCommonSettings();

		// 射线检测
		bool rayDetect(dtNavMeshQuery* query, const Vector3& startPos, const Vector3& dir, float& oLength, const dtQueryFilter* filter, Echo::Vector3& oHitPosition) const;
	
	protected:
		InputGeometryData*			m_geom;					// 用于生成寻路数据的几何体
		bool						m_isLoaded;				// 是否已加载导航网格
		dtNavMesh*					m_navMesh;				// 导航网格
		dtNavMeshQuery*				m_navQuery;

		dtObstacleAvoidanceDebugData*m_vod;
		dtCrowdAgentDebugInfo		m_agentDebug;
		dtCrowd*					m_crowd;

		float						m_cellSize;
		float						m_cellHeight;
		float						m_agentHeight;
		float						m_agentRadius;
		float						m_agentMaxClimb;
		float						m_agentMaxSlope;
		float						m_regionMinSize;
		float						m_regionMergeSize;
		float						m_edgeMaxLen;
		float						m_edgeMaxError;
		float						m_vertsPerPoly;
		float						m_detailSampleDist;
		float						m_detailSampleMaxError;
		bool						m_monotonePartitioning;

		class BuildContext*			m_ctx;					// 辅助寻路图生成

		float						m_targetPos[3];			// 寻路目标位置
		dtPolyRef					m_targetRef;

		float						m_spos[3];
		float						m_epos[3];
		dtQueryFilter				m_filter;

		static const int			MAX_POLYS = 256;
		dtPolyRef					m_startRef;
		dtPolyRef					m_endRef;
		dtPolyRef					m_polys[MAX_POLYS];
		int							m_npolys;
		float						m_polyPickExt[3];

		float						m_straightPath[MAX_POLYS * 3];
		unsigned char				m_straightPathFlags[MAX_POLYS];
		dtPolyRef					m_straightPathPolys[MAX_POLYS];
		int							m_nstraightPath;

		// path finder data
		static const int			MAX_SMOOTH = 2048;
		float						m_smoothPath[MAX_SMOOTH * 3];
		int							m_nsmoothPath;
	};
}