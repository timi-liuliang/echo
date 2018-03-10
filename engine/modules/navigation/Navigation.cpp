#include "engine/core/Util/LogManager.h"
#include <float.h>
#include "Navigation.h"
#include "BuildContext.h"
#include "Engine/core/Geom/Ray.h"
#include <thirdparty/Recast/Recast/DetourCommon.h>

namespace Echo
{
	static void *dtAllocDefault(size_t size, dtAllocHint)
	{
		return EchoMalloc(size);
	}

	static void dtFreeDefault(void *ptr)
	{
		EchoSafeFree(ptr);
	}

	static bool inRange(const float* v1, const float* v2, const float r, const float h)
	{
		const float dx = v2[0] - v1[0];
		const float dy = v2[1] - v1[1];
		const float dz = v2[2] - v1[2];
		return (dx*dx + dz*dz) < r*r && fabsf(dy) < h;
	}

	// 获取目标
	static bool getSteerTarget(dtNavMeshQuery* navQuery, const float* startPos, const float* endPos,
		const float minTargetDist,
		const dtPolyRef* path, const int pathSize,
		float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef,
		float* outPoints = 0, int* outPointCount = 0)
	{
		// Find steer target.
		static const int MAX_STEER_POINTS = 3;
		float steerPath[MAX_STEER_POINTS * 3];
		unsigned char steerPathFlags[MAX_STEER_POINTS];
		dtPolyRef steerPathPolys[MAX_STEER_POINTS];
		int nsteerPath = 0;
		navQuery->findStraightPath(startPos, endPos, path, pathSize,
			steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
		if (!nsteerPath)
			return false;

		if (outPoints && outPointCount)
		{
			*outPointCount = nsteerPath;
			for (int i = 0; i < nsteerPath; ++i)
				dtVcopy(&outPoints[i * 3], &steerPath[i * 3]);
		}


		// Find vertex far enough to steer to.
		int ns = 0;
		while (ns < nsteerPath)
		{
			// Stop at Off-Mesh link or when point is further than slop away.
			if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
				!inRange(&steerPath[ns * 3], startPos, minTargetDist, 1000.0f))
				break;
			ns++;
		}
		// Failed to find good point to steer to.
		if (ns >= nsteerPath)
			return false;

		dtVcopy(steerPos, &steerPath[ns * 3]);
		steerPos[1] = startPos[1];
		steerPosFlag = steerPathFlags[ns];
		steerPosRef = steerPathPolys[ns];

		return true;
	}

	static int fixupCorridor(dtPolyRef* path, const int npath, const int maxPath, const dtPolyRef* visited, const int nvisited)
	{
		int furthestPath = -1;
		int furthestVisited = -1;

		// Find furthest common polygon.
		for (int i = npath - 1; i >= 0; --i)
		{
			bool found = false;
			for (int j = nvisited - 1; j >= 0; --j)
			{
				if (path[i] == visited[j])
				{
					furthestPath = i;
					furthestVisited = j;
					found = true;
				}
			}
			if (found)
				break;
		}

		// If no intersection found just return current path. 
		if (furthestPath == -1 || furthestVisited == -1)
			return npath;

		// Concatenate paths.	

		// Adjust beginning of the buffer to include the visited.
		const int req = nvisited - furthestVisited;
		const int orig = rcMin(furthestPath + 1, npath);
		int size = rcMax(0, npath - orig);
		if (req + size > maxPath)
			size = maxPath - req;
		if (size)
			memmove(path + req, path + orig, size*sizeof(dtPolyRef));

		// Store visited
		for (int i = 0; i < req; ++i)
			path[i] = visited[(nvisited - 1) - i];

		return req + size;
	}

	// This function checks if the path has a small U-turn, that is,
	// a polygon further in the path is adjacent to the first polygon
	// in the path. If that happens, a shortcut is taken.
	// This can happen if the target (T) location is at tile boundary,
	// and we're (S) approaching it parallel to the tile edge.
	// The choice at the vertex can be arbitrary, 
	//  +---+---+
	//  |:::|:::|
	//  +-S-+-T-+
	//  |:::|   | <-- the step can end up in here, resulting U-turn path.
	//  +---+---+
	static int fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery)
	{
		if (npath < 3)
			return npath;

		// Get connected polygons
		static const int maxNeis = 16;
		dtPolyRef neis[maxNeis];
		int nneis = 0;

		const dtMeshTile* tile = 0;
		const dtPoly* poly = 0;
		if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
			return npath;

		for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
		{
			const dtLink* link = &tile->links[k];
			if (link->ref != 0)
			{
				if (nneis < maxNeis)
					neis[nneis++] = link->ref;
			}
		}

		// If any of the neighbour polygons is within the next few polygons
		// in the path, short cut to that polygon directly.
		static const int maxLookAhead = 6;
		int cut = 0;
		for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
			for (int j = 0; j < nneis; j++)
			{
				if (path[i] == neis[j]) {
					cut = i;
					break;
				}
			}
		}
		if (cut > 1)
		{
			int offset = cut - 1;
			npath -= offset;
			for (int i = 1; i < npath; i++)
				path[i] = path[i + offset];
		}

		return npath;
	}

	// 构造函数
	Navigation::Navigation()
		: m_isLoaded(false)
		, m_geom(nullptr)
		, m_navMesh(nullptr)
		, m_navQuery(nullptr)
		, m_startRef(0)
		, m_endRef(0)
		, m_npolys(0)
		, m_nsmoothPath(0)
	{
		// 设置内存分配函数
		dtAllocSetCustom(dtAllocDefault, dtFreeDefault);

		m_polyPickExt[0] = 2;
		m_polyPickExt[1] = 4;
		m_polyPickExt[2] = 2;

		m_filter.setIncludeFlags(SAMPLE_POLYFLAGS_ALL ^ SAMPLE_POLYFLAGS_DISABLED);
		m_filter.setExcludeFlags(0);

		resetCommonSettings();
		m_navQuery = dtAllocNavMeshQuery();
		m_crowd = dtAllocCrowd();

		m_ctx = EchoNew(BuildContext);

		m_vod = dtAllocObstacleAvoidanceDebugData();
		m_vod->init(2048);

		memset(&m_agentDebug, 0, sizeof(m_agentDebug));
		m_agentDebug.idx = -1;
		m_agentDebug.vod = m_vod;
	}

	Navigation::~Navigation()
	{
		dtFreeCrowd(m_crowd);

		dtFreeNavMeshQuery(m_navQuery);
		cleanup();

		EchoSafeDelete(m_ctx, BuildContext);
		dtFreeObstacleAvoidanceDebugData(m_vod);
	}

	void Navigation::resetCommonSettings()
	{
		m_cellSize = 0.2f;
		m_cellHeight = 0.1f;
		m_agentHeight = 2.0f;
		m_agentRadius = 0.7f;
		m_agentMaxClimb = 0.9f;
		m_agentMaxSlope = 45.0f;
		m_regionMinSize = 8;
		m_regionMergeSize = 20;
		m_monotonePartitioning = false;
		m_edgeMaxLen = 12.0f;
		m_edgeMaxError = 1.3f;
		m_vertsPerPoly = 6.0f;
		m_detailSampleDist = 6.0f;
		m_detailSampleMaxError = 1.0f;
	}

	// 更新群体寻路目标
	void  Navigation::crowdUpdateAgentTarget(const float* p)
	{
		if (m_navQuery && m_crowd)
		{
			const float* ext = m_crowd->getQueryExtents();
			const dtQueryFilter* filter = m_crowd->getEditableFilter(0);
			m_navQuery->findNearestPoly(p, ext, filter, &m_targetRef, m_targetPos);
		}
	}

	void Navigation::crowdSetAgentTarget(int aIndex)
	{
		if (m_crowd)
		{
			dtCrowdAgent* ag = m_crowd->getEditableAgent(aIndex);
			if (ag && ag->active)
			{
				m_crowd->requestMoveTarget(aIndex, m_targetRef, m_targetPos);
			}
			ag->params.maxSpeed = 2.f;
		}
	}

	void  Navigation::crowdRemoveAgentTarget(int aIndex)
	{
		if (m_crowd)
		{
			dtCrowdAgent* ag = m_crowd->getEditableAgent(aIndex);
			if (ag && ag->active)
			{
				m_crowd->resetMoveTarget(aIndex);
			}
		}
	}

	void Navigation::setAreaCost(int i, float cost)
	{
		if (i >= DT_MAX_AREAS || i < 0)
		{
			return;
		}
		m_filter.setAreaCost(i, cost);
		if (m_crowd)
		{
			dtQueryFilter* pFilter = const_cast<dtQueryFilter*>(m_crowd->getEditableFilter(0));
			pFilter->setAreaCost(i, cost);
		}
	}

	bool Navigation::rayCast(const Vector3& startPos, const Vector3& endPos, float& oLength, Vector3& hitPosition)
	{
		if (!m_navMesh)
		{
			return false;
		}

		memcpy(m_spos, &startPos, sizeof(float) * 3);
		memcpy(m_epos, &endPos, sizeof(float) * 3);

		m_navQuery->findNearestPoly(m_spos, m_polyPickExt, &m_filter, &m_startRef, 0);

		float fHit = FLT_MAX;
		float fNormal[3];

		m_navQuery->raycast(m_startRef, m_spos, m_epos, &m_filter, &fHit, fNormal, m_polys, &m_npolys, MAX_POLYS);

		if (fHit != FLT_MAX)
		{
			Ray rayHit = Ray(startPos, endPos - startPos);
			oLength = fHit;
			hitPosition = rayHit.getPoint(fHit);
			return true;
		}
		else
		{
			return false;
		}
	}

	void Navigation::setIncludeFlag(unsigned int nFlag)
	{
		m_filter.setIncludeFlags(nFlag);
		if (m_crowd)
		{
			dtQueryFilter* pFilter = const_cast<dtQueryFilter*>(m_crowd->getEditableFilter(0));
			pFilter->setIncludeFlags(nFlag);
		}
	}

	void Navigation::setExcludeFlag(unsigned int nFlag)
	{
		m_filter.setExcludeFlags(nFlag);
		if (m_crowd)
		{
			dtQueryFilter* pFilter = const_cast<dtQueryFilter*>(m_crowd->getEditableFilter(0));
			pFilter->setExcludeFlags(nFlag);
		}
	}

	void Navigation::crowdRemoveAllAgents()
	{
		if (m_crowd)
		{
			for (int i = 0; i < m_crowd->getAgentCount(); i++)
				m_crowd->removeAgent(i);
		}
	}

	void Navigation::findStraightPath(const Vector3& startPos, const Vector3& endPos, float*& path, int& pathCount)
	{
		memcpy(m_spos, &startPos, sizeof(float) * 3);
		memcpy(m_epos, &endPos, sizeof(float) * 3);
		if (!m_navMesh)
			return;
		m_navQuery->findNearestPoly(m_spos, m_polyPickExt, &m_filter, &m_startRef, 0);
		m_navQuery->findNearestPoly(m_epos, m_polyPickExt, &m_filter, &m_endRef, 0);

		if (m_startRef && m_endRef)
		{
			m_navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, &m_filter, m_polys, &m_npolys, MAX_POLYS);
			m_nstraightPath = 0;
			if (m_npolys)
			{
				// In case of partial path, make sure the end point is clamped to the last polygon.
				float epos[3];
				dtVcopy(epos, m_epos);
				if (m_polys[m_npolys - 1] != m_endRef)
					m_navQuery->closestPointOnPoly(m_polys[m_npolys - 1], m_epos, epos, nullptr);

				m_navQuery->findStraightPath(m_spos, epos, m_polys, m_npolys,
					m_straightPath, m_straightPathFlags,
					m_straightPathPolys, &m_nstraightPath, MAX_POLYS);
			}
		}
		else
		{
			m_npolys = 0;
			m_nstraightPath = 0;
		}

		path = m_straightPath;
		pathCount = m_nstraightPath;
	}

	void Navigation::crowdInit(float agentRadius)
	{
		if (m_crowd)
		{
			m_crowd->init(100,  agentRadius, m_navMesh);
		}
	}

	void Navigation::setQueryExtents(float agentRadius)
	{
		if (m_crowd)
		{
			const float* ext_const = m_crowd->getQueryExtents();
			float* ext = const_cast<float*>(ext_const);
			dtVset(ext, agentRadius*2.0f, agentRadius*1.5f, agentRadius*2.0f);
		}
	}

	void Navigation::crowdStopAgentMove(int nAgentIndex /*= 0*/)
	{
		if (m_crowd)
		{
			m_crowd->resetMoveTarget(nAgentIndex);
		}
	}

	void Navigation::crowdRemoveAgent(int nAgentIndex)
	{
		if (m_crowd)
		{
			m_crowd->removeAgent(nAgentIndex);
		}
	}

	// 射线检测(求射线与与某多边形相交点)
	bool Navigation::rayDetectPoly(const Vector3& startPos, const Echo::Vector3& dir, float& oLength, dtPolyRef polyRef, Vector3& oHitPosition) const
	{
		const dtMeshTile* tile = 0;
		const dtPoly*     poly = 0;
		if (dtStatusFailed(m_navQuery->getAttachedNavMesh()->getTileAndPolyByRef(polyRef, &tile, &poly)))
			return false;
		if (!tile)
			return false;

		const unsigned int  ip = (unsigned int)(poly - tile->polys);
		const dtPolyDetail* pd = &tile->detailMeshes[ip];

		float  resultLen = 1e30f;

		// Find height at the location.
		for (int j = 0; j < pd->triCount; ++j)
		{
			const unsigned char* t = &tile->detailTris[(pd->triBase + j) * 4];
			Echo::Vector3 v[3];
			for (int k = 0; k < 3; ++k)
			{
				float* tv = NULL;
				if (t[k] < poly->vertCount)
					v[k] = (Echo::Vector3&)tile->verts[poly->verts[t[k]] * 3];
				else
					v[k] = (Echo::Vector3&)tile->detailVerts[(pd->vertBase + (t[k] - poly->vertCount)) * 3];
			}

			float minDis;
			Echo::Ray      ray(startPos, dir);
			Echo::Triangle triangle;
			triangle.set(v[0], v[1], v[2]);
			if (ray.hitTri(triangle, minDis))
			{
				if (resultLen > minDis)
					resultLen = minDis;
			}
		}

		if (resultLen != 1e30f)
		{
			oLength = resultLen;
			oHitPosition = startPos + dir * resultLen;

			return true;
		}

		return false;
	}

	/// Ray cast the surface of the navigation mesh to find the nearest hit point
	/// @param[in]     startPos     The start position of the ray
	/// @param[in]	   dir		    The direction of the ray
	/// @param[in]     length	    The length of the ray
	/// @param[out]	   oHitPosition The hitposition of the ray with the navigation mesh surface
	bool Navigation::rayDetect(dtNavMeshQuery* query, const Vector3& startPos, const Vector3& dir, float& oLength, const dtQueryFilter* filter, Echo::Vector3& oHitPosition) const
	{
		//dtAssert(m_nav);

		Echo::Vector3 extends = Echo::Vector3(Echo::Math::Abs(dir.x), Echo::Math::Abs(dir.y), Echo::Math::Abs(dir.z)) * oLength * 0.5f;
		Echo::Vector3 center = startPos + dir * oLength * 0.5f;

		int		  polyCount;
		dtPolyRef polyRefs[512];
		if (DT_SUCCESS == query->queryPolygons(&center[0], &extends[0], filter, polyRefs, &polyCount, 512))
		{		
			float    resultLen = 1e30f;
			for (int i = 0; i < polyCount; i++)
			{
				float minDis;
				Vector3 tmpHitPosition;
				if (rayDetectPoly(startPos, dir, minDis, polyRefs[i], tmpHitPosition))
				{
					if (resultLen > minDis)
						resultLen = minDis;
				}
			}

			if (resultLen != 1e30f)
			{
				oLength = resultLen;
				oHitPosition = startPos + dir * resultLen;

				return true;
			}
		}

		return false;
	}

	// 射线检测(求最近点)
	bool Navigation::rayDetect(const Vector3& startPos, const Vector3& dir, float& oLength, Vector3& oHitPosition) const
	{
		if (m_isLoaded)
		{
			const float line_segment = 50.0f; //50米 一分段

			if (oLength < line_segment)
			{
				return rayDetect(m_navQuery, startPos, dir, oLength, &m_filter, oHitPosition);
			}
			else
			{
				int dive_time = (int)(oLength / line_segment);
				for (int i = 0; i < dive_time; ++i)
				{
					Echo::Vector3 newStarPos = startPos + dir * line_segment * Echo::Real(i);
					float new_length = line_segment;
					if (rayDetect(m_navQuery, newStarPos, dir, new_length, &m_filter, oHitPosition))
					{
						return true;
					}
				}

				Echo::Vector3 newStarPos = startPos + dir * line_segment * Echo::Real(dive_time);
				float new_length = oLength - line_segment * dive_time;
				if (new_length > 0.1f)
				{
					return rayDetect(m_navQuery, newStarPos, dir, new_length, &m_filter, oHitPosition);
				}
			}

			return false;
		}
		else
			return false;
	}

	bool Navigation::crowdIsAgentActive(const int agentId)
	{
		if (m_crowd == NULL)
		{
			return false;
		}

		int agentCount = m_crowd->getAgentCount();
		if (agentId < 0 || agentId >= agentCount)
		{
			return false;
		}

		const dtCrowdAgent* pAgent = m_crowd->getAgent(agentId);
		if (pAgent == NULL)
		{
			return false;
		}

		return pAgent->active != 0 ? true : false;
	}

	int Navigation::crowdAddAgent(const Vector3& position, float weight, float speed, float radius, float maxAcceleration)
	{
		float pos[3];
		pos[0] = position.x;
		pos[1] = position.y;
		pos[2] = position.z;
		dtCrowdAgentParams ap;
		memset(&ap, 0, sizeof(ap));

		ap.radius = Echo::Math::Max(m_agentRadius, radius);
		ap.height = m_agentHeight;
		ap.maxAcceleration = maxAcceleration;
		ap.maxSpeed = speed;
		ap.collisionQueryRange = ap.radius * 12.0f;
		ap.pathOptimizationRange = ap.radius * 30.0f;
		ap.updateFlags = 0;
		//if (m_toolParams.m_anticipateTurns)
		//	ap.updateFlags |= DT_CROWD_ANTICIPATE_TURNS;
		//if (m_toolParams.m_optimizeVis)
		//	ap.updateFlags |= DT_CROWD_OPTIMIZE_VIS;
		//if (m_toolParams.m_optimizeTopo)
		//	ap.updateFlags |= DT_CROWD_OPTIMIZE_TOPO;
		//if (m_toolParams.m_obstacleAvoidance)
		//	ap.updateFlags |= DT_CROWD_OBSTACLE_AVOIDANCE;
		//if (m_toolParams.m_separation)
		//	ap.updateFlags |= DT_CROWD_SEPARATION;
		//ap.obstacleAvoidanceType = (unsigned char)m_toolParams.m_obstacleAvoidanceType;
		//ap.separationWeight = m_toolParams.m_separationWeight;
		if (weight > 0.01f)
		{
			ap.updateFlags |= DT_CROWD_SEPARATION;
			ap.separationWeight = weight;
		}
		return m_crowd->addAgent(pos, &ap);
	}

	void Navigation::setDoCollide(bool docollide)
	{
		if (!m_crowd)
			return;
		m_crowd->setDoCollide(docollide);
	}

	// 查找离某位置最近寻路多边形
	bool Navigation::findNearestPoly(const Vector3& position, const Vector3& ext, Vector3& nearest)
	{
		if (!m_navMesh)
			return false;

		float pos[3];
		float extend[3];
		float nearestpos[3];
		memcpy(pos, &position, sizeof(float) * 3);
		memcpy(extend, &ext, sizeof(float) * 3);

		dtPolyRef		nearestRef = 0;
		if (m_navQuery->findNearestPoly(pos, extend, &m_filter, &nearestRef, nearestpos) != DT_SUCCESS)
		{
			return false;
		}
		if (nearestRef == 0)
		{
			return false;
		}
		nearest.x = nearestpos[0];
		nearest.y = nearestpos[1];
		nearest.z = nearestpos[2];
		return true;
	}

	bool Navigation::findNearestPolyBetween(const Vector3& start, const Vector3& end, Vector3& nearest, float deltaY, float step)
	{
		if (!m_navMesh)
			return false;

		Vector3 dir = start - end;
		float deltaDis = dir.len();
		if (deltaDis <= 1.f)
		{
			dir = Vector3::ONE;
			deltaDis = dir.len();
		}
		dir.normalize();
		if (step < 0.f)
		{
			step = 2.f;
		}
		step = step / 2;
		Vector3 ext = Vector3(step, step, step);
		ext.y += deltaY;
		Vector3 centerPos;
		float checkedDis = 0.f;
		float target[3];
		memcpy(target, &end, sizeof(float) * 3);
		float extend[3];
		memcpy(extend, &ext, sizeof(float) * 3);
		float nearestpos[3];
		bool find = false;
		dtPolyRef	 nearestRef = 0;
		while (checkedDis < deltaDis)
		{
			checkedDis += step * 2;
			centerPos = end + dir * (checkedDis - step);

			float center[3];
			memcpy(center, &centerPos, sizeof(float) * 3);

			if (m_navQuery->findNearestPoly(/*target,*/ center, extend, &m_filter, &nearestRef, nearestpos) == DT_SUCCESS && nearestRef != 0)
			{
				nearest.x = nearestpos[0];
				nearest.y = nearestpos[1];
				nearest.z = nearestpos[2];
				find = true;

				break;
			}
		}
		return find;
	}

	Echo::Vector3 Navigation::crowdGetAgentPosition(int nAgentIndex)
	{
		Vector3 position(Vector3::ZERO);
		if (m_crowd)
		{
			if (nAgentIndex < 0 || nAgentIndex >= m_crowd->getAgentCount())
			{
				return position;
			}

			const dtCrowdAgent* pAgent = m_crowd->getAgent(nAgentIndex);
			position.x = pAgent->npos[0];
			position.y = pAgent->npos[1];
			position.z = pAgent->npos[2];
		}

		return position;
	}

	void Navigation::crowdResetAgentPosition(int nAgentIndex, Echo::Vector3 pos)
	{
		if (m_crowd)
		{
			if (nAgentIndex < 0 || nAgentIndex >= m_crowd->getAgentCount())
			{
				return;
			}

			dtCrowdAgent* pAgent = m_crowd->getEditableAgent(nAgentIndex);
			pAgent->npos[0] = pos.x;
			pAgent->npos[1] = pos.y;
			pAgent->npos[2] = pos.z;
		}
	}

	void Navigation::crowdMoveAgentToword(const Vector3& dir, int nAgentIndex)
	{
		if (m_crowd)
		{
			m_crowd->requestMoveVelocity(nAgentIndex, (float*)&dir);
		}
	}

	void Navigation::crowdUpdateAllAgents(ui32 nElapes)
	{
		if (m_crowd)
		{
			m_crowd->update(nElapes*0.001f, NULL);
		}
	}

	void Navigation::crowdUpdateAgentOnly(ui32 nElapes, int idx)
	{
		if (m_crowd)
		{
			m_agentDebug.idx = idx;
			m_crowd->update(nElapes*0.001f, &m_agentDebug);
		}
	}

	void Navigation::findPath(const Vector3& startPos, const Vector3& endPos, float*& path, int& pathCount, int type /*=0*/)
	{
		memcpy(m_spos, &startPos, sizeof(float) * 3);
		memcpy(m_epos, &endPos, sizeof(float) * 3);
		if (!m_navMesh)
			return;

		EchoAssert(type >= 0 && type < DT_CROWD_MAX_QUERY_FILTER_TYPE);
		dtQueryFilter* filter = &m_filter;
		if (type > 0)
		{
			filter = m_crowd->getEditableFilter(type);
		}

		m_navQuery->findNearestPoly(m_spos, m_polyPickExt, filter, &m_startRef, 0);
		m_navQuery->findNearestPoly(m_epos, m_polyPickExt, filter, &m_endRef, 0);

		if (m_startRef && m_endRef)
		{
			m_navQuery->findPath(m_startRef, m_endRef, m_spos, m_epos, filter, m_polys, &m_npolys, MAX_POLYS);

			m_nsmoothPath = 0;

			if (m_npolys)
			{
				// Iterate over the path to find smooth path on the detail mesh surface.
				dtPolyRef polys[MAX_POLYS];
				memcpy(polys, m_polys, sizeof(dtPolyRef)*m_npolys);
				int npolys = m_npolys;

				float iterPos[3], targetPos[3];
				m_navQuery->closestPointOnPoly(m_startRef, m_spos, iterPos, nullptr);
				m_navQuery->closestPointOnPoly(polys[npolys - 1], m_epos, targetPos, nullptr);

				static const float STEP_SIZE = 0.5f;
				static const float SLOP = 0.01f;

				m_nsmoothPath = 0;

				dtVcopy(&m_smoothPath[m_nsmoothPath * 3], iterPos);
				m_nsmoothPath++;

				// Move towards target a small advancement at a time until target reached or
				// when ran out of memory to store the path.
				while (npolys && m_nsmoothPath < MAX_SMOOTH)
				{
					// Find location to steer towards.
					float steerPos[3];
					unsigned char steerPosFlag;
					dtPolyRef steerPosRef;

					if (!getSteerTarget(m_navQuery, iterPos, targetPos, SLOP,
						polys, npolys, steerPos, steerPosFlag, steerPosRef))
						break;

					bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
					bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

					// Find movement delta.
					float delta[3], len;
					dtVsub(delta, steerPos, iterPos);
					len = sqrtf(dtVdot(delta, delta));
					// If the steer target is end of path or off-mesh link, do not move past the location.
					if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
						len = 1;
					else
						len = STEP_SIZE / len;
					float moveTgt[3];
					dtVmad(moveTgt, iterPos, delta, len);

					// Move
					float result[3];
					dtPolyRef visited[16];
					int nvisited = 0;
					m_navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, filter,
						result, visited, &nvisited, 16);

					npolys = fixupCorridor(polys, npolys, MAX_POLYS, visited, nvisited);
					npolys = fixupShortcuts(polys, npolys, m_navQuery);
					float h = 0;
					if( DT_SUCCESS == m_navQuery->getPolyHeight(polys[0], result, &h))
						result[1] = h;

					dtVcopy(iterPos, result);

					// Handle end of path and off-mesh links when close enough.
					if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
					{
						// Reached end of path.
						dtVcopy(iterPos, targetPos);
						if (m_nsmoothPath < MAX_SMOOTH)
						{
							dtVcopy(&m_smoothPath[m_nsmoothPath * 3], iterPos);
							m_nsmoothPath++;
						}
						break;
					}
					else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
					{
						// Reached off-mesh connection.
						float startPos[3], endPos[3];

						// Advance the path up to and over the off-mesh connection.
						dtPolyRef prevRef = 0, polyRef = polys[0];
						int npos = 0;
						while (npos < npolys && polyRef != steerPosRef)
						{
							prevRef = polyRef;
							polyRef = polys[npos];
							npos++;
						}
						for (int i = npos; i < npolys; ++i)
							polys[i - npos] = polys[i];
						npolys -= npos;

						// Handle the connection.
						dtStatus status = m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
						if (dtStatusSucceed(status))
						{
							if (m_nsmoothPath < MAX_SMOOTH)
							{
								dtVcopy(&m_smoothPath[m_nsmoothPath * 3], startPos);
								m_nsmoothPath++;
								// Hack to make the dotted path not visible during off-mesh connection.
								if (m_nsmoothPath & 1)
								{
									dtVcopy(&m_smoothPath[m_nsmoothPath * 3], startPos);
									m_nsmoothPath++;
								}
							}
							// Move position at the other side of the off-mesh link.
							dtVcopy(iterPos, endPos);
							float eh = 0.0f;
							m_navQuery->getPolyHeight(polys[0], iterPos, &eh);
							iterPos[1] = eh;
						}
					}

					// Store results.
					if (m_nsmoothPath < MAX_SMOOTH)
					{
						dtVcopy(&m_smoothPath[m_nsmoothPath * 3], iterPos);
						m_nsmoothPath++;
					}
				}
			}

		}
		else
		{
			m_npolys = 0;
			m_nsmoothPath = 0;
		}

		path = m_smoothPath;
		pathCount = m_nsmoothPath;
	}

	void Navigation::findPath(const float spos[], const float espos[], float*& smoothPath, int & nsmoothPath, int include_flags)
	{
		if (!m_isLoaded)
			return;

		dtQueryFilter filter;
		filter.setIncludeFlags(include_flags);

		dtPolyRef					startRef = 0;
		dtPolyRef					endRef = 0;

		if (DT_SUCCESS != m_navQuery->findNearestPoly(spos, m_polyPickExt, &filter, &startRef, 0))
			return;

		if (DT_SUCCESS != m_navQuery->findNearestPoly(espos, m_polyPickExt, &filter, &endRef, 0))
			return;

		nsmoothPath = 0;
		int							npolygons = 0;
		dtPolyRef					polygons[MAX_POLYS] = {};
		m_navQuery->findPath(startRef, endRef, spos, espos, &filter, polygons, &npolygons, MAX_POLYS);

		if (npolygons)
		{
			// Iterate over the path to find smooth path on the detail mesh surface.
			dtPolyRef polys[MAX_POLYS];
			memcpy(polys, polygons, sizeof(dtPolyRef)*npolygons);
			int npolys = npolygons;

			float iterPos[3], targetPos[3];
			m_navQuery->closestPointOnPoly(startRef, spos, iterPos, nullptr);
			m_navQuery->closestPointOnPoly(polys[npolys - 1], espos, targetPos, nullptr);

			static const float STEP_SIZE = 0.5f;
			static const float SLOP = 0.01f;

			nsmoothPath = 0;

			dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
			nsmoothPath++;

			// Move towards target a small advancement at a time until target reached or
			// when ran out of memory to store the path.
			while (npolys && nsmoothPath < MAX_SMOOTH)
			{
				// Find location to steer towards.
				float steerPos[3];
				unsigned char steerPosFlag;
				dtPolyRef steerPosRef;

				if (!getSteerTarget(m_navQuery, iterPos, targetPos, SLOP,
					polys, npolys, steerPos, steerPosFlag, steerPosRef))
					break;

				bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) ? true : false;
				bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ? true : false;

				// Find movement delta.
				float delta[3], len;
				dtVsub(delta, steerPos, iterPos);
				len = sqrtf(dtVdot(delta, delta));
				// If the steer target is end of path or off-mesh link, do not move past the location.
				if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
					len = 1;
				else
					len = STEP_SIZE / len;
				float moveTgt[3];
				dtVmad(moveTgt, iterPos, delta, len);

				// Move
				float result[3];
				dtPolyRef visited[16];
				int nvisited = 0;
				m_navQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &filter,
					result, visited, &nvisited, 16);

				npolys = fixupCorridor(polys, npolys, MAX_POLYS, visited, nvisited);
				float h = 0;
				m_navQuery->getPolyHeight(polys[0], result, &h);
				result[1] = h;
				dtVcopy(iterPos, result);

				// Handle end of path and off-mesh links when close enough.
				if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
				{
					// Reached end of path.
					dtVcopy(iterPos, targetPos);
					if (nsmoothPath < MAX_SMOOTH)
					{
						dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
						nsmoothPath++;
					}
					break;
				}
				else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
				{
					// Reached off-mesh connection.
					float start_pos[3], end_pos[3];

					// Advance the path up to and over the off-mesh connection.
					dtPolyRef prevRef = 0, polyRef = polys[0];
					int npos = 0;
					while (npos < npolys && polyRef != steerPosRef)
					{
						prevRef = polyRef;
						polyRef = polys[npos];
						npos++;
					}
					for (int i = npos; i < npolys; ++i)
						polys[i - npos] = polys[i];
					npolys -= npos;

					// Handle the connection.
					dtStatus status = m_navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, start_pos, end_pos);
					if (dtStatusSucceed(status))
					{
						if (nsmoothPath < MAX_SMOOTH)
						{
							dtVcopy(&smoothPath[nsmoothPath * 3], start_pos);
							nsmoothPath++;
							// Hack to make the dotted path not visible during off-mesh connection.
							if (nsmoothPath & 1)
							{
								dtVcopy(&smoothPath[nsmoothPath * 3], start_pos);
								nsmoothPath++;
							}
						}
						// Move position at the other side of the off-mesh link.
						dtVcopy(iterPos, end_pos);
						float eh = 0.0f;
						m_navQuery->getPolyHeight(polys[0], iterPos, &eh);
						iterPos[1] = eh;
					}
				}

				// Store results.
				if (nsmoothPath < MAX_SMOOTH)
				{
					dtVcopy(&smoothPath[nsmoothPath * 3], iterPos);
					nsmoothPath++;
				}
			}
		}
	}

	#define RVec3(x) (const float*)&x

	// Casts a 'walkability' ray along the surface of the navigation mesh from the start position toward the end position.
	bool Navigation::rayCast(const Vector3& startPos, const Vector3& dir, float& distance, Vector3& endPos, int include_flags)
	{
		if (!m_isLoaded)
			return false;

	#ifdef ECHO_DEBUG
		if (distance > 200.f)
			EchoLogError("Navigation::rayCast distance [%f] is too big.", distance);
	#endif

		dtQueryFilter filter;
		filter.setIncludeFlags(include_flags);

		// 
		dtPolyRef startRef = -1;
		if (DT_SUCCESS != m_navQuery->findNearestPoly(RVec3(startPos),
			m_polyPickExt,
			&filter,
			&startRef, 0))
		{
			return false;
		}

		Vector3 normalizeDir;
		dir.Normalize(normalizeDir, dir);
		endPos = startPos + normalizeDir * distance;

		Vector3   hitNormal;
		dtPolyRef polygons[MAX_POLYS] = {};
		int		  pathCount = 0;
		float	  tDistance;

		dtStatus status = m_navQuery->raycast(startRef,
			RVec3(startPos),
			RVec3(endPos),
			&filter,
			&tDistance,
			(float*)&hitNormal,
			polygons,
			&pathCount,
			MAX_POLYS);
		if (!dtStatusSucceed(status))
		{
			return false;
		}

		if (tDistance != FLT_MAX)
		{
			endPos = startPos + (endPos - startPos) * tDistance;
			distance = (endPos - startPos).len();

			return true;
		}
		else
		{
			EchoAssert(pathCount > 0);
			Vector3 detectStartPos = endPos - Vector3::UNIT_Y * 10000.f;
			float detectDistance = 20000.f;
			if (rayDetectPoly(detectStartPos, Vector3::UNIT_Y, detectDistance, polygons[pathCount-1], endPos))
			{
				distance = (endPos - startPos).len();

				return true;
			}
			else
			{
				EchoAssert(false);
			}
		}

		return false;
	}

	void Navigation::setCrowdIncludeFlag(int type, unsigned int nFlag)
	{
		EchoAssert(type >= 0 && type < DT_CROWD_MAX_QUERY_FILTER_TYPE);
		if (type == 0)
		{
			m_filter.setIncludeFlags(nFlag);
		}

		if (m_crowd)
		{
			dtQueryFilter* pFilter = m_crowd->getEditableFilter(type);
			pFilter->setIncludeFlags(nFlag);
		}
	}

	void Navigation::setAgentIncludeFlagType(int agentId, unsigned char type)
	{
		int agentCount = m_crowd->getAgentCount();
		if (agentId < 0 || agentId >= agentCount)
		{
			return;
		}

		dtCrowdAgent* pAgent = m_crowd->getEditableAgent(agentId);
		if (pAgent == NULL)
		{
			return;
		}

		pAgent->params.queryFilterType = type;
	}

}