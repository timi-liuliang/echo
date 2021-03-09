#include "FilePackage.h"
#include "engine/core/util/PathUtil.h"
#include "engine/core/io/memory_reader.h"
#include "engine/core/io/stream/MemoryDataStream.h"
#include "zlib/zlib.h"

namespace Echo
{
	static void* zlibAlloc(void* opaque,unsigned int items,unsigned int size)
	{
		return EchoMalloc(size*items);
	}

	static void zlibFree(void* opaque,void* ptr)
	{
		EchoSafeFree(ptr);
	}

	FilePackage::FilePackage(const char* packageFile)
	{
        m_packageFile = packageFile;
        String packageName = PathUtil::GetPureFilename(m_packageFile, false);
        if(m_reader.load(m_packageFile.c_str()))
        {
            for(const String& name : m_reader.getBinaryNames())
            {
                m_files["Res://" + packageName + "/" + name] = name;
            }
        }
	}

	FilePackage::~FilePackage()
	{

	}

	DataStream* FilePackage::open(const char* fileName)
	{
        auto it = m_files.find(fileName);
        if(it!=m_files.end())
        {
            XmlBinaryReader::Data data;
            if(m_reader.getData(it->second.c_str(), data))
            {
                MemoryDataStream* stream = EchoNew(MemoryDataStream(data.m_size, true));
                std::memcpy(stream->getPtr(), data.m_data.data(), data.m_size);
                
                return stream;
            }
        }
        
		return nullptr;
	}

    bool FilePackage::isExist(const String& filename)
    {
        return m_files.find(filename) != m_files.end();
    }

	void FilePackage::compressFolder(const char* inFolderPath)
	{
		String folderPath = inFolderPath;
		PathUtil::FormatPath(folderPath, false);

		XmlBinaryWriter	writer;

		StringArray allFiles;
		PathUtil::EnumFilesInDir(allFiles, folderPath, false, true, true);
		for (String file : allFiles)
		{
			MemoryReader fileReader(file);
			if (fileReader.getSize())
			{
				file = StringUtil::Replace(file, folderPath, "");
				writer.addData(file.c_str(), "Uncompress", fileReader.getData<void*>(), fileReader.getSize());
			}
		}

		folderPath.pop_back();
		String packagPathName = folderPath + ".pkg";
		writer.save(packagPathName.c_str());
	}

	int FilePackage::uncompress(unsigned char* dest, unsigned int* destLen, const unsigned char* source, unsigned int sourceLen)
	{
		z_stream stream;
		int err;

		stream.next_in = (Bytef*)source;
		stream.avail_in = (unsigned int)sourceLen;
		if (stream.avail_in != sourceLen) 
			return Z_BUF_ERROR;

		stream.next_out = dest;
		stream.avail_out = *destLen;
		if (stream.avail_out != *destLen) 
			return Z_BUF_ERROR;

		stream.zalloc = &zlibAlloc;
		stream.zfree = &zlibFree;

		err = inflateInit(&stream);
		if (err != Z_OK) return err;

		err = inflate(&stream, Z_FINISH);
		if (err != Z_STREAM_END)
		{
			inflateEnd(&stream);
			return err == Z_OK ? Z_BUF_ERROR : err;
		}
		*destLen = static_cast<unsigned int>(stream.total_out);

		err = inflateEnd(&stream);
		return err;
	}

	int	FilePackage::compress(unsigned char* dest, unsigned int* destLen, const unsigned char* source, unsigned int sourceLen)
	{
		z_stream stream;
		int err;

		stream.next_in = (Bytef*)source;
		stream.avail_in = (unsigned int)sourceLen;
		stream.next_out = dest;
		stream.avail_out = (unsigned int)*destLen;
		if ((uLong)stream.avail_out != *destLen) 
			return Z_BUF_ERROR;

		stream.zalloc = &zlibAlloc;
		stream.zfree = &zlibFree;
		stream.opaque = (void*)0;

		err = deflateInit(&stream, Z_BEST_SPEED);
		if (err != Z_OK) return err;

		err = deflate(&stream, Z_FINISH);
		if (err != Z_STREAM_END) 
		{
			deflateEnd(&stream);
			return err == Z_OK ? Z_BUF_ERROR : err;
		}
		*destLen = static_cast<unsigned int>(stream.total_out);

		err = deflateEnd(&stream);
		return err;
	}
}
