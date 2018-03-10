#ifndef _DOF_MANAGER_H_
#define _DOF_MANAGER_H_

#include "engine/core/Memory/MemAllocDef.h"

namespace Echo
{
	class DofManager
	{
	public:
		DofManager();
		~DofManager();

		enum DOF_PARAM
		{
			DOF_Param_Fource = 1,
			DOF_Param_Attenuation,
			DOF_Param_Bright,
		};
	public:
		void setEnableDof(bool _val);

		bool getEnableDof()const;

		void setDofParam(float focus, float attenuation);

		void setBright(float bright);

		float getParam(DofManager::DOF_PARAM param_type);

		void setEnableGuassBlur(bool _val);

		void setGuassBright(float bright);

		bool getEnableGuassBlur()const;

		void cacheGuassBlurOneFrame();

		void uncacheGuassBlurOneFrame();

		void setBloomStatus(bool val) { m_bloomEnabled = val; }

		void setMotionBlurEnable(bool enable);

		bool getMotionBlurEnable() const;

		void setMotionBlurParam(float param_value);

		float getMotionBlurParam();

	private:
		bool m_bloomEnabled;
		bool m_gaussianBlur;
		bool m_dofEnabled;
	};
}
#endif