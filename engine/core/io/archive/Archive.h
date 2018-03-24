#pragma once

#include <functional>
#include "engine/core/Util/StringUtil.h"
#include "engine/core/io/DataStream.h"

namespace Echo
{
	typedef std::function<bool(const Echo::String&, DataStream*)>	 EchoOpenResourceCb;

	class Archive;
	struct FileInfo 
	{
		Archive* archive;				// The archive in which the file has been found (for info when performing
		String filename;				// The file's fully qualified name
		String path;					// Path name; separated by '/' and ending with '/'
		String basename;				// Base filename
		size_t compressedSize;			// Compressed size
		size_t uncompressedSize;		// Uncompressed size
	};
	typedef vector<FileInfo>::type FileInfoList;

	class Archive
	{
	public:
		// create
		static Archive* create( const String& type);

		// load
		static Archive* load(const String& name, const String& type);

	public:
		/// Get the name of this archive
		const String& getName(void) const { return mName; }

		/// Returns whether this archive is case sensitive in the way it matches files
		virtual bool isCaseSensitive(void) const = 0;

		/** Loads the archive.
		@remarks
		This initializes all the internal data of the class.
		@warning
		Do not call this function directly, it is meant to be used
		only by the ArchiveManager class.
		*/
		virtual void load() = 0;

		/** Unloads the archive.
		@warning
		Do not call this function directly, it is meant to be used
		only by the ArchiveManager class.
		*/
		virtual void unload() = 0;

		/** Open a stream on a given file. 
		@note
		There is no equivalent 'close' method; the returned stream
		controls the lifecycle of this file operation.
		@param filename The fully qualified name of the file
		@param readOnly Whether to open the file in read-only mode or not (note, 
		if the archive is read-only then this cannot be set to false)
		@returns A shared pointer to a DataStream which can be used to 
		read / write the file. If the file is not present, returns a null
		shared pointer.
		*/
		virtual DataStream* open(const String& filename) = 0;

		// only ok in filesystem.
		virtual String location(const String& filename) { return StringUtil::BLANK; }

		/** List all file names in the archive.
		@note
		This method only returns filenames, you can also retrieve other
		information using listFileInfo.
		@param recursive Whether all paths of the archive are searched (if the 
		archive has a concept of that)
		@param dirs Set to true if you want the directories to be listed
		instead of files
		@returns A list of filenames matching the criteria, all are fully qualified
		*/
		virtual StringArray list(bool recursive = true, bool dirs = false) = 0;

		/** List all files in the archive with accompanying information.
		@param recursive Whether all paths of the archive are searched (if the 
		archive has a concept of that)
		@param dirs Set to true if you want the directories to be listed
		instead of files
		@returns A list of structures detailing quite a lot of information about
		all the files in the archive.
		*/
		virtual FileInfoList* listFileInfo(bool dirs = false) = 0;

		/** Find out if the named file exists (note: fully qualified filename required) */
		virtual bool exists(const String& filename) = 0; 

		/// Return the type code of this Archive
		const String& getType(void) const { return mType; }

		// 是否可以添加文件
		virtual bool isCanAddFile(const String& fullPath) { return false; }

		// 添加文件
		virtual bool addFile(const String& fullPath) { return false; }

		// 移除文件
		virtual bool removeFile( const char* fileName) { return false; }

		// 判断文件是否异步加载文件
		virtual bool isAsync( const char* fileName) = 0;

		// 设置异步加载回调
		virtual bool addAsyncCompleteCallback(const Echo::String& name, EchoOpenResourceCb callback) = 0;

		// constructor
		Archive(const String& name, const String& archType)
			: mName(name), mType(archType) 
		{}

		// destructor
		virtual ~Archive()
		{}

	protected:
		/// Archive name
		String mName;
		/// Archive type code
		String mType;
	};
}
