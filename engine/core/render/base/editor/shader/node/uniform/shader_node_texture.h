#pragma once

#include "../shader_node.h"

namespace Echo
{
#ifdef ECHO_EDITOR_MODE
	class ShaderNodeTexture : public ShaderNode
	{
		ECHO_CLASS(ShaderNodeTexture, ShaderNode)

	public:
		ShaderNodeTexture();
		virtual ~ShaderNodeTexture();

		// name
		virtual QString name() const override { return "Texture"; }

		// caption
		virtual QString caption() const override { return getVariableName().c_str(); }

	private:
		// get default value
		virtual bool getDefaultValue(Echo::StringArray& uniformNames, Echo::VariantArray& uniformValues) override;

		// set in
		virtual void setInData(std::shared_ptr<NodeData> nodeData, int portIndex) override;

		// generate code
		virtual bool generateCode(Echo::ShaderCompiler& compiler) override;

	protected:
		// get embedded widget
		virtual QWidget* embeddedWidget() override { return m_toolButton; }

	public:
		String getVariableName() const { return m_variableName.empty() ? StringUtil::Format("Texture%d", getId()) : m_variableName; }
		void setVariableName(const String& variableName);

		// type
		const StringOption& getType() { return m_type; }
		void setType(const StringOption& type) { m_type.setValue(type.getValue()); }

		// atlas
		void setAtla(bool isAtla) { m_isAtla = isAtla; }
		bool isAtla() const { return m_isAtla; }

		// default texture
		const ResourcePath& getTexture() const { return m_texture; }
		void setTexture(const ResourcePath& texture) { m_texture.setPath(texture.getPath()); }

	private:
		// helper function, update output's variable name
		void updateOutputDataVariableName();

	protected:
		String			m_variableName = "";
		bool			m_isAtla = false;
		StringOption	m_type = StringOption("General", { "General", "NormalMap" });
		ResourcePath	m_texture;
		QToolButton*	m_toolButton = nullptr;
	};

#endif
}
