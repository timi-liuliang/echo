#ifndef __ECHO_EFFECT_LAYER_3DGRID_H__
#define __ECHO_EFFECT_LAYER_3DGRID_H__

#include "EffectLayer.h"

namespace Echo
{
	class EffectLayer3DGrid;
	typedef void (*ES_FUN)(EffectLayer3DGrid* ptr);

	class EffectLayer3DGrid : public EffectLayer
	{
	public:
		virtual void getPropertyList(PropertyList & list);
		virtual bool getPropertyType(const String & name, PropertyType &type);
		virtual bool getPropertyValue(const String & name, String &value);
		virtual bool setPropertyValue(const String & name,const String & value);

	public:
		EffectLayer3DGrid();
		virtual ~EffectLayer3DGrid();

		//
		void setCallBackFunction(ES_FUN fun);

		virtual void getMatrixWorld(Matrix4& mtxWorld);
		virtual void updateRenderData();
		
		// 提交到渲染队列
		virtual void submitToRenderQueue(std::vector<EffectRenderable*>& renderables);

		virtual void prepare();
		virtual void unprepare();

		virtual void importData(DataStream* pStream, int version);
		virtual void exportData(DataStream* pStream, int version);
		virtual void mergeAligenBox(Box& aabb);

		virtual void copyAttributesTo(EffectParticle* particle);

		// for editor.
		size_t getRows() const { return mRows; }
		size_t getColumns() const { return mColumns; }
		Vector3 getSurfaceVector(int row, int column)const;
		void setSurfaceVector(int row, int column, const Vector3& newPos);
		Vector3 getSurfaceVector(int index)const;
		void setSurfaceVector(int index, const Vector3& newPos);
		Color getSurfaceColor(int row, int column) const;
		void setSurfaceColor(int row, int column, const Color& newColor);
		Color getSurfaceColor(int index)const;
		void setSurfaceColor(int index, const Color& newColor);
		virtual const Quaternion& getSurfaceOritation() const;
	private:
		struct DecalInfo
		{
			
			int		index[2];
			bool	checkHit;
			bool	checkAround;
			bool	valid;
			Vector3	beginPos;
			float	y;
		}; 

		bool modifyDecalY(int x, int y);

		bool checkDecalAroundY(int x, int y, list<DecalInfo*>::type& unChecks);

		void resetCheckAround(list<DecalInfo*>::type& unChecks);

	protected:
		void _adjustGridRows(size_t rows);
		void _adjustGridColumns(size_t columns);
		void _adjustGridInterval(float interval);

		ui32		mRows;
		ui32		mColumns;
		float		mInterval;				
		Vector3*	mpVectorArray;			
		Color*		mpColorArray;			
		float		mOffViewport;
		Box			mGridBox;
		ES_FUN		mCallbackFunction;
		bool		mbDecal;
		DecalInfo*  mDecalInfoArray;
	};
}


#endif