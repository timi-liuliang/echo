#include "Engine/Core.h"
#include "engine/core/util/PathUtil.h"
#include "NavigationSolo.h"
#include <thirdparty/recast/recast/recast.h>
#include "DetourNavMeshBuilder.h"
#include "DetourCommon.h"
#include "InputGeometryData.h"
#include "engine/core/Util/LogManager.h"
#include "engine/core/io/DataStream.h"
#include "DetourCrowd.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include <math.h>
#include "BuildContext.h"

namespace Echo
{
	// 标识三角形所区域
	void EchoMarkWalkableTriangles(rcContext* ctx, const float* verts, int nv, const int* tris, int nt, const rcChunkyTriInfo* origTriInfos, unsigned char* areas)
	{
		rcIgnoreUnused(ctx);
		rcIgnoreUnused(nv);

		for (int i = 0; i < nt; ++i)
		{
			const int* tri = &tris[i * 3];
			Triangle lordTri((const Vector3&)verts[tri[0] * 3], (const Vector3&)verts[tri[1] * 3], (const Vector3&)verts[tri[2] * 3]);

			// 计算垂直角度
			Echo::Vector3 normal = lordTri.GetNormal(); normal.normalize();
			float hAngle = acos(normal.dot(Echo::Vector3::UNIT_Y)) * Echo::Math::RAD2DEG;
			bool  isWalkEnable = hAngle < origTriInfos[i].walkableSlopeAngle;
			if (isWalkEnable)
				areas[i] = origTriInfos[i].area;
			else
				areas[i] = RC_NULL_AREA;
		}
	}

	// 构造函数
	NavigationSolo::NavigationSolo()
		: Navigation()
		, m_triareas(NULL)
		, m_solid(NULL)
		, m_chf(NULL)
		, m_cset(NULL)
		, m_pmesh(NULL)
		, m_dmesh(NULL)
		, m_navData(nullptr)
		, m_navDataSize(0)
	{
	}

	// 析构函数
	NavigationSolo::~NavigationSolo()
	{
		cleanup();
	}

	// 构建
	bool NavigationSolo::build(float agentRadius, float agentHeight, float stepHeight)
	{
		EchoAssert(m_geom);

		cleanup();

		m_agentRadius = agentRadius;
		m_agentHeight = agentHeight;
		m_agentMaxSlope = 45.f;			// 未使用，可行走角度由模型配置决定
		m_agentHeight = stepHeight;

		const float* bmin = (float*)&(m_geom->getAABB().vMin);
		const float* bmax = (float*)&(m_geom->getAABB().vMax);
		const float* verts = m_geom->getVerts();
		const int nverts = m_geom->getVertCount();
		const int* tris = m_geom->getTris();
		const int ntris = m_geom->getTriCount();
		const rcChunkyTriInfo* triInfos = m_geom->getTriInfos();

		rcConfig buildConfig;
		//
		// Step 1. Initialize build config.
		//

		// Init build configuration from GUI
		memset(&buildConfig, 0, sizeof(buildConfig));
		buildConfig.cs = m_cellSize;
		buildConfig.ch = m_cellHeight;
		buildConfig.walkableSlopeAngle = m_agentMaxSlope;
		buildConfig.walkableHeight = (int)ceilf(m_agentHeight / buildConfig.ch);
		buildConfig.walkableClimb = (int)floorf(m_agentMaxClimb / buildConfig.ch);
		buildConfig.walkableRadius = (int)ceilf(m_agentRadius / buildConfig.cs);
		buildConfig.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
		buildConfig.maxSimplificationError = m_edgeMaxError;
		buildConfig.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
		buildConfig.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
		buildConfig.maxVertsPerPoly = (int)m_vertsPerPoly;
		buildConfig.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
		buildConfig.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;

		// Set the area where the navigation will be build.
		// Here the bounds of the input mesh are used, but the
		// area could be specified by an user defined box, etc.
		rcVcopy(buildConfig.bmin, bmin);
		rcVcopy(buildConfig.bmax, bmax);
		rcCalcGridSize(buildConfig.bmin, buildConfig.bmax, buildConfig.cs, &buildConfig.width, &buildConfig.height);

		//
		// Step 2. Rasterize input polygon soup.
		//

		// Allocate voxel heightfield where we rasterize our input data to.
		m_solid = rcAllocHeightfield();
		if (!m_solid)
		{
			EchoLogError("buildNavigation: Out of memory 'solid'.");
			return false;
		}
		if (!rcCreateHeightfield(m_ctx, *m_solid, buildConfig.width, buildConfig.height, buildConfig.bmin, buildConfig.bmax, buildConfig.cs, buildConfig.ch))
		{
			EchoLogError("buildNavigation: Could not create solid heightfield.");
			return false;
		}

		// Allocate array that can hold triangle area types.
		// If you have multiple meshes you need to process, allocate
		// and array which can hold the max number of triangles you need to process.
		m_triareas = (unsigned char*)EchoMalloc(ntris);
		if (!m_triareas)
		{
			EchoLogError("buildNavigation: Out of memory 'm_triareas' (%d).", ntris);
			return false;
		}

		// Find triangles which are walkable based on their slope and rasterize them.
		// If your input data is multiple meshes, you can transform them here, calculate
		// the are type for each of the meshes and rasterize them.
		memset(m_triareas, 0, ntris*sizeof(unsigned char));

		// 此处与原版Recast相比，做稍微改动。使用原三角形决定区域
		EchoMarkWalkableTriangles(m_ctx, verts, nverts, tris, ntris, triInfos, m_triareas);
		rcRasterizeTriangles(m_ctx, verts, nverts, tris, m_triareas, ntris, *m_solid, buildConfig.walkableClimb);

		EchoSafeFree(m_triareas);
		m_triareas = 0;

		//
		// Step 3. Filter walkables surfaces.
		//

		// Once all geoemtry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		rcFilterLowHangingWalkableObstacles(m_ctx, buildConfig.walkableClimb, *m_solid);
		rcFilterLedgeSpans(m_ctx, buildConfig.walkableHeight, buildConfig.walkableClimb, *m_solid);
		rcFilterWalkableLowHeightSpans(m_ctx, buildConfig.walkableHeight, *m_solid);


		//
		// Step 4. Partition walkable surface to simple regions.
		//

		// Compact the heightfield so that it is faster to handle from now on.
		// This will result more cache coherent data as well as the neighbours
		// between walkable cells will be calculated.
		m_chf = rcAllocCompactHeightfield();
		if (!m_chf)
		{
			EchoLogError("buildNavigation: Out of memory 'chf'.");
			return false;
		}
		if (!rcBuildCompactHeightfield(m_ctx, buildConfig.walkableHeight, buildConfig.walkableClimb, *m_solid, *m_chf))
		{
			EchoLogError("buildNavigation: Could not build compact data.");
			return false;
		}

		rcFreeHeightField(m_solid);
		m_solid = 0;

		// ------------------------------------------------------------------------------------------
		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(m_ctx, buildConfig.walkableRadius, *m_chf))
		{
			EchoLogError("buildNavigation: Could not erode.");
			return false;
		}

		// (Optional) Mark areas.
		//const ConvexVolume* vols = m_geom->getConvexVolumes();
		//for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
		//	rcMarkConvexPolyArea(&m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);
		//---------------------------------------------------------------------------------------------------------------------------------

		if (m_monotonePartitioning)
		{
			// Partition the walkable surface into simple regions without holes.
			// Monotone partitioning does not need distancefield.
			if (!rcBuildRegionsMonotone(m_ctx, *m_chf, 0, buildConfig.minRegionArea, buildConfig.mergeRegionArea))
			{
				EchoLogError("buildNavigation: Could not build regions.");
				return false;
			}
		}
		else
		{
			// Prepare for region partitioning, by calculating distance field along the walkable surface.
			if (!rcBuildDistanceField(m_ctx, *m_chf))
			{
				EchoLogError("buildNavigation: Could not build distance field.");
				return false;
			}

			// Partition the walkable surface into simple regions without holes.
			if (!rcBuildRegions(m_ctx, *m_chf, 0, buildConfig.minRegionArea, buildConfig.mergeRegionArea))
			{
				EchoLogError("buildNavigation: Could not build regions.");
				return false;
			}
		}

		//
		// Step 5. Trace and simplify region contours.
		//

		// Create contours.
		m_cset = rcAllocContourSet();
		if (!m_cset)
		{
			EchoLogError("buildNavigation: Out of memory 'cset'.");
			return false;
		}
		if (!rcBuildContours(m_ctx, *m_chf, buildConfig.maxSimplificationError, buildConfig.maxEdgeLen, *m_cset))
		{
			EchoLogError("buildNavigation: Could not create contours.");
			return false;
		}

		//
		// Step 6. Build polygons mesh from contours.
		//

		// Build polygon navmesh from the contours.
		m_pmesh = rcAllocPolyMesh();
		if (!m_pmesh)
		{
			EchoLogError("buildNavigation: Out of memory 'pmesh'.");
			return false;
		}
		if (!rcBuildPolyMesh(m_ctx, *m_cset, buildConfig.maxVertsPerPoly, *m_pmesh))
		{
			EchoLogError("buildNavigation: Could not triangulate contours.");
			return false;
		}

		//
		// Step 7. Create detail mesh which allows to access approximate height on each polygon.
		//

		m_dmesh = rcAllocPolyMeshDetail();
		if (!m_dmesh)
		{
			EchoLogError("buildNavigation: Out of memory 'pmdtl'.");
			return false;
		}

		if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, buildConfig.detailSampleDist, buildConfig.detailSampleMaxError, *m_dmesh))
		{
			EchoLogError("buildNavigation: Could not build detail mesh.");
			return false;
		}

		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;

		// At this point the navigation mesh data is ready, you can access it from m_pmesh.
		// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.

		//
		// (Optional) Step 8. Create Detour data from Recast poly mesh.
		//

		// The GUI may allow more max points per polygon than Detour can handle.
		// Only build the detour navmesh if we do not exceed the limit.
		if (buildConfig.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
		{
			unsigned char* navData = 0;
			int navDataSize = 0;

			// Update poly flags from areas.
			for (int i = 0; i < m_pmesh->npolys; ++i)
			{
				if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
					m_pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

				if (m_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND ||
					m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
					m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_TYPE_0)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_TYPE_0;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_TYPE_1)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_TYPE_1;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_TYPE_2)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_TYPE_2;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_TYPE_3)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_TYPE_3;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_TYPE_4)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_TYPE_4;
				}
				else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_TYPE_5)
				{
					m_pmesh->flags[i] = SAMPLE_POLYFLAGS_TYPE_5;
				}
			}

			dtNavMeshCreateParams params;
			memset(&params, 0, sizeof(params));
			params.verts = m_pmesh->verts;
			params.vertCount = m_pmesh->nverts;
			params.polys = m_pmesh->polys;
			params.polyAreas = m_pmesh->areas;
			params.polyFlags = m_pmesh->flags;
			params.polyCount = m_pmesh->npolys;
			params.nvp = m_pmesh->nvp;
			params.detailMeshes = m_dmesh->meshes;
			params.detailVerts = m_dmesh->verts;
			params.detailVertsCount = m_dmesh->nverts;
			params.detailTris = m_dmesh->tris;
			params.detailTriCount = m_dmesh->ntris;
			params.offMeshConVerts = m_geom->getOffMeshConnectionVerts();
			params.offMeshConRad = m_geom->getOffMeshConnectionRads();
			params.offMeshConDir = m_geom->getOffMeshConnectionDirs();
			params.offMeshConAreas = m_geom->getOffMeshConnectionAreas();
			params.offMeshConFlags = m_geom->getOffMeshConnectionFlags();
			params.offMeshConUserID = m_geom->getOffMeshConnectionId();
			params.offMeshConCount = m_geom->getOffMeshConnectionCount();
			params.walkableHeight = m_agentHeight;
			params.walkableRadius = m_agentRadius;
			params.walkableClimb = m_agentMaxClimb;
			rcVcopy(params.bmin, m_pmesh->bmin);
			rcVcopy(params.bmax, m_pmesh->bmax);
			params.cs = buildConfig.cs;
			params.ch = buildConfig.ch;
			params.buildBvTree = true;


			if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
			{
				EchoLogError("Could not build Detour navmesh.");
				return false;
			}

			// backup a navigation mesh data for export
			EchoSafeFree(m_navData);

			m_navData = (unsigned char*)EchoMalloc(navDataSize);
			memcpy(m_navData, navData, navDataSize);
			m_navDataSize = navDataSize;
			// ------------------------------------------------------

			m_navMesh = dtAllocNavMesh();
			if (!m_navMesh)
			{
				dtFree(navData);
				EchoLogError("Could not create Detour navmesh");
				return false;
			}

			dtStatus status;

			status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status))
			{
				dtFree(navData);
				EchoLogError("Could not init Detour navmesh");
				return false;
			}

			status = m_navQuery->init(m_navMesh, 2048);
			if (dtStatusFailed(status))
			{
				EchoLogError("Could not init Detour navmesh query");
				return false;
			}

			crowdInit();
		}

		return true;
	}

	// 清空数据
	void NavigationSolo::cleanup()
	{
		EchoSafeFree(m_triareas);
		m_triareas = 0;
		rcFreeHeightField(m_solid);
		m_solid = 0;
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
		rcFreePolyMesh(m_pmesh);
		m_pmesh = 0;
		rcFreePolyMeshDetail(m_dmesh);
		m_dmesh = 0;
		dtFreeNavMesh(m_navMesh);
		m_navMesh = 0;

		EchoSafeFree(m_navData);
		m_navDataSize = 0;
	}

	// 加载
	void NavigationSolo::load(const String& filePath)
	{
		DataStream* pStream = IO::instance()->open(filePath);
		if (pStream)
		{
			int navDataSize = 0;
			pStream->read(&navDataSize, sizeof(int));

			unsigned char* navData = (unsigned char*)dtAlloc(navDataSize, DT_ALLOC_PERM);
			EchoAssert(navData);
			pStream->read(navData, navDataSize);

			cleanup();

			m_navMesh = dtAllocNavMesh();
			if (!m_navMesh)
			{
				dtFree(navData);
				EchoLogError("Could not create Detour navmesh");
				return;
			}

			dtStatus status;

			status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
			if (dtStatusFailed(status))
			{
				dtFree(navData);
				EchoLogError("Could not init Detour navmesh");
				return;
			}

			status = m_navQuery->init(m_navMesh, 2048);
			if (dtStatusFailed(status))
			{
				EchoLogError("Could not init Detour navmesh query");
				return;
			}

			crowdInit();

			m_isLoaded = true;

			EchoSafeDelete(pStream, DataStream);
		}
	}

	// 保存
	void NavigationSolo::save(const char* savePath)
	{
#ifdef ECHO_EDITOR_MODE
		if (m_navData && m_navDataSize)
		{
			FILE* fileHandle = fopen(savePath, "wb");
			if (fileHandle)
			{
				fwrite(&m_navDataSize, sizeof(ui32), 1, fileHandle);
				fwrite(m_navData, m_navDataSize, 1, fileHandle);

				fflush(fileHandle);
				fclose(fileHandle);
			}
		}
		else
		{
			if (PathUtil::IsFileExist(savePath))
				PathUtil::DelPath(savePath);
		}
#endif
	}
}
