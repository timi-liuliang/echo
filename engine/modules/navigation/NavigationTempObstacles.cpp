#include <math.h>
#include "engine/core/util/PathUtil.h"
#include "engine/core/log/Log.h"
#include "engine/core/io/IO.h"
#include <engine/core/memory/MemAllocDef.h>
#include <engine/core/base/echo_def.h>
#include <thirdparty/recast/Recast/Recast.h>
#include <thirdparty/recast/Recast/DetourNavMeshBuilder.h>
#include <thirdparty/recast/Recast/DetourCommon.h>
#include <thirdparty/recast/Recast/DetourCrowd.h>
#include <thirdparty/recast/Recast/DetourTileCache.h>
#include <thirdparty/recast/Recast/DetourTileCacheBuilder.h>
#include "InputGeometryData.h"
#include "NavigationTempObstacles.h"
#include "fastlz.h"
#include "BuildContext.h"
#include "ChunkyTriMesh.h"

namespace Echo
{
	// 标识三角形所区域
	extern void EchoMarkWalkableTriangles(rcContext* ctx, const float* verts, int nv, const int* tris, int nt, const rcChunkyTriInfo* origTriInfos, unsigned char* areas);
	// This value specifies how many layers (or "floors") each navmesh tile is expected to have.
	static const int EXPECTED_LAYERS_PER_TILE = 4;
	static const int MAX_LAYERS = 32;

	// TileCacheData
	struct TileCacheData
	{
		unsigned char* data;
		int dataSize;
	};

	struct FastLZCompressor : public dtTileCacheCompressor
	{
		virtual int maxCompressedSize(const int bufferSize)
		{
			return (int)(bufferSize* 1.05f);
		}

		virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
			unsigned char* compressed, const int /*maxCompressedSize*/, int* compressedSize)
		{
			*compressedSize = fastlz_compress((const void *const)buffer, bufferSize, compressed);
			return DT_SUCCESS;
		}

		virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
			unsigned char* buffer, const int maxBufferSize, int* bufferSize)
		{
			*bufferSize = fastlz_decompress(compressed, compressedSize, buffer, maxBufferSize);
			return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
		}
	};

	struct LinearAllocator : public dtTileCacheAlloc
	{
		unsigned char* buffer;
		size_t capacity;
		size_t top;
		size_t high;

		LinearAllocator(const size_t cap) : buffer(0), capacity(0), top(0), high(0)
		{
			resize(cap);
		}

		~LinearAllocator()
		{
			dtFree(buffer);
		}

		void resize(const size_t cap)
		{
			if (buffer) dtFree(buffer);
			buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
			capacity = cap;
		}

		virtual void reset()
		{
			high = dtMax(high, top);
			top = 0;
		}

		virtual void* alloc(const size_t size)
		{
			if (!buffer)
				return 0;
			if (top + size > capacity)
				return 0;
			unsigned char* mem = &buffer[top];
			top += size;
			return mem;
		}

		virtual void free(void* /*ptr*/)
		{
			// Empty
		}
	};

	/**
	 * 模型处理
	 */
	struct MeshProcess : public dtTileCacheMeshProcess
	{
		InputGeometryData* m_geom;

		inline MeshProcess() : m_geom(0)
		{
		}

		inline void init(InputGeometryData* geom)
		{
			m_geom = geom;
		}

		virtual void process(struct dtNavMeshCreateParams* params,
			unsigned char* polyAreas, unsigned short* polyFlags)
		{
			// Update poly flags from areas.
			for (int i = 0; i < params->polyCount; ++i)
			{
				if (polyAreas[i] == DT_TILECACHE_WALKABLE_AREA)
					polyAreas[i] = SAMPLE_POLYAREA_GROUND;

				if (polyAreas[i] == SAMPLE_POLYAREA_GROUND ||
					polyAreas[i] == SAMPLE_POLYAREA_GRASS ||
					polyAreas[i] == SAMPLE_POLYAREA_ROAD)
				{
					polyFlags[i] = SAMPLE_POLYFLAGS_WALK;
				}
				else if (polyAreas[i] == SAMPLE_POLYAREA_WATER)
				{
					polyFlags[i] = SAMPLE_POLYFLAGS_SWIM;
				}
				else if (polyAreas[i] == SAMPLE_POLYAREA_DOOR)
				{
					polyFlags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
				}
			}

			// Pass in off-mesh connections.
			if (m_geom)
			{
				params->offMeshConVerts = m_geom->getOffMeshConnectionVerts();
				params->offMeshConRad = m_geom->getOffMeshConnectionRads();
				params->offMeshConDir = m_geom->getOffMeshConnectionDirs();
				params->offMeshConAreas = m_geom->getOffMeshConnectionAreas();
				params->offMeshConFlags = m_geom->getOffMeshConnectionFlags();
				params->offMeshConUserID = m_geom->getOffMeshConnectionId();
				params->offMeshConCount = m_geom->getOffMeshConnectionCount();
			}
		}
	};

	static int calcLayerBufferSize(const int gridWidth, const int gridHeight)
	{
		const int headerSize = dtAlign4(sizeof(dtTileCacheLayerHeader));
		const int gridSize = gridWidth * gridHeight;
		return headerSize + gridSize * 4;
	}

	struct RasterizationContext
	{
		RasterizationContext() :
			solid(0),
			triareas(0),
			lset(0),
			chf(0),
			ntiles(0)
		{
			memset(tiles, 0, sizeof(TileCacheData)*MAX_LAYERS);
		}

		~RasterizationContext()
		{
			rcFreeHeightField(solid);
			delete[] triareas;
			rcFreeHeightfieldLayerSet(lset);
			rcFreeCompactHeightfield(chf);
			for (int i = 0; i < MAX_LAYERS; ++i)
			{
				dtFree(tiles[i].data);
				tiles[i].data = 0;
			}
		}

		rcHeightfield* solid;
		unsigned char* triareas;
		rcHeightfieldLayerSet* lset;
		rcCompactHeightfield* chf;
		TileCacheData tiles[MAX_LAYERS];
		int ntiles;
	};

	// 构造函数
	NavigationTempObstacles::NavigationTempObstacles()
		: Navigation()
		, m_tileCache(nullptr)
		, m_cacheCompressedSize(0)
		, m_cacheLayerCount(0)
		, m_cacheBuildMemUsage(0)
		, m_maxTiles(0)
		, m_maxPolysPerTile(0)
		, m_tileSize(48)
		, m_filterLedgeSpans(true)
		, m_filterLowHangingObstacles(true)
		, m_filterWalkableLowHeightSpans(true)
		, m_tileCacheData(nullptr)
		, m_navMeshData(nullptr)
	{
		m_talloc = EchoNew(LinearAllocator(32000));
		m_tcomp = EchoNew(FastLZCompressor);
		m_tmproc = EchoNew(MeshProcess);
	}

	// 析构
	NavigationTempObstacles::~NavigationTempObstacles()
	{
		cleanup();

		EchoSafeDelete(m_talloc, LinearAllocator);
		EchoSafeDelete(m_tcomp, FastLZCompressor);
		EchoSafeDelete(m_tmproc, MeshProcess);
	}

	void NavigationTempObstacles::cleanup()
	{
		dtFreeNavMesh(m_navMesh);	m_navMesh = nullptr;
		dtFreeTileCache(m_tileCache); m_tileCache = nullptr;
		dtFreeNavMeshData(m_navMeshData); m_navMeshData = nullptr;
		dtFreeTileCacheData(m_tileCacheData); m_tileCacheData = nullptr;
	}

	// 每帧更新
	void NavigationTempObstacles::update(float delta)
	{
		if (m_tileCache)
			m_tileCache->update(delta, m_navMesh);
	}

	int NavigationTempObstacles::rasterizeTileLayers(const rcChunkyTriMesh* chunkyMesh, const int tx, const int ty, const rcConfig& cfg, TileCacheData* tiles, const int maxTiles)
	{
		if (!m_geom)
		{
			EchoLogError("buildTile: Input mesh is not specified.");
			return 0;
		}

		FastLZCompressor comp;
		RasterizationContext rc;

		const float* verts = m_geom->getVerts();
		const int nverts = m_geom->getVertCount();

		// Tile bounds.
		const float tcs = cfg.tileSize * cfg.cs;

		rcConfig tcfg;
		memcpy(&tcfg, &cfg, sizeof(tcfg));

		tcfg.bmin[0] = cfg.bmin[0] + tx*tcs;
		tcfg.bmin[1] = cfg.bmin[1];
		tcfg.bmin[2] = cfg.bmin[2] + ty*tcs;
		tcfg.bmax[0] = cfg.bmin[0] + (tx + 1)*tcs;
		tcfg.bmax[1] = cfg.bmax[1];
		tcfg.bmax[2] = cfg.bmin[2] + (ty + 1)*tcs;
		tcfg.bmin[0] -= tcfg.borderSize*tcfg.cs;
		tcfg.bmin[2] -= tcfg.borderSize*tcfg.cs;
		tcfg.bmax[0] += tcfg.borderSize*tcfg.cs;
		tcfg.bmax[2] += tcfg.borderSize*tcfg.cs;

		// Allocate voxel heightfield where we rasterize our input data to.
		rc.solid = rcAllocHeightfield();
		if (!rc.solid)
		{
			EchoLogError("buildNavigation: Out of memory 'solid'.");
			return 0;
		}
		if (!rcCreateHeightfield(m_ctx, *rc.solid, tcfg.width, tcfg.height, tcfg.bmin, tcfg.bmax, tcfg.cs, tcfg.ch))
		{
			EchoLogError("buildNavigation: Could not create solid heightfield.");
			return 0;
		}

		// Allocate array that can hold triangle flags.
		// If you have multiple meshes you need to process, allocate
		// and array which can hold the max number of triangles you need to process.
		rc.triareas = new unsigned char[chunkyMesh->maxTrisPerChunk];
		if (!rc.triareas)
		{
			EchoLogError("buildNavigation: Out of memory 'm_triareas' (%d).", chunkyMesh->maxTrisPerChunk);
			return 0;
		}

		float tbmin[2], tbmax[2];
		tbmin[0] = tcfg.bmin[0];
		tbmin[1] = tcfg.bmin[2];
		tbmax[0] = tcfg.bmax[0];
		tbmax[1] = tcfg.bmax[2];
		int cid[2048];// TODO: Make grow when returning too many items.
		const int ncid = rcGetChunksOverlappingRect(chunkyMesh, tbmin, tbmax, cid, 2048);
		if (!ncid)
		{
			return 0; // empty
		}

		for (int i = 0; i < ncid; ++i)
		{
			const rcChunkyTriMeshNode& node = chunkyMesh->nodes[cid[i]];
			const int* tris = &chunkyMesh->tris[node.i * 3];
			const rcChunkyTriInfo* triInfos = &chunkyMesh->triInfos[node.i];
			const int ntris = node.n;

			memset(rc.triareas, 0, ntris*sizeof(unsigned char));
			//rcMarkWalkableTriangles(m_ctx, tcfg.walkableSlopeAngle,
			//	verts, nverts, tris, ntris, rc.triareas);

			EchoMarkWalkableTriangles(m_ctx, verts, nverts, tris, ntris, triInfos, rc.triareas);

			if (!rcRasterizeTriangles(m_ctx, verts, nverts, tris, rc.triareas, ntris, *rc.solid, tcfg.walkableClimb))
				return 0;
		}

		// Once all geometry is rasterized, we do initial pass of filtering to
		// remove unwanted overhangs caused by the conservative rasterization
		// as well as filter spans where the character cannot possibly stand.
		if (m_filterLowHangingObstacles)
			rcFilterLowHangingWalkableObstacles(m_ctx, tcfg.walkableClimb, *rc.solid);
		if (m_filterLedgeSpans)
			rcFilterLedgeSpans(m_ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid);
		if (m_filterWalkableLowHeightSpans)
			rcFilterWalkableLowHeightSpans(m_ctx, tcfg.walkableHeight, *rc.solid);


		rc.chf = rcAllocCompactHeightfield();
		if (!rc.chf)
		{
			EchoLogError("buildNavigation: Out of memory 'chf'.");
			return 0;
		}
		if (!rcBuildCompactHeightfield(m_ctx, tcfg.walkableHeight, tcfg.walkableClimb, *rc.solid, *rc.chf))
		{
			EchoLogError("buildNavigation: Could not build compact data.");
			return 0;
		}

		// Erode the walkable area by agent radius.
		if (!rcErodeWalkableArea(m_ctx, tcfg.walkableRadius, *rc.chf))
		{
			EchoLogError("buildNavigation: Could not erode.");
			return 0;
		}

		rc.lset = rcAllocHeightfieldLayerSet();
		if (!rc.lset)
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'lset'.");
			return 0;
		}
		if (!rcBuildHeightfieldLayers(m_ctx, *rc.chf, tcfg.borderSize, tcfg.walkableHeight, *rc.lset))
		{
			m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build heighfield layers.");
			return 0;
		}

		rc.ntiles = 0;
		for (int i = 0; i < rcMin(rc.lset->nlayers, MAX_LAYERS); ++i)
		{
			TileCacheData* tile = &rc.tiles[rc.ntiles++];
			const rcHeightfieldLayer* layer = &rc.lset->layers[i];

			// Store header
			dtTileCacheLayerHeader header;
			header.magic = DT_TILECACHE_MAGIC;
			header.version = DT_TILECACHE_VERSION;

			// Tile layer location in the navmesh.
			header.tx = tx;
			header.ty = ty;
			header.tlayer = i;
			dtVcopy(header.bmin, layer->bmin);
			dtVcopy(header.bmax, layer->bmax);

			// Tile info.
			header.width = (unsigned char)layer->width;
			header.height = (unsigned char)layer->height;
			header.minx = (unsigned char)layer->minx;
			header.maxx = (unsigned char)layer->maxx;
			header.miny = (unsigned char)layer->miny;
			header.maxy = (unsigned char)layer->maxy;
			header.hmin = (unsigned short)layer->hmin;
			header.hmax = (unsigned short)layer->hmax;

			dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons,
				&tile->data, &tile->dataSize);
			if (dtStatusFailed(status))
			{
				return 0;
			}
		}

		// Transfer ownsership of tile data from build context to the caller.
		int n = 0;
		for (int i = 0; i < rcMin(rc.ntiles, maxTiles); ++i)
		{
			tiles[n++] = rc.tiles[i];
			rc.tiles[i].data = 0;
			rc.tiles[i].dataSize = 0;
		}

		return n;
	}

	// 构建
	bool NavigationTempObstacles::build(float agentRadius, float agentHeight, float stepHeight)
	{
		EchoAssert(m_geom);

		dtStatus status;

		m_tmproc->init(m_geom);

		// Init cache
		const float* bmin = (float*)&(m_geom->getAABB().vMin);
		const float* bmax = (float*)&(m_geom->getAABB().vMax);
		int gw = 0, gh = 0;
		rcCalcGridSize(bmin, bmax, m_cellSize, &gw, &gh);
		const int ts = (int)m_tileSize;
		const int tw = (gw + ts - 1) / ts;
		const int th = (gh + ts - 1) / ts;

		// Max tiles and max polys affect how the tile IDs are caculated.
		// There are 22 bits available for identifying a tile and a polygon.
		int tileBits = rcMin((int)dtIlog2(dtNextPow2(tw*th*EXPECTED_LAYERS_PER_TILE)), 14);
		if (tileBits > 14) tileBits = 14;
		int polyBits = 22 - tileBits;
		m_maxTiles = 1 << tileBits;
		m_maxPolysPerTile = 1 << polyBits;

		// Generation params.
		rcConfig cfg;
		memset(&cfg, 0, sizeof(cfg));
		cfg.cs = m_cellSize;
		cfg.ch = m_cellHeight;
		cfg.walkableSlopeAngle = m_agentMaxSlope;
		cfg.walkableHeight = (int)ceilf(m_agentHeight / cfg.ch);
		cfg.walkableClimb = (int)floorf(m_agentMaxClimb / cfg.ch);
		cfg.walkableRadius = (int)ceilf(m_agentRadius / cfg.cs);
		cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
		cfg.maxSimplificationError = m_edgeMaxError;
		cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
		cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
		cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
		cfg.tileSize = (int)m_tileSize;
		cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
		cfg.width = cfg.tileSize + cfg.borderSize * 2;
		cfg.height = cfg.tileSize + cfg.borderSize * 2;
		cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
		cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;
		rcVcopy(cfg.bmin, bmin);
		rcVcopy(cfg.bmax, bmax);

		// Tile cache params.
		dtTileCacheParams tcparams;
		memset(&tcparams, 0, sizeof(tcparams));
		rcVcopy(tcparams.orig, bmin);
		tcparams.cs = m_cellSize;
		tcparams.ch = m_cellHeight;
		tcparams.width = (int)m_tileSize;
		tcparams.height = (int)m_tileSize;
		tcparams.walkableHeight = m_agentHeight;
		tcparams.walkableRadius = m_agentRadius;
		tcparams.walkableClimb = m_agentMaxClimb;
		tcparams.maxSimplificationError = m_edgeMaxError;
		tcparams.maxTiles = tw*th*EXPECTED_LAYERS_PER_TILE;
		tcparams.maxObstacles = 512;

		dtFreeTileCache(m_tileCache);
		dtFreeTileCacheData(m_tileCacheData);

		m_tileCache = dtAllocTileCache();
		m_tileCacheData = dtAllocTileCacheData();
		if (!m_tileCache)
		{
			EchoLogError( "buildTiledNavigation: Could not allocate tile cache.");
			return false;
		}

		status = m_tileCache->init(&tcparams, m_talloc, m_tcomp, m_tmproc, m_tileCacheData);
		if (dtStatusFailed(status))
		{
			EchoLogError("buildTiledNavigation: Could not init tile cache.");
			return false;
		}

		dtFreeNavMesh(m_navMesh);
		dtFreeNavMeshData(m_navMeshData);

		m_navMesh = dtAllocNavMesh();
		m_navMeshData = dtAllocNavMeshData();
		if (!m_navMesh)
		{
			EchoLogError("buildTiledNavigation: Could not allocate navmesh.");
			return false;
		}

		dtNavMeshParams params;
		memset(&params, 0, sizeof(params));
		rcVcopy(params.orig, bmin);
		params.tileWidth = m_tileSize*m_cellSize;
		params.tileHeight = m_tileSize*m_cellSize;
		params.maxTiles = m_maxTiles;
		params.maxPolys = m_maxPolysPerTile;

		status = m_navMesh->init(&params, m_navMeshData);
		if (dtStatusFailed(status))
		{
			EchoLogError("buildTiledNavigation: Could not init navmesh.");
			return false;
		}

		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			EchoLogError("buildTiledNavigation: Could not init Detour navmesh query");
			return false;
		}

		// Preprocess tiles.
		m_cacheLayerCount = 0;
		m_cacheCompressedSize = 0;
		m_cacheRawSize = 0;

		const float* verts = m_geom->getVerts();
		const int nverts = m_geom->getVertCount();
		rcChunkyTriMesh* chunkyMesh = EchoNew(rcChunkyTriMesh);
		if (!rcCreateChunkyTriMesh(m_geom->getVerts(), m_geom->getTris(), m_geom->getTriInfos(), m_geom->getTriCount(), 256, chunkyMesh))
		{
			m_ctx->log(RC_LOG_ERROR, "buildTiledNavigation: Failed to build chunky mesh.");
			return false;
		}
		
		for (int y = 0; y < th; ++y)
		{
			for (int x = 0; x < tw; ++x)
			{
				TileCacheData tiles[MAX_LAYERS];
				memset(tiles, 0, sizeof(tiles));
				int ntiles = rasterizeTileLayers(chunkyMesh, x, y, cfg, tiles, MAX_LAYERS);

				for (int i = 0; i < ntiles; ++i)
				{
					TileCacheData* tile = &tiles[i];
					status = m_tileCache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
					if (dtStatusFailed(status))
					{
						dtFree(tile->data);
						tile->data = 0;
						continue;
					}

					m_cacheLayerCount++;
					m_cacheCompressedSize += tile->dataSize;
					m_cacheRawSize += calcLayerBufferSize(tcparams.width, tcparams.height);
				}
			}
		}

		// Build initial meshes
		for (int y = 0; y < th; ++y)
			for (int x = 0; x < tw; ++x)
				m_tileCache->buildNavMeshTilesAt(x, y, m_navMesh);

		m_cacheBuildMemUsage = m_talloc->high;

		const dtNavMesh* nav = m_navMesh;
		int navmeshMemUsage = 0;
		for (int i = 0; i < nav->getMaxTiles(); ++i)
		{
			const dtMeshTile* tile = nav->getTile(i);
			if (tile->header)
				navmeshMemUsage += tile->dataSize;
		}
		printf("navmeshMemUsage = %.1f kB", navmeshMemUsage / 1024.0f);

		return true;
	}

	// 保存
	void NavigationTempObstacles::save(const char* savePath)
	{
#ifdef ECHO_EDITOR_MODE
 		if ( m_navMeshData && m_tileCacheData)
 		{
 			FILE* fileHandle = fopen(savePath, "wb");
 			if (fileHandle)
 			{
				i32 version = 0;
				fwrite(&version, sizeof(ui32), 1, fileHandle);

				// 保存寻路数据
				ui32 tileSize = (ui32)m_navMeshData->tileDatas.size();
				fwrite(&m_navMeshData->params, sizeof(dtNavMeshParams), 1, fileHandle);
				fwrite(&tileSize, sizeof(ui32), 1, fileHandle);
				for (ui32 i = 0; i < tileSize; i++)
				{
					dtNavMeshTileData* tileData = &m_navMeshData->tileDatas[i];
					fwrite(&tileData->dataSize, sizeof(ui32), 1, fileHandle);
					fwrite(tileData->data, tileData->dataSize, 1, fileHandle);
				}

				// 保存块数据
				tileSize = m_tileCacheData->tileDatas.size();
				fwrite(&m_tileCacheData->params, sizeof(dtTileCacheParams), 1, fileHandle);
				fwrite(&tileSize, sizeof(ui32), 1, fileHandle);
				for (ui32 i = 0; i < tileSize; i++)
				{
					dtTileCacheTileData* tileData = &m_tileCacheData->tileDatas[i];
					fwrite(&tileData->dataSize, sizeof(ui32), 1, fileHandle);
					fwrite(tileData->data, tileData->dataSize, 1, fileHandle);
				}

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

	// 加载
	void NavigationTempObstacles::load(const String& filePath)
	{
		DataStream* pStream = IO::instance()->open(filePath);
		if (pStream)
		{		
			i32 version = 0;
			pStream->read(&version, sizeof(ui32));
			if (version == 0)
			{
				dtNavMeshData	navMeshData;				// 导航网格数据
				dtTileCacheData	tileCacheData;				// 块缓存数据

				// 加载寻路数据
				pStream->read(&navMeshData.params, sizeof(dtNavMeshParams));

				ui32 tileSize;
				pStream->read(&tileSize, sizeof(ui32));
				for (ui32 i = 0; i < tileSize; i++)
				{
					dtNavMeshTileData tileData;
					pStream->read(&tileData.dataSize, sizeof(ui32));

					tileData.data = (unsigned char*)dtAlloc(tileData.dataSize, DT_ALLOC_PERM);
					pStream->read(tileData.data, tileData.dataSize);

					navMeshData.tileDatas.push_back(tileData);
				}

				// 加载块数据
				pStream->read(&tileCacheData.params, sizeof(dtTileCacheParams));

				pStream->read(&tileSize, sizeof(ui32));
				for (ui32 i = 0; i < tileSize; i++)
				{
					dtTileCacheTileData tileData;
					pStream->read(&tileData.dataSize, sizeof(ui32));

					tileData.data = (unsigned char*)dtAlloc(tileData.dataSize, DT_ALLOC_PERM);
					pStream->read(tileData.data, tileData.dataSize);

					tileCacheData.tileDatas.push_back(tileData);
				}

				cleanup();

				// 组织NavMesh
				m_navMesh = dtAllocNavMesh();
				m_navMesh->init(&navMeshData.params, nullptr);
				for (dtNavMeshTileData& tile : navMeshData.tileDatas)
				{
					m_navMesh->addTile(tile.data, tile.dataSize, DT_TILE_FREE_DATA, 0, 0);
				}

 				// 组织块数据
  				m_tileCache = dtAllocTileCache();
  				m_tileCache->init(&tileCacheData.params, m_talloc, m_tcomp, m_tmproc, nullptr);
  				for (dtTileCacheTileData& tile : tileCacheData.tileDatas)
  				{
  					m_tileCache->addTile(tile.data, tile.dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
  				}
			}
			else
			{
				// solo nav mesh
				int navDataSize = version;

				unsigned char* navData = (unsigned char*)dtAlloc(navDataSize, DT_ALLOC_PERM);
				EchoAssert(navData);
				pStream->read(navData, navDataSize);

				cleanup();

				m_navMesh = dtAllocNavMesh();
				dtStatus status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
				if (dtStatusFailed(status))
				{
					dtFree(navData);
					EchoLogError("Could not init Detour navmesh");
					return;
				}
			}

			dtStatus status;
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

	// 添加圆柱形障碍物
	void NavigationTempObstacles::addTempObstacleCylinder(const Vector3& pos, float radius, float height)
	{
		if (!m_tileCache)
			return;
		float p[3];
		dtVcopy(p, (float*)&pos);
		p[1] -= 0.5f;
		m_tileCache->addObstacle(p, radius, height, 0);
	}

	// 添加盒子障碍物
	void NavigationTempObstacles::addTempObstacleBox(const Vector3& vMin, const Vector3& vMax)
	{
		if ( m_tileCache)
			m_tileCache->addBoxObstacle((float*)&vMin, (float*)&vMax, 0);
	}

	// 添加轴向包围盒障碍物
	void NavigationTempObstacles::addTempObstacleObb(const Vector3& p0, const Vector3& p1, float width, float height)
	{
		if (m_tileCache)
		{
			Vector3 beginP = p0;
			Vector3 endP   = p1;
			beginP.y = endP.y = (p0.y + p1.y) * 0.5f;
			m_tileCache->addObbObstacle((float*)&beginP, (float*)&endP, width, height, 0);
		}
	}

	// 
	static bool isectSegAABB(const float* sp, const float* sq, const float* amin, const float* amax, float& tmin, float& tmax)
	{
		static const float EPS = 1e-6f;

		float d[3];
		rcVsub(d, sq, sp);
		tmin = 0;  // set to -FLT_MAX to get first hit on line
		tmax = FLT_MAX;		// set to max distance ray can travel (for segment)

		// For all three slabs
		for (int i = 0; i < 3; i++)
		{
			if (fabsf(d[i]) < EPS)
			{
				// Ray is parallel to slab. No hit if origin not within slab
				if (sp[i] < amin[i] || sp[i] > amax[i])
					return false;
			}
			else
			{
				// Compute intersection t value of ray with near and far plane of slab
				const float ood = 1.0f / d[i];
				float t1 = (amin[i] - sp[i]) * ood;
				float t2 = (amax[i] - sp[i]) * ood;
				// Make t1 be intersection with near plane, t2 with far plane
				if (t1 > t2) rcSwap(t1, t2);
				// Compute the intersection of slab intersections intervals
				if (t1 > tmin) tmin = t1;
				if (t2 < tmax) tmax = t2;
				// Exit with no collision as soon as slab intersection becomes empty
				if (tmin > tmax) return false;
			}
		}

		return true;
	}

	// 
	dtObstacleRef hitTestObstacle(const dtTileCache* tc, const float* sp, const float* sq)
	{
		float tmin = FLT_MAX;
		const dtTileCacheObstacle* obmin = 0;
		for (int i = 0; i < tc->getObstacleCount(); ++i)
		{
			const dtTileCacheObstacle* ob = tc->getObstacle(i);
			if (ob->state == DT_OBSTACLE_EMPTY)
				continue;

			float bmin[3], bmax[3], t0, t1;
			tc->getObstacleBounds(ob, bmin, bmax);

			if (isectSegAABB(sp, sq, bmin, bmax, t0, t1))
			{
				if (t0 < tmin)
				{
					tmin = t0;
					obmin = ob;
				}
			}
		}
		return tc->getObstacleRef(obmin);
	}

	// 移除障碍物
	void NavigationTempObstacles::removeTempObstacle(const float* sp, const float* sq)
	{
		if (!m_tileCache)
			return;
		dtObstacleRef ref = hitTestObstacle(m_tileCache, sp, sq);
		m_tileCache->removeObstacle(ref);
	}

	// 移除所有障碍物
	void NavigationTempObstacles::clearAllTempObstacles()
	{
		if (!m_tileCache)
			return;
		for (int i = 0; i < m_tileCache->getObstacleCount(); ++i)
		{
			const dtTileCacheObstacle* ob = m_tileCache->getObstacle(i);
			if (ob->state == DT_OBSTACLE_EMPTY) continue;
			m_tileCache->removeObstacle(m_tileCache->getObstacleRef(ob));
		}
	}
}
