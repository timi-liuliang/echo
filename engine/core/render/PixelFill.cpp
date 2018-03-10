#include "Engine/core/Render/PixelFill.h"
#include "engine/core/render/TextureResManager.h"
#include "engine/core/resource/ResourceGroupManager.h"
#include "engine/core/Util/LogManager.h"
#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_utils.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace rapidxml;

namespace Echo
{
	PixeFill::PixeFill(TextureRes* baseTexture)
		: mpBaseTexture(baseTexture)
	{

	}

	PixeFill::~PixeFill()
	{

	}

	bool PixeFill::LoadCFGFile( const String& cfgFile )
	{
		try
		{
			DataStream* pXMLStream = ResourceGroupManager::instance()->openResource(cfgFile);
			if(pXMLStream == NULL)
			{
				EchoLogError("PixelFill file not found [%s].", cfgFile.c_str());
				return false;
			}
			size_t fileSize = pXMLStream->size();
			char* xmlData = (char*)EchoMalloc(fileSize + 1);
			pXMLStream->read(xmlData, fileSize);
			xmlData[fileSize] = 0;

			xml_document<> doc;        
			doc.parse<0>(xmlData);

			xml_node<> *pRootNode = doc.first_node();
			if(!pRootNode)
			{
				EchoLogError("PixelFill file content is valid. this is no root node");
				EchoSafeFree(xmlData);
				EchoSafeDelete(pXMLStream, DataStream);
				return false;
			}

			xml_node<>* typeNode = pRootNode->first_node();
			while(typeNode)
			{
				String l_TypeName = typeNode->name();
				if(l_TypeName == "Elements")
				{
					// parse property nodes
					xml_node<> *pElementNode = typeNode->first_node();
					while(pElementNode)
					{
						PixeFillElement* pElement = EchoNew(PixeFillElement);
						xml_attribute<> *pElementAttribute = pElementNode->first_attribute();
						pElement->id = pElementAttribute->value();
						pElementAttribute = pElementAttribute->next_attribute();
						pElement->inArea = StringUtil::ParseVec4(pElementAttribute->value());
						pElementAttribute = pElementAttribute->next_attribute();
						pElement->outPoint = StringUtil::ParseVec2(pElementAttribute->value());
						pElementAttribute = pElementAttribute->next_attribute();
						pElement->inTexture = pElementAttribute->value();
						
						// ¼ÓÔØÍ¼Æ¬
						TextureMapIt it = mTextureMap.find(pElement->inTexture);
						if(it == mTextureMap.end())
						{
							loadTexture(pElement->inTexture);
						}
						mElementMap[pElement->id] = pElement;
						pElementNode = pElementNode->next_sibling();
					}
				}
				

				typeNode = typeNode->next_sibling();
			}


			EchoSafeFree(xmlData);
			EchoSafeDelete(pXMLStream, DataStream);
			return true;
		}
		catch(...)
		{
			EchoLogError("Parse model file [%s] failed.", cfgFile.c_str());
		}
		return false;
	}

	void PixeFill::SaveCFGFile( const String& fileName )
	{

	}

	void PixeFill::loadTexture( const String& fileName )
	{
		TextureRes* _pTexture = TextureResManager::instance()->createTexture(fileName, Texture::TU_STATIC);
		_pTexture->setRetainPreparedData(true);
		_pTexture->prepareLoad();
		mTextureMap[fileName] = _pTexture;
	}

	void PixeFill::freeTextures()
	{
		TextureMapIt it = mTextureMap.begin();
		for (;it!=mTextureMap.end();++it)
		{
			TextureResManager::instance()->releaseResource(it->second);
		}
		mTextureMap.clear();
	}

	void PixeFill::PixeFillByID( const String& id )
	{
		PixeFillElementMapIt it = mElementMap.find(id);

		if(it != mElementMap.end())
		{
			TextureMapIt itTexture = mTextureMap.find(it->second->inTexture);
			if(itTexture != mTextureMap.end())
			{
				mpBaseTexture->setPixelsByArea(ui32(it->second->inArea.x),
					ui32(it->second->inArea.y),
					ui32(it->second->inArea.z),
					ui32(it->second->inArea.w),
					ui32(it->second->outPoint.x),
					ui32(it->second->outPoint.y),
					itTexture->second->getTexture());
			}
		}
	}

}