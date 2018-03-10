#pragma once

#include "RenderStage.h"
#include "engine/core/render/render/Material.h"

namespace Echo
{
	/**
	* LDRRenderStage
	*/
	class QueryObject;
	class CoverageRenderStage : public RenderStage
	{
		friend class RenderStageManager;
		typedef vector<QueryObject*>::type QueryObjectVector;
	public:
		CoverageRenderStage();
		virtual ~CoverageRenderStage();

		// 初始化
		virtual bool initialize();

		// 执行渲染
		virtual void render();

		// 销毁
		virtual void destroy();

		virtual Echo::ShaderProgram* getProgram(){ return m_pMtlCoverage->getShaderProgram(); }

		// 添加查询对像
		void addQueryObject(class QueryObject* pQo);

		// 清除查询对像
		void clearQueryObjects() { m_vecQueryObjects.clear(); }

	protected:
		QueryObjectVector m_vecQueryObjects;

	private:
		const SamplerState* m_coverageSamplerState;

		class Material* m_pMtlCoverage;
	};
}