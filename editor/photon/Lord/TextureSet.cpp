#include "TextureSet.h"
#include <string>
#include "LordMath.h"
#include "DataStream.h"
#include <fstream>
#include <Windows.h>
#include "PublicTypes.h"

using namespace std;

#define INVALID -1

namespace Lightmass
{
	const string TextureSet::s_TextureName = "TextureSet_";
	ui32 TextureSet::s_TextureSetID = 0;
	
	// 构造函数
	TextureSet::Node::Node()
		: m_id( INVALID)
	{
		++s_TextureSetID;
		m_child[0] = INVALID;
		m_child[1] = INVALID;
	}

	// 是否为叶结点
	bool TextureSet::Node::IsLeaf() const
	{ 
		return m_child[0]==INVALID && m_child[1]==INVALID; 
	}

	TextureSet::TextureSet()
		: m_width(512)
		, m_height(512)
		//, m_texture(NULL)
		, m_TextureData(NULL)
	{

	}

	// 构造函数
	TextureSet::TextureSet(int width, int height)
		: m_width(width)
		, m_height(height)
		//, m_texture(NULL)
		, m_TextureData(NULL)
	{
		// 插入默认主节点
		Node rootNode;
		rootNode.m_rc = IRect( 0, 0, m_width, m_height);
		m_nodes.reserve(256);
		m_nodes.push_back( rootNode);
	}

	TextureSet::~TextureSet()
	{
		//if (m_texture)
		//{
		//	LordTextureManager->releaseResource(m_texture);
		//	m_texture = NULL;
		//}
		SAFE_DELETE_ARRAY(m_TextureData);
	}

	// 获取块纹理Viewport
	const Vector4 TextureSet::GetViewport( int nodeIdx) const
	{
		LordAssert( nodeIdx>=0 && nodeIdx<static_cast<int>(m_nodes.size()));

		Vector4	result;
		const IRect& tRc   = m_nodes[nodeIdx].m_rc;
		result.x = static_cast<float>(tRc.left)  / static_cast<float>(m_width);
		result.y = static_cast<float>(tRc.top)   / static_cast<float>(m_height);
		result.z = static_cast<float>(tRc.width) / static_cast<float>(m_width);
		result.w = static_cast<float>(tRc.height)/ static_cast<float>(m_height);

		return result;
	}

	// 插入函数
	int TextureSet::Insert(Color* data, int width, int height)
	{
		if( !data)
			return INVALID;

		int nodeIdx = Insert( 0, data, width, height);

		return OverWrite( nodeIdx, data, width, height);
	}

	// 覆写纹理
	int TextureSet::OverWrite( int nodeIdx, Color* data, int width, int height )
	{
		if( nodeIdx!=INVALID)
		{
			m_nodes[nodeIdx].m_id = nodeIdx;

			// copy pixels over from texture to pNode->m_rc part of texture
			if (!m_TextureData)
			{
				m_TextureData = ::new Color[m_width*m_height];
				memset(m_TextureData, 0, m_width*m_height * sizeof(Color));
			}
			const IRect& rc = m_nodes[nodeIdx].m_rc;

			Color* pDestData = (Color*)m_TextureData;	
			for ( int h=0; h<height; h++)
			{
				for( int w=0; w<width; w++)
				{
					int destIdx = ( rc.top + h) * m_width + w + rc.left;
					int srcIdx  = h * width + w;

					pDestData[destIdx] = data[srcIdx];
				}
			}
			
			return nodeIdx;
		}

		return INVALID;
	}

	// 插入纹理
	int TextureSet::Insert( int nodeIdx, Color* data, int width, int height)
	{
		if( nodeIdx==INVALID)
			return INVALID;

		// if we're not a leaf then
		if( !m_nodes[nodeIdx].IsLeaf())
		{
			// 优先插入面积较小的块
			int child0Idx = m_nodes[nodeIdx].m_child[0];
			int child1Idx = m_nodes[nodeIdx].m_child[1];
			if( child0Idx!=INVALID && child1Idx!=INVALID)
			{
				if( m_nodes[child0Idx].m_rc.getArea()>m_nodes[child1Idx].m_rc.getArea())
					std::swap( child0Idx, child1Idx);
			}

			// try inserting into first child
			int newIdx = Insert( child0Idx, data, width, height);
			if( newIdx != INVALID)
				return newIdx;

			// no room, insert into second
			return Insert( child1Idx, data, width, height);
		}
		else
		{
			// if there is already a lightmap here, return
			if( m_nodes[nodeIdx].m_id != INVALID)
				return INVALID;

			int texWidth  = static_cast<int>(width);
			int texHeight = static_cast<int>(height);

			// if we're too small, return
			if( m_nodes[nodeIdx].m_rc.width<texWidth || m_nodes[nodeIdx].m_rc.height<texHeight)
				return INVALID;

			// if we'rs just right accept, "no surplus space"
			if( m_nodes[nodeIdx].m_rc.width==texWidth && m_nodes[nodeIdx].m_rc.height==texHeight)
				return nodeIdx;

			// otherwise, gotta split this node and create some kids
			Node node0, node1;

			// deside split pattern
			int dw = m_nodes[nodeIdx].m_rc.width  - texWidth;
			int dh = m_nodes[nodeIdx].m_rc.height - texHeight;

			const IRect& nRect = m_nodes[nodeIdx].m_rc;
			if( dw>dh)
			{
				node0.m_rc = IRect( nRect.left+texWidth, nRect.top, nRect.width-texWidth, nRect.height);
				node1.m_rc = IRect( nRect.left, nRect.top+texHeight, texWidth, nRect.height-texHeight);
			}
			else
			{
				node0.m_rc = IRect( nRect.left+texWidth, nRect.top, nRect.width-texWidth, texHeight);
				node1.m_rc = IRect( nRect.left, nRect.top+texHeight, nRect.width, nRect.height-texHeight);
			}

			// 添加有省余空间的子结点
			if( node0.m_rc.getArea() > 0)
			{
				m_nodes[nodeIdx].m_child[0] = m_nodes.size();
				m_nodes.push_back( node0);
			}
			if( node1.m_rc.getArea() > 0)
			{
				m_nodes[nodeIdx].m_child[1] = m_nodes.size();
				m_nodes.push_back( node1);
			}

			m_nodes[nodeIdx].m_rc.width = texWidth;
			m_nodes[nodeIdx].m_rc.height= texHeight;

			return nodeIdx;
		}
	}
	// 加载
	void TextureSet::Load( const char* filePath)
	{
		//String		  tePath(filePath);
		//int index = tePath.find_last_of('.');
		//String ddsName = tePath.substr(0, index) + ".tga";

		//m_texture = LordTextureManager->createTexture(ddsName);
		//m_texture->load();

		//DataStream* pStream = ResourceGroupManager::Instance()->openResource(tePath);
		//if( pStream)
		//{
		//	// 保存上方向索引
		//	size_t numNode = 0;
		//	pStream->read(&numNode, sizeof(numNode));

		//	// 保存数据大小
		//	m_nodes.resize( numNode);
		//	pStream->read( m_nodes.data(), sizeof( Node)*numNode);
		//	LordDelete(pStream);
		//}
	//	// 加载纹理
	//	string tePath;
	//	FileUtils::ReplaceExt( tePath, filePath, ".dds");

	//#ifdef ARES_EDITOR_MODE
 //       TexturePtr tePtr = AResSystem.SyncLoadDDS( tePath.c_str(), EAH_GPURead | EAH_CPUWrite);
	//	Init( tePtr);
	//#else
 //       TexturePtr tePtr = AResSystem.SyncLoadDDS( tePath.c_str(), EAH_GPURead | EAH_Immutable);
	//	Init( tePtr);
	//#endif

	//	// 加载结点信息
	//	FILE* fileHandle = fopen( filePath, "rb");
	//	if( fileHandle)
	//	{
	//		// 保存上方向索引
	//		size_t numNode = 0;
	//		fread( &numNode, sizeof(numNode), 1, fileHandle);

	//		// 保存数据大小
	//		m_nodes.resize( numNode);
	//		fread( m_nodes.data(), sizeof( Node)*numNode, 1, fileHandle);

	//		fflush( fileHandle);
	//		fclose( fileHandle);
	//	}
	}

	#define GCC_PACK(n)

	/* Only works on powers of two, Alignment is in bytes */
	template< class T > inline T Align( const T Ptr, INT Alignment )
	{
		return (T)(((INT)Ptr + Alignment - 1) & ~(Alignment-1));
	}

	// 保存
	void TextureSet::Save( const char* filePath)
	{
		LordAssert(m_TextureData);
		// 保存纹理
		string		  tePath(filePath);
		int index = tePath.find_last_of('.');
		string ddsName = tePath.substr(0, index) + ".bmp";

		std::fstream f;
		f.open(filePath, std::ios::out|std::ios::binary|std::ios::trunc);
		DataStream* filePtr( new FileStreamDataStream(&f, false));
		if (filePtr)
		{
			ui32 version = 20160810;
			filePtr->write(&version, sizeof(ui32));

			filePtr->write(&m_width, sizeof(i32));
			filePtr->write(&m_height, sizeof(i32));

			ui32 numNode = m_nodes.size();
			filePtr->write(&numNode, sizeof(numNode));

			//	// 保存数据大小
			filePtr->write( m_nodes.data(), sizeof(Node)*numNode);
		}

		SAFE_DELETE( filePtr);

		f.close();

		// 保存纹理
#pragma pack (push,1)
		struct BITMAPFILEHEADER
		{
			WORD	bfType GCC_PACK(1);
			unsigned __int32	bfSize GCC_PACK(1);
			WORD	bfReserved1 GCC_PACK(1); 
			WORD	bfReserved2 GCC_PACK(1);
			unsigned __int32	bfOffBits GCC_PACK(1);
		} FH; 
		struct BITMAPINFOHEADER
		{
			unsigned __int32	biSize GCC_PACK(1); 
			INT		biWidth GCC_PACK(1);
			INT		biHeight GCC_PACK(1);
			WORD	biPlanes GCC_PACK(1);
			WORD	biBitCount GCC_PACK(1);
			unsigned __int32	biCompression GCC_PACK(1);
			unsigned __int32	biSizeImage GCC_PACK(1);
			INT		biXPelsPerMeter GCC_PACK(1); 
			INT		biYPelsPerMeter GCC_PACK(1);
			unsigned __int32	biClrUsed GCC_PACK(1);
			unsigned __int32	biClrImportant GCC_PACK(1); 
		} IH;
#pragma pack (pop)

		INT BytesPerLine = Align(m_width * 3,4);

		HANDLE File = CreateFile( ddsName.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		// File header.
		FH.bfType       		= (WORD) ('B' + 256*'M');
		FH.bfSize       		= (unsigned __int32) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + BytesPerLine * m_height);
		FH.bfReserved1  		= (WORD) 0;
		FH.bfReserved2  		= (WORD) 0;
		FH.bfOffBits    		= (unsigned __int32) (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
		::DWORD BytesWritten;

		WriteFile( File, &FH, sizeof(FH), &BytesWritten, NULL);

		// Info header.
		IH.biSize               = (unsigned __int32) sizeof(BITMAPINFOHEADER);
		IH.biWidth              = (unsigned __int32) m_width;
		IH.biHeight             = (unsigned __int32) m_height;
		IH.biPlanes             = (WORD) 1;
		IH.biBitCount           = (WORD) 24;
		IH.biCompression        = (unsigned __int32) 0; //BI_RGB
		IH.biSizeImage          = (unsigned __int32) BytesPerLine * m_height;
		IH.biXPelsPerMeter      = (unsigned __int32) 0;
		IH.biYPelsPerMeter      = (unsigned __int32) 0;
		IH.biClrUsed            = (unsigned __int32) 0;
		IH.biClrImportant       = (unsigned __int32) 0;

		WriteFile(File, &IH, sizeof(IH), &BytesWritten, NULL);

		// write out the image, bottom up
		for (INT Y = m_height - 1; Y >= 0; Y--)
		{
			for (INT X = 0; X < m_width; X++)
			{
				Color& CurColor = ((Color*)m_TextureData)[Y * m_width + X];
				FColorBase Color = FColorBase(
					(BYTE)Math::Clamp<INT>((INT)(CurColor.r * 255), 0, 255), 
					(BYTE)Math::Clamp<INT>((INT)(CurColor.g * 255), 0, 255), 
					(BYTE)Math::Clamp<INT>((INT)(CurColor.b * 255), 0, 255),
					0);

				WriteFile( File, &Color, 3, &BytesWritten, NULL);
			}

			// pad if necessary
			static BYTE Zero[3] = {0, 0, 0};
			if (m_width * 3 < BytesPerLine)
			{
				WriteFile(File, &Zero, BytesPerLine - m_width * 3, &BytesWritten, NULL);
			}
		}

		// close the open files
		CloseHandle(File);
	}
}