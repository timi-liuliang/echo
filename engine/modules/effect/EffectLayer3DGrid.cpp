#include "EffectLayer3DGrid.h"
#include "EffectSystemManager.h"
#include "Engine/core/Scene/Node.h"
#include "engine/core/io/DataStream.h"
#include "engine/core/Scene/NodeTree.h"

namespace Echo
{
	const Vector3  DECAC_CHECK_DIR	= Vector3(0.f, -1.f, 0.f);
	const float	   DECAL_CHECK_UP	= 0.5f;
	const float	   DECAL_CHECK_DOWN	= 1.0f;
	const float    DECAL_CHECK_ANGLE = 0.5f;
	const float	   DECAL_GAP        = 0.02f;

	EffectLayer3DGrid::EffectLayer3DGrid()
		: EffectLayer()
		, mRows(4)
		, mColumns(4)
		, mInterval(0.3f)
		, mpVectorArray(NULL)
		, mpColorArray(NULL)
		, mOffViewport(0)
		, mCallbackFunction(NULL)
		, mbDecal(false)
		, mDecalInfoArray(NULL)
	{
		direction = Vector3::UNIT_Z;
		originalDirection = Vector3::UNIT_Z;
		mType = ELT_3DEditableSurface;
	}

	EffectLayer3DGrid::~EffectLayer3DGrid()
	{
		EchoSafeFree(mpVectorArray);
		EchoSafeFree(mpColorArray);
		EchoSafeFree(mDecalInfoArray);
	}

	void EffectLayer3DGrid::setCallBackFunction(ES_FUN fun)
	{
		mCallbackFunction = fun;
	}

	void EffectLayer3DGrid::prepare()
	{
		EffectLayer::prepare();
		EchoAssert(mRows>1);
		EchoAssert(mColumns>1);
		EchoAssert(mInterval>0.001f);
		if(!mpVectorArray)
		{
			mpVectorArray = (Vector3*)(EchoMalloc(sizeof(Vector3)*mColumns*mRows));

			float xbegin = -((mColumns-1)*mInterval/2.0f);
			float ybegin = ((mRows-1)*mInterval/2.0f);
			for(ui32 i=0; i<mRows; ++i)
			{
				for(ui32 j=0; j<mColumns; ++j)
				{
					Vector3& vec = mpVectorArray[i*mColumns+j];
					vec.x = xbegin + j*mInterval;
					vec.y = ybegin - i*mInterval;
					vec.z = 0.0f;
				}
			}
		}
		if(!mpColorArray)
		{
			mpColorArray = (Color*)(EchoMalloc(sizeof(Color)*mColumns*mRows));

			for(ui32 i=0; i<mRows; ++i)
			{
				for(ui32 j=0; j<mColumns; ++j)
				{
					mpColorArray[i*mColumns+j] = Color::WHITE;
				}
			}
		}
		mGridBox.reset();
		for(ui32 i=0; i<mRows; ++i)
		{
			for(ui32 j=0; j<mColumns; ++j)
			{
				mGridBox.addPoint(mpVectorArray[i*mColumns+j]);
			}
		}

		if (mbDecal && !mDecalInfoArray)
		{
			mDecalInfoArray = (DecalInfo*)(EchoMalloc(sizeof(DecalInfo)*mColumns*mRows));
		}
	}

	void EffectLayer3DGrid::unprepare()
	{
		EffectLayer::unprepare();
	}

	void EffectLayer3DGrid::_adjustGridRows(size_t rows)
	{
		if(rows < 2)
			return ;

		if(mRows != rows)
		{
			if(mpVectorArray && mpColorArray)
			{
				ui32 newSize = rows * mColumns;
				Vector3* pNewVectorArray = (Vector3*)EchoMalloc(sizeof(Vector3)*newSize);
				Color* pNewColorArray = (Color*)EchoMalloc(sizeof(Color)*newSize);
				for(ui32 i=0; i<rows; ++i)
				{
					for(ui32 j=0; j<mColumns; ++j)
					{
						if(i<mRows)
						{
							pNewVectorArray[i*(mColumns)+j] = mpVectorArray[i*(mColumns)+j];
							pNewColorArray[i*(mColumns)+j] = mpColorArray[i*(mColumns)+j];
						}
						else
						{
							pNewVectorArray[i*(mColumns)+j] = pNewVectorArray[(i-1)*(mColumns)+j];
							pNewVectorArray[i*(mColumns)+j].y -= mInterval;
							pNewColorArray[i*(mColumns)+j] = Color::WHITE;
						}
					}
				}
				float offset = mInterval*(int(rows)-int(mRows))/2.0f;
				for(ui32 i=0; i<newSize; ++i)
					pNewVectorArray[i].y += offset;

				EchoSafeFree(mpVectorArray);
				EchoSafeFree(mpColorArray);
				mpVectorArray = pNewVectorArray;
				mpColorArray = pNewColorArray;
			}
			mRows = rows;

			if (mbDecal)
			{
				EchoSafeFree(mDecalInfoArray);
				mDecalInfoArray = (DecalInfo*)(EchoMalloc(sizeof(DecalInfo)*mColumns*mRows));
			}
		}
	}

	void EffectLayer3DGrid::_adjustGridColumns(size_t columns)
	{
		if(columns < 2)
			return;

		if(mColumns != columns)
		{
			if(mpVectorArray && mpColorArray)
			{
				ui32 newSize = (columns) * (mRows);
				Vector3* pNewVectorArray = (Vector3*)EchoMalloc(sizeof(Vector3)*newSize);
				Color* pNewColorArray = (Color*)EchoMalloc(sizeof(Color)*newSize);
				for(ui32 i=0; i<mRows; ++i)
				{
					Vector3 edgeVector = mpVectorArray[(i+1)*(mColumns)-1];
					Color edgeColor = mpColorArray[(i+1)*(mColumns)-1];
					for(ui32 j=0; j<columns; ++j)
					{
						if(j<mColumns)
						{
							pNewVectorArray[i*(columns)+j] = mpVectorArray[i*(mColumns)+j];
							pNewColorArray[i*(columns)+j] = mpColorArray[i*(mColumns)+j];
						}
						else
						{
							edgeVector.x += mInterval;
							pNewVectorArray[i*(columns)+j] = edgeVector;
							pNewColorArray[i*(columns)+j] = Color::WHITE;
						}
					}
				}

				float offset = -mInterval*(int(columns) - int(mColumns))/2.0f;
				for(ui32 i=0; i<newSize; ++i)
					pNewVectorArray[i].x += offset;

				EchoSafeFree(mpVectorArray);
				EchoSafeFree(mpColorArray);
				mpVectorArray = pNewVectorArray;
				mpColorArray = pNewColorArray;
			}
			mColumns = columns;

			if (mbDecal)
			{
				EchoSafeFree(mDecalInfoArray);
				mDecalInfoArray = (DecalInfo*)(EchoMalloc(sizeof(DecalInfo)*mColumns*mRows));
			}
		}
	}

	void EffectLayer3DGrid::_adjustGridInterval(float interval)
	{
		if(interval != mInterval)
		{
			mInterval = interval;
			if(mpVectorArray)
			{
				float xbegin = -((mColumns-1)*mInterval/2.0f);
				float ybegin = ((mRows-1)*mInterval/2.0f);
				for(size_t i=0; i<mRows; ++i)
				{
					for(size_t j=0; j<mColumns; ++j)
					{
						Vector3& vec = mpVectorArray[i*mColumns+j];
						vec.x = xbegin + j*mInterval;
						vec.y = ybegin - i*mInterval;
						vec.z = 0.0f;

						Color& color = mpColorArray[i*mColumns+j];
						color = Color::WHITE;
					}
				}
			}
		}
	}

	//-----------------------------------------------------------------------
	Vector3 EffectLayer3DGrid::getSurfaceVector(int row, int column)const
	{
		Vector3 ret = Vector3::ZERO;
		if(mpVectorArray)
		{
			ret = mpVectorArray[row*mColumns+column];
		}

		return ret;
	}
	//-----------------------------------------------------------------------
	void EffectLayer3DGrid::setSurfaceVector(int row, int column, const Vector3& newPos)
	{
		if(mpVectorArray)
		{
			mpVectorArray[row*mColumns+column] = newPos;
		}
	}
	//-----------------------------------------------------------------------
	Vector3 EffectLayer3DGrid::getSurfaceVector(int index)const
	{
		Vector3 ret = Vector3::ZERO;
		if(mpVectorArray)
		{
			ret = mpVectorArray[index];
		}

		return ret;
	}
	//-----------------------------------------------------------------------
	void EffectLayer3DGrid::setSurfaceVector(int index, const Vector3& newPos)
	{
		if(mpVectorArray)
		{
			mpVectorArray[index] = newPos;
		}
	}
	//-----------------------------------------------------------------------
	Color EffectLayer3DGrid::getSurfaceColor(int row, int column)const
	{
		Color ret = Color::WHITE;
		if(mpColorArray)
		{
			ret = mpColorArray[row*mColumns+column];
		}

		return ret;
	}
	//-----------------------------------------------------------------------
	void EffectLayer3DGrid::setSurfaceColor(int row, int column, const Color& newColor)
	{
		if(mpColorArray)
		{
			mpColorArray[row*mColumns+column] = newColor;
		}
	}
	//-----------------------------------------------------------------------
	Color EffectLayer3DGrid::getSurfaceColor(int index)const
	{
		Color ret = Color::WHITE;
		if(mpColorArray)
		{
			ret = mpColorArray[index];
		}

		return ret;
	}
	//-----------------------------------------------------------------------
	void EffectLayer3DGrid::setSurfaceColor(int index, const Color& newColor)
	{
		if(mpColorArray)
		{
			mpColorArray[index] = newColor;
		}
	}

	const Quaternion& EffectLayer3DGrid::getSurfaceOritation() const
	{
		return oritation;
	}

	void EffectLayer3DGrid::updateRenderData()
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel<EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;
		mVertexData.resize(mColumns * mRows);
		//ui32 colour = color.getABGR();

		Matrix4 mtxWorld;
		getMatrixWorld(mtxWorld);

		size_t i,j;
		float tl = mMaterial->getTexAddrLeft();
		float tr = mMaterial->getTexAddrRight();
		float tt = mMaterial->getTexAddrTop();
		float tb = mMaterial->getTexAddrBottom();
		float txSeg = (tr-tl)/(mColumns-1);
		float tySeg = (tb-tt)/(mRows-1);
		bool swapUV = mMaterial->getSwapUV();

		float tl1 = mMaterial->getTexAddrLeft1();
		float tr1 = mMaterial->getTexAddrRight1();
		float tt1 = mMaterial->getTexAddrTop1();
		float tb1 = mMaterial->getTexAddrBottom1();
		float txSeg1 = (tr1 - tl1) / (mColumns - 1);
		float tySeg1 = (tb1 - tt1) / (mRows - 1);
		bool swapUV1 = mMaterial->getSwapUV1();

		Color gridColor;
		color = color * mParentSystem->getSystemColor();
		EffectVertexFmt* verticsPtr = &mVertexData[0];
		for(i=0; i<mRows; ++i)
		{
			for(j=0; j<mColumns; ++j)
			{
				verticsPtr->pos = mpVectorArray[i*mColumns + j] * mtxWorld;
				if (mbDecal)
				{
					// 重置检测数据
					mDecalInfoArray[i*mColumns + j].index[0] = i;
					mDecalInfoArray[i*mColumns + j].index[1] = j;
					mDecalInfoArray[i*mColumns + j].valid = false;
					mDecalInfoArray[i*mColumns + j].checkHit= false;
					mDecalInfoArray[i*mColumns + j].checkAround = false;
					mDecalInfoArray[i*mColumns + j].beginPos = verticsPtr->pos;
					mDecalInfoArray[i*mColumns + j].y = verticsPtr->pos.y;
				}

					
				gridColor = color * mpColorArray[i*mColumns+j];
				verticsPtr->color = gridColor.getABGR();
				if(swapUV)
				{
					verticsPtr->tx = tl + i*txSeg;
					verticsPtr->ty = tb - j*tySeg;
				}
				else
				{
					verticsPtr->tx = tl + j*txSeg;
					verticsPtr->ty = tt + i*tySeg;
				}

				if (mMaterial->getUV2Enable())
				{
					if (swapUV1)
					{
						verticsPtr->tx1 = tl1 + i*txSeg1;
						verticsPtr->ty1 = tb1 - j*tySeg1;
					}
					else
					{
						verticsPtr->tx1 = tl1 + j*txSeg1;
						verticsPtr->ty1 = tt1 + i*tySeg1;
					}
				}

				++verticsPtr;
			}
		}

		if (mbDecal)
		{
			list<DecalInfo*>::type unChecks;
			EffectVertexFmt* verticsPtr = &mVertexData[0];
			for (i = 0; i < mRows; ++i)
			{
				for (j = 0; j < mColumns; ++j)
				{
					if (modifyDecalY(i, j) || (!mDecalInfoArray[i*mColumns + j].checkAround && checkDecalAroundY(i,j,unChecks)))
					{
						verticsPtr->pos.y = mDecalInfoArray[i*mColumns + j].y + DECAL_GAP;
					}
					++verticsPtr;
				}
			}
		}
		
	}

	void EffectLayer3DGrid::submitToRenderQueue(std::vector<EffectRenderable*>& renderables)
	{
		if ((!mbAffected) || (!mbVisible) || m_renderLevel < EffectSystemManager::instance()->getRenderLevelThreshhold())
			return;

		if (m_vertexDataMainThread.empty())
			return;

		EffectRenderable* pRenderable = EffectSystemManager::instance()->getRenderable(mMaterial->getTexturePtr(), mRenderGroup, mMaterial->getBlendMode(), mMaterial);
		if (!pRenderable)
			return;
		renderables.push_back(pRenderable);

		EffectVertexFmt* pVertexPtr = NULL;
		ui16* pIdxLockPtr = NULL;
		ui16 vertBegin;
		if (pRenderable->getLockPtr(mColumns*mRows, 6 * (mRows - 1)*(mColumns - 1), vertBegin, pVertexPtr, pIdxLockPtr))
		{
			size_t i, j;
			for (ui32 i = 0; i < mColumns*mRows; i++)
			{
				pVertexPtr[i].pos = m_vertexDataMainThread[i].pos;
				pVertexPtr[i].color = m_vertexDataMainThread[i].color;
				pVertexPtr[i].tx = m_vertexDataMainThread[i].tx;
				pVertexPtr[i].ty = m_vertexDataMainThread[i].ty;
				pVertexPtr[i].tx1 = m_vertexDataMainThread[i].tx1;
				pVertexPtr[i].ty1 = m_vertexDataMainThread[i].ty1;
			}
			for (i = 0; i < mRows - 1; ++i)
			{
				for (j = 0; j < mColumns - 1; ++j)
				{
					*pIdxLockPtr++ = vertBegin + i*mColumns + j;
					*pIdxLockPtr++ = vertBegin + (i + 1)*mColumns + j;
					*pIdxLockPtr++ = vertBegin + i*mColumns + j + 1;

					*pIdxLockPtr++ = vertBegin + (i + 1)*mColumns + j;
					*pIdxLockPtr++ = vertBegin + (i + 1)*mColumns + j + 1;
					*pIdxLockPtr++ = vertBegin + i*mColumns + j + 1;
				}
			}
		}
	}

	void EffectLayer3DGrid::getMatrixWorld(Matrix4& mtxWorld)
	{
		mtxWorld = mParentSystem->getEffectSceneNode()->getWorldMatrix();

		Matrix4 mtxLocal;
		mtxLocal.makeScaling(scale, scale, scale);

		Matrix4 mtxRot;
		Quaternion quanRot;
		quanRot.fromAxisAngle(originalDirection, selfRotation*Math::DEG2RAD);
		quanRot = quanRot*oritation;
		quanRot.toMat4(mtxRot);
		mtxLocal = mtxLocal * mtxRot;

		mtxLocal.translate(position);
		mtxWorld = mtxLocal * mtxWorld;
	}

	void EffectLayer3DGrid::copyAttributesTo(EffectParticle* particle)
	{
		EffectLayer::copyAttributesTo(particle);
		EffectLayer3DGrid* grid = static_cast<EffectLayer3DGrid*>(particle);
		grid->mRows = mRows;
		grid->mColumns = mColumns;
		grid->mInterval = mInterval;
		EchoSafeFree(grid->mpVectorArray);
		EchoSafeFree(grid->mpColorArray);

		grid->mpVectorArray = (Vector3*)EchoMalloc(sizeof(Vector3)*mRows*mColumns);
		grid->mpColorArray = (Color*)EchoMalloc(sizeof(Color)*mRows*mColumns);
		memcpy(grid->mpVectorArray, mpVectorArray, sizeof(Vector3)*mRows*mColumns);
		memcpy(grid->mpColorArray, mpColorArray, sizeof(Color)*mRows*mColumns);
		grid->mOffViewport = mOffViewport;
		grid->mbDecal = mbDecal;
	}

	void EffectLayer3DGrid::exportData(DataStream* pStream, int version)
	{
		EffectLayer::exportData(pStream, version);
		pStream->write(&mRows, sizeof(ui32));
		pStream->write(&mColumns, sizeof(ui32));
		pStream->write(&mInterval, sizeof(float));
		pStream->write(&mOffViewport, sizeof(float));
		pStream->write(mpVectorArray, sizeof(Vector3)*mRows*mColumns);
		pStream->write(mpColorArray, sizeof(Color)*mRows*mColumns);
		pStream->write(&mbDecal, sizeof(bool));
	}

	void EffectLayer3DGrid::importData(DataStream* pStream, int version)
	{
		EffectLayer::importData(pStream, version);
		pStream->read(&mRows, sizeof(ui32));
		pStream->read(&mColumns, sizeof(ui32));
		pStream->read(&mInterval, sizeof(float));
		pStream->read(&mOffViewport, sizeof(float));
		EchoAssertX(mpColorArray==NULL, "Error: EffectLayer3DGrid::importData mpColorArray!=NULL. ");
		EchoAssertX(mpVectorArray==NULL, "Error: EffectLayer3DGrid::importData mpVectorArray!=NULL. ");
		mpVectorArray = (Vector3*)EchoMalloc(sizeof(Vector3)*mRows*mColumns);
		mpColorArray = (Color*)EchoMalloc(sizeof(Color)*mRows*mColumns);
		pStream->read(mpVectorArray, sizeof(Vector3)*mRows*mColumns);
		pStream->read(mpColorArray, sizeof(Color)*mRows*mColumns);
		mGridBox.reset();
		for(ui32 i=0; i<mRows; ++i)
		{
			for(ui32 j=0; j<mColumns; ++j)
			{
				mGridBox.addPoint(mpVectorArray[i*mColumns+j]);
			}
		}

		if (version > 0x00010014)
		{
			pStream->read(&mbDecal, sizeof(bool));

			if (mbDecal && !mDecalInfoArray)
			{
				mDecalInfoArray = (DecalInfo*)(EchoMalloc(sizeof(DecalInfo)*mColumns*mRows));
			}
		}
	}

	void EffectLayer3DGrid::mergeAligenBox(Box& aabb)
	{
		if (!mbAffected)
			return;

		Matrix4 mtxLocal;
		mtxLocal.makeScaling(scale, scale, scale);

		Matrix4 mtxRot;
		Quaternion quanRot;
		quanRot.fromAxisAngle(originalDirection, selfRotation*Math::DEG2RAD);
		quanRot = quanRot*oritation;
		quanRot.toMat4(mtxRot);
		mtxLocal = mtxLocal * mtxRot;

		mtxLocal.translate(position);

		for(ui32 i=0; i<mRows*mColumns; ++i)
		{
			aabb.addPoint(mpVectorArray[i]*mtxLocal);
		}
	}

	void EffectLayer3DGrid::getPropertyList(PropertyList & list)
	{
		EffectLayer::getPropertyList(list);
		list.push_back(token[TOKEN_LY_ES_ROW]);
		list.push_back(token[TOKEN_LY_ES_COLUMN]);
		list.push_back(token[TOKEN_LY_ES_INTERVAL]);
		list.push_back(token[TOKEN_LY_ES_OFFVIEWPORT]);
		list.push_back(token[TOKEN_LY_ES_DECAL]);
	}

	bool EffectLayer3DGrid::getPropertyType(const String & name, PropertyType &type)
	{
		if(EffectLayer::getPropertyType(name, type))
			return true;

		if (name == token[TOKEN_LY_ES_ROW])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_ES_COLUMN])
		{
			type = IElement::PT_INT; return true;
		}
		else if(name == token[TOKEN_LY_ES_INTERVAL])
		{
			type = IElement::PT_REAL; return true;
		}
		else if(name == token[TOKEN_LY_ES_OFFVIEWPORT])
		{
			type = IElement::PT_REAL; return true;
		}
		else if (name == token[TOKEN_LY_ES_DECAL])
		{
			type = IElement::PT_BOOL; return true;
		}
		return false;
	}

	bool EffectLayer3DGrid::getPropertyValue(const String & name, String &value)
	{
		if(EffectLayer::getPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_ES_ROW])
		{
			value = StringUtil::ToString(mRows); return true;
		}
		else if (name == token[TOKEN_LY_ES_COLUMN])
		{
			value = StringUtil::ToString(mColumns); return true;
		}
		else if (name == token[TOKEN_LY_ES_INTERVAL])
		{
			value = StringUtil::ToString(mInterval); return true;
		}
		else if (name == token[TOKEN_LY_ES_OFFVIEWPORT])
		{
			value = StringUtil::ToString(mOffViewport); return true;
		}
		else if (name == token[TOKEN_LY_ES_DECAL])
		{
			value = StringUtil::ToString(mbDecal); return true;
		}

		return false;
	}

	bool EffectLayer3DGrid::setPropertyValue(const String & name,const String & value)
	{
		if(EffectLayer::setPropertyValue(name, value))
			return true;

		if (name == token[TOKEN_LY_ES_ROW])
		{
			ui32 rows = StringUtil::ParseInt(value);
			_adjustGridRows(rows);
			if(mCallbackFunction)
				mCallbackFunction(this);
			return true;
		}
		else if (name == token[TOKEN_LY_ES_COLUMN])
		{
			ui32 columns = StringUtil::ParseInt(value);
			_adjustGridColumns(columns);
			if(mCallbackFunction)
				mCallbackFunction(this);
			return true;
		}
		else if (name == token[TOKEN_LY_ES_INTERVAL])
		{
			float interval = StringUtil::ParseReal(value);
			_adjustGridInterval(interval);
			if(mCallbackFunction)
				mCallbackFunction(this);
			return true;
		}
		else if (name == token[TOKEN_LY_ES_OFFVIEWPORT])
		{
			mOffViewport = StringUtil::ParseReal(value); return true;
		}
		else if (name == token[TOKEN_LY_ES_DECAL])
		{
			mbDecal = StringUtil::ParseBool(value); return true;
		}

		return false;
	}

	bool EffectLayer3DGrid::modifyDecalY(int i, int j)
	{
		// 有效
		if (mDecalInfoArray[i*mColumns + j].valid)
		{
			return true;
		}

		// 检测过且无效
		if (mDecalInfoArray[i*mColumns + j].checkHit)
		{
			return false;
		}
		
		Vector3	 hitPos;
		Vector3	 hitNormal;
		Vector3	 _beginPos = mDecalInfoArray[i*mColumns + j].beginPos;
		Real	 distance = DECAL_CHECK_UP + DECAL_CHECK_DOWN;
		_beginPos.y += DECAL_CHECK_UP;

		bool valid = false;
		/*if (SceneManager::instance()->PxRaycast(_beginPos, DECAC_CHECK_DIR, distance, hitPos, hitNormal))
		{
			hitNormal.normalize();
			if (hitNormal.y > DECAL_CHECK_ANGLE)
				valid = true;
		}*/

		if (valid)
		{
			mDecalInfoArray[i*mColumns + j].checkHit = true;
			mDecalInfoArray[i*mColumns + j].valid = true;
			mDecalInfoArray[i*mColumns + j].y = hitPos.y;
			return true;
		}

		mDecalInfoArray[i*mColumns + j].checkHit = true;
		mDecalInfoArray[i*mColumns + j].valid = false;
		return false;
	}

	void EffectLayer3DGrid::resetCheckAround(list<DecalInfo*>::type& unChecks)
	{
		while (!unChecks.empty())
		{
			DecalInfo* pInfo = unChecks.front();
			unChecks.pop_front();
			mDecalInfoArray[pInfo->index[0] * mColumns + pInfo->index[1]].checkAround = false;
		}
	}

	bool EffectLayer3DGrid::checkDecalAroundY(int x, int y, list<DecalInfo*>::type& unChecks)
	{
		mDecalInfoArray[x*mColumns + y].checkAround = true;
		int step[4][2] = { 0, 1, 0, -1, 1, 0, -1, 0 };
		for (int i = 0; i < 4;++i)
		{
			int _x = x + step[i][0];
			int _y = y + step[i][1];
			if (_x >= 0 && _y >= 0 && _x < mRows && _y < mColumns)
			{
				if (modifyDecalY(_x, _y))
				{
					mDecalInfoArray[x*mColumns + y].valid = true;
					mDecalInfoArray[x*mColumns + y].y = mDecalInfoArray[_x*mColumns + _y].y;
					resetCheckAround(unChecks);
					return true;
				}
				if (!mDecalInfoArray[_x*mColumns + _y].checkAround)
				{
					mDecalInfoArray[_x*mColumns + _y].checkAround = true;
					unChecks.push_back(&mDecalInfoArray[_x*mColumns + _y]);
				}
					
			}
		}

		if (!unChecks.empty())
		{
			DecalInfo* pInfo = unChecks.front();
			unChecks.pop_front();
			if (checkDecalAroundY(pInfo->index[0], pInfo->index[1], unChecks))
			{
				mDecalInfoArray[x*mColumns + y].valid = true;
				mDecalInfoArray[x*mColumns + y].y = mDecalInfoArray[pInfo->index[0]*mColumns + pInfo->index[1]].y;
				resetCheckAround(unChecks);
				return true;
			}
		}

		return false;
	}

}