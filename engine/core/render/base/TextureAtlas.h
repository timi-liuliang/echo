#pragma once

#include "engine/core/resource/Res.h"

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

		// texture res path
		void setTextureRes(const ResourcePath& path);
		const ResourcePath& getTextureRes() { return m_textureRes; }

		// add
		void addAtla(const String& name, const Vector4& viewPort);

		// clear
		void clear();

        // load
        static Res* load(const ResourcePath& path);

        // save
        virtual void save() override;

    protected:
		vector<Atla>::type	m_atlas;
		ResourcePath		m_textureRes = ResourcePath("", ".png");
    };
    typedef ResRef<TextureAtlas> TextureAtlasPtr;
}
