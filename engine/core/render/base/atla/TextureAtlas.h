#pragma once

#include "engine/core/resource/Res.h"
#include "engine/core/render/base/Texture.h"

namespace Echo
{
    class TextureAtlas : public Res
    {
        ECHO_RES(TextureAtlas, Res, ".atlas", Res::create<TextureAtlas>, TextureAtlas::load);

	public:
		// Atla
		struct Atla
		{
			String	 m_name;
			Vector4  m_viewPort;
		};

    public:
        TextureAtlas();
        TextureAtlas(const ResourcePath& path);
        virtual ~TextureAtlas();

		// is a package
		virtual bool isPackage() const { return true; }

		// texture res path
		void setTextureRes(const ResourcePath& path);
		const ResourcePath& getTextureRes() { return m_textureRes; }

		// texture
		const TexturePtr& getTexture() { return m_texture; }

		// add
		void addAtla(const String& name, const Vector4& viewPort);
		void removeAtla(const String& name);

		// get viewport
		bool getViewport(const String& name, Vector4& viewPort);

		// get all atlas
		const vector<Atla>::type& getAllAtlas() { return m_atlas; }

		// clear
		void clear();

        // load | save
        static Res* load(const ResourcePath& path);
        virtual void save() override;

	protected:
		// enum files
		virtual void enumFilesInDir(StringArray& ret, const String& rootPath, bool bIncDir = false, bool bIncSubDirs = false, bool isAbsPath = false);

    protected:
		vector<Atla>::type	m_atlas;
		ResourcePath		m_textureRes = ResourcePath("", ".png");
		TexturePtr			m_texture;
    };
    typedef ResRef<TextureAtlas> TextureAtlasPtr;
}
