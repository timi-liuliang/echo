#include "engine/core/io/IO.h"
#include "base/texture/texture.h"
#include <spine/extension.h>

// create texture res
void _spAtlasPage_createTexture(spAtlasPage* self, const char* path)
{
	Echo::Texture* texRes = (Echo::Texture*)Echo::Res::get(Echo::ResourcePath(path));
	//texRes->prepareLoad();

	self->rendererObject = texRes;
}

// dispose texture
void _spAtlasPage_disposeTexture(spAtlasPage* self)
{
	Echo::Texture* texRes = static_cast<Echo::Texture*>(self->rendererObject);
	texRes->subRefCount();
}

// read file
char* _spUtil_readFile(const char* path, int* length)
{
	Echo::DataStream* stream = Echo::IO::instance()->open(path);
	if (stream)
	{
		char* data = MALLOC(char, stream->size() + 1);
		*length = int(stream->size());

		stream->read(data, stream->size());
		data[stream->size()] = '\0';
		EchoSafeDelete(stream, DataStream);

		return data;
	}
	else
	{
		*length = 0;
		return NULL;
	}
}
