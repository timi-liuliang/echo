#pragma once

#include "Navigation.h"

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

	/**
	* 寻路接口封装
	*/
	class NavigationSolo : public Navigation
	{
	public:
		NavigationSolo();
		~NavigationSolo();

		// 构建
		virtual bool build(float agentRadius, float agentHeight, float stepHeight);

		// 加载
		virtual void load(const String& filePath);

		// 保存
		virtual void save(const char* savePath);

	protected:
		// 清空数据
		virtual void cleanup();

	private:
		unsigned char*				m_triareas;
		rcHeightfield*				m_solid;
		rcCompactHeightfield*		m_chf;
		rcContourSet*				m_cset;
		rcPolyMesh*					m_pmesh;
		rcPolyMeshDetail*			m_dmesh;

		unsigned char*				m_navData;				// 寻路网络数据
		ui32						m_navDataSize;			// 数据大小
	};
}