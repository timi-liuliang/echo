#include "LordExport.h"
#include "Vector4.h"
#include "LightmapMgr.h"
#include <Foundation/Util/StringUtil.h>

namespace Lightmass
{
	extern char GLordSavePath[512];
	static LightmapMgr lightmapMgr;
	
	struct LightmapData
	{
		string		  UniqueName;	// 名称
		vector<Color> Colors;		// 颜色
		int			  Width;		// 宽
		int			  Height;		// 高
		Vector3		  Scale;		// 缩放
	};
	
	static multimap<int, LightmapData*> LightmapDatas;

	/**
	 * Send complete lighting data to LORD
	 *
	 * @param LightingData - Object containing the computed data
	 */
	void ExportResultsToLORDLightMgr( const char* UniqueName, void* colors, int width, int height, const float* scale)
	{
		LightmapData* pLightmapData = new LightmapData;
		if( pLightmapData)
		{
			pLightmapData->UniqueName = UniqueName;

			// 结果数据
			pLightmapData->Colors.resize(width*height);
			for( int i=0; i<width*height; i++)
			{
				pLightmapData->Colors[i] = Color( ((Vector4*)colors)[i].x, ((Vector4*)colors)[i].y, ((Vector4*)colors)[i].z, 1.f);
			}

			pLightmapData->Width = width;
			pLightmapData->Height= height;
			pLightmapData->Scale = Vector3( scale[0], scale[1], scale[2]);

			LightmapDatas.insert( make_pair(width*height, pLightmapData));
		}
	}

	/**
	 * Save LightmapMgr
	 */
	void LORDSaveLightMgr(int num)
	{	
		// 添加光照图
		for( multimap<int, LightmapData*>::reverse_iterator it=LightmapDatas.rbegin(); it!=LightmapDatas.rend(); it++)
		{
			LightmapData* pLightmapData = it->second;
			if( pLightmapData)
			{
				lightmapMgr.AddLightmap( pLightmapData->UniqueName.c_str(), pLightmapData->Colors.data(), pLightmapData->Width, pLightmapData->Height, pLightmapData->Scale);

				SAFE_DELETE( it->second);
			}
		}

		LightmapDatas.clear();

		std::string temStr(GLordSavePath);
		int size = temStr.find_last_of('/');

		char strNum[5];
		sprintf(strNum, "%d", num);

		lightmapMgr.Save( GLordSavePath, temStr.substr(size+1, temStr.length())+strNum, num);
	}
}