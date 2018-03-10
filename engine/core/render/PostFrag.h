#pragma once

#include "PostImageEffect.h"

namespace Echo
{
	class PostProcessRenderStage;
	class PostFrag : public PostImageEffectPass
	{
		typedef vector<std::pair<Material*, Renderable*> >::type Materials;
	public:
		PostFrag(PostImageEffect* parent);
		~PostFrag();

		virtual void process() override;
		virtual void setParameter(const String& uniform, const Vector4& value) override;
		virtual bool find(const String& uniform, Vector4& value) const;

		void setForEffect(const String& name, const String& uniform, const Vector4& value);
		bool findForEffect(const String& name, const String& uniform, Vector4& value) const;

	protected:
		virtual Material* createMaterial(const String& fileName);
		virtual Renderable* createRenderable(Material* material);
		virtual void initializeShaderParams();
	private:
		Materials m_materials;
		PostProcessRenderStage* m_renderStage;
		StringArray m_effects;
		ulong m_index;
	};

}  // namespace Echo
