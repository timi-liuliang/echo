#ifndef _LIGHTMAPMGR_H_
#define _LIGHTMAPMGR_H_

#include <map>
#include <string>
#include <vector>
#include "TextureSet.h"

using namespace std;

namespace Lightmass
{
	//---------------------------------
	// 场景光照图管理器 2012-7-7
	//     添加,删除,合并,保存
	//---------------------------------
	class LightmapMgr
	{
	public:
		// TextureTile
		struct TileInfo
		{
			string  m_uniqueName;	// 唯一标识
			int		m_setIdx;		// 纹理索引
			int		m_tileId;		// 纹理块ID
			Vector3 m_scale;		// 纹理亮度缩放值
		};
		typedef map<string, TileInfo> TileInfoMap;

	public:
		LightmapMgr();
		LightmapMgr( int width, int height);

		// 添加光照图(以实体名与mesh索引命名)
		bool AddLightmap( const char* uniqueName, Color* texture, int width, int height, const Vector3& scale);

		// 查询
		//bool Query( const char* uniqueName, const Texture*& lightmap, Vector4& lightmapViewPort);

		// 清除所有
		void Clear();

		// 加载
		bool Load();

		// 保存
		bool Save(const string& locationPath, const string& sceneName, int num);

		ui32 getTextureSetNum() const;
		//// 加载xml格式
		//bool LoadXml( const char* locationPath);

		//// 保存xml格式
		//bool SaveXml( const char* locationPath);

	private:
		// 新建纹理集
		TextureSet* AllocateTextureSet( int width, int height);

		// 添加到指定纹理集
		bool AddLightmapToTextureSet( TextureSet* texSet, int setIdx, const char* uniqueName, Color* texture, int width, int height, const Vector3& scale);

		// 覆写纹理集
		bool OverWriteLightmapToTextureSet( TextureSet* texSet, int tileId, Color* texture, int width, int height);

		// 获取合适的纹理长度
		int calcTexSetLen( int texLen);

	private:
		int							m_tsWidth;		// TextureSet宽
		int							m_tsHeight;		// TextureSet高
		TileInfoMap					m_tileInfos;	// 块信息
		vector<TextureSet*>			m_texSets;		// 纹理集
	};
}

#endif