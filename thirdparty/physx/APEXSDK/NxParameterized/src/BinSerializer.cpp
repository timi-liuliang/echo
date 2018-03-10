/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


/*
	Binary serialization is very complex and error-prone
	process. Ensuring that it works properly is _critically_
	important for the engine.

	Before you do any significant changes to this code
	please get familiar with C/C++ object layouts.
	You may find these links helpful:

	* APEX binary file format
		https://wiki.nvidia.com/engwiki/index.php/PhysX/APEX/architecture/design/File_Formats
		(may not be up-to-date)

	* C++ data alignment and portability
		http://www.codesynthesis.com/~boris/blog/2009/04/06/cxx-data-alignment-portability/

	* Data structure alignment
		http://en.wikipedia.org/wiki/Data_structure_alignment

	* About Data Alignment
		http://msdn.microsoft.com/en-us/library/ms253949%28v=VS.80%29.aspx

	* Types and Storage
		http://msdn.microsoft.com/en-us/library/02c56cw3.aspx

	* Jan Gray's article in "C++: Under the Hood"
		http://www.openrce.org/articles/files/jangrayhood.pdf

	* Windows Data Alignment on IPF, x86, and x64
		http://msdn.microsoft.com/en-us/library/aa290049%28VS.71%29.aspx

	* GCC alignment in classes gotcha
		https://ps3.scedev.net/forums/thread/41593/

	* all messages in thread http://compilers.iecc.com/comparch/article/94-08-119

	* Memory Layout for Multiple and Virtual Inheritance
		http://www.phpcompiler.org/articles/virtualinheritance.html

	* The "Empty Member" C++ Optimization
		http://www.cantrip.org/emptyopt.html

	You may also want to see examples of existing ABI
	for various platforms:

	* Agner Fog's Calling conventions for different C++ compilers
		and operating systems,
		http://www.agner.org/optimize/calling_conventions.pdf

	* Itanium C++ ABI Data Layout
		http://www.codesourcery.com/public/cxx-abi/abi.html#layout

	* Introduction to Mac OS X ABI Function Call Guide
		http://developer.apple.com/library/mac/#documentation/DeveloperTools/Conceptual/LowLevelABI/000-Introduction/introduction.html

	After you implement you changes be sure to check that ALL
	NxParameterized unit tests
	* Test/MediaTest
	* Test/RandomClasses
	* Test/ParamRandomTest
	run without errors on ALL supported platforms:
	* Win32
	* Win64
	* PS3
	* XBOX 360
	(in future we may also have Mac, Tegra, Linux on PC, etc.).

	If you need to add new platform to this list, be sure to
	analyze it's ABI with ABIDumper
	(//sw/physx/shared/general/ABIDumper) and check whether
	it requires special changes in current serialization code
	(Mac does).

	If you ever implement new features be sure to cover them
	in random schema generator
	NxParameterized/scripts/RandomSchema.pl, generate a bunch
	(10 at least) of new random schemas for Test/RandomClasses
	and add them to p4 (and verify that they work on all
	platforms!).
*/

#include "PsShare.h"
#include "PsArray.h"
#include "PsHash.h"
#include "PsHashMap.h"

#include "NxParameters.h"
#include "NxParamUtils.h"
#include "NxTraitsInternal.h"

#include "BinaryHelper.h"

#include "PlatformABI.h"
#include "PlatformInputStream.h"
#include "PlatformOutputStream.h"

#include "BinSerializer.h"

namespace NxParameterized
{

// Verify that file header is ok
Serializer::ErrorType BinSerializer::verifyFileHeader(
	const BinaryHeader &hdr,
	const BinaryHeaderExt *ext,
	physx::PxU32 dataLen ) const
{
	NX_BOOL_ERR_CHECK_WARN_RETURN( BinSerializer::Magic == hdr.magic, Serializer::ERROR_INVALID_MAGIC, "Invalid APB file: magic number does not match: %x", hdr.magic );
	NX_BOOL_ERR_CHECK_WARN_RETURN( (physx::PxU32)BINARY_TYPE_PLAIN == hdr.type, Serializer::ERROR_INVALID_FILE_FORMAT, "Invalid APB file: unknown file format: %d", hdr.type );

	NX_BOOL_ERR_CHECK_WARN_RETURN(
		BinVersions::WithExtendedHeader == hdr.version || BinVersions::WithAlignment == hdr.version,
		Serializer::ERROR_INVALID_FILE_VERSION, "Invalid APB header: unexpected version: %x", hdr.version );

	// Verify offsets
	NX_BOOL_ERR_CHECK_WARN_RETURN( hdr.fileLength <= dataLen
			&& hdr.dataOffset < hdr.fileLength
			&& hdr.relocOffset < hdr.fileLength
			&& hdr.dictOffset < hdr.fileLength
			&& hdr.metadataOffset < hdr.fileLength,
		Serializer::ERROR_INVALID_INTERNAL_PTR,
		"Invalid APB file: section offsets do not match file length" );

	if( ext )
	{
		NX_BOOL_ERR_CHECK_WARN_RETURN(
			0 == memcmp((const char *)&ext->vcsSafetyFlags, VCS_SAFETY_FLAGS, 4),
			Serializer::ERROR_INVALID_CHAR,
			"Invalid APB file: perhaps your editor inserts redundant carriage returns?"
		);
	}

	return Serializer::ERROR_NONE;
}

// Peeks APB header from stream
static Serializer::ErrorType peekFileHeader(physx::PxFileBuf &stream, BinaryHeader &hdr, BinaryHeaderExt &ext, bool doDecanonize = true)
{
	NX_BOOL_ERR_CHECK_RETURN(
		sizeof(BinaryHeader) == stream.peek(&hdr, sizeof(BinaryHeader)),
		Serializer::ERROR_INVALID_FILE_FORMAT );

	if( hdr.version >= BinVersions::WithExtendedHeader )
	{
		physx::PxU32 fileOff = stream.tellRead();

		stream.seekRead(fileOff + sizeof(BinaryHeader));
		NX_BOOL_ERR_CHECK_RETURN(
			sizeof(BinaryHeaderExt) == stream.peek(&ext, sizeof(BinaryHeaderExt)),
			Serializer::ERROR_INVALID_FILE_FORMAT );

		stream.seekRead(fileOff);
	}

	if( doDecanonize )
		hdr.decanonize();

	return Serializer::ERROR_NONE;
}

// Extract platform ABI from file header
Serializer::ErrorType BinSerializer::getPlatformInfo(
	BinaryHeader &hdr,
	BinaryHeaderExt *ext,
	PlatformABI &abi ) const
{
	//Difference between these versions is not important for non-inplace serializer
	if( hdr.version == BinVersions::Initial || hdr.version == BinVersions::AllRefsCounted )
		hdr.version = BinVersions::WithAlignment;

	NX_ERR_CHECK_RETURN( verifyFileHeader(hdr, ext, hdr.fileLength) );

	SerializePlatform platform;
	NX_ERR_CHECK_WARN_RETURN(hdr.getPlatform(platform), "Invalid platform");

	NX_ERR_CHECK_WARN_RETURN( PlatformABI::GetPredefinedABI(platform, abi), "Unknown platform" );

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::peekInplaceAlignment(physx::PxFileBuf& stream, physx::PxU32& align)
{
	BinaryHeader hdr;
	BinaryHeaderExt ext;
	NX_ERR_CHECK_RETURN( peekFileHeader(stream, hdr, ext, false) );
	align = Canonize(hdr.alignment);
	return Serializer::ERROR_NONE;
}

// Verify that object matches its header
Serializer::ErrorType BinSerializer::verifyObjectHeader(const ObjHeader &hdr, const Interface *obj, Traits *traits) const
{
	DEBUG_ASSERT(obj);
	NX_BOOL_ERR_CHECK_WARN_RETURN(
		obj->version() == hdr.version,
		Serializer::ERROR_UNKNOWN,
		"Invalid object: version does not match object header" );

	PX_UNUSED(traits);

	physx::PxU32 bits = PX_MAX_U32;
	const physx::PxU32 *checksum = obj->checksum(bits);

	if( !DoIgnoreChecksum(*obj) || hdr.version != obj->version() )
	{
		if( bits != 32 * hdr.checksumSize )
			NX_PARAM_TRAITS_WARNING(
				traits,
				"Schema checksum is different for object of class %s and version %u, asset may be corrupted",
				hdr.className, (unsigned)hdr.version
			);
		else
		{
			for(physx::PxU32 i = 0; i < hdr.checksumSize; ++i)
				if( checksum[i] != hdr.checksum[i] )
				{
					NX_PARAM_TRAITS_WARNING(
						traits,
						"Schema checksum is different for object of class %s and version %u, asset may be corrupted",
						hdr.className, (unsigned)hdr.version
					);
					break;
				}
		}
	}

	return Serializer::ERROR_NONE;
}

#if 0
//TODO: this is just for tests, remove it after metadata works
static void DumpDefinition(const Definition *def, const char *className, physx::PxU32 version, physx::PxU32 off = 0)
{
	char *tab = (char *)::malloc(off + 1);
	memset(tab, ' ', off);
	tab[off] = 0;

	if( className )
		printf("%sClass %s:%d\n", tab, className, version);

	printf("%sDefinition at 0x%p:\n", tab, def);
	printf("%s  name =  %s\n", tab, def->name());
	printf("%s  type =  %s\n", tab, typeToStr(def->type()));
	printf("%s  align = %d\n", tab, def->alignment());
	printf("%s  pad = %d\n", tab, def->padding());

	printf("%s  %d hints:\n", tab, def->numHints());
	for(physx::PxI32 i = 0; i < def->numHints(); ++i)
	{
		const Hint *hint = def->hint(i);

		printf("%s    %s => ", tab, hint->name());
		switch( hint->type() )
		{
		case TYPE_U64:
			printf("%llu", hint->asUInt());
			break;
		case TYPE_F64:
			printf("%f", hint->asFloat());
			break;
		case TYPE_STRING:
			printf("\"%s\"", hint->asString());
			break;
		default:
			DEBUG_ALWAYS_ASSERT();
		}
		printf("\n");
	}

	printf("%s  %d enums: ", tab, def->numEnumVals());
	for(physx::PxI32 i = 0; i < def->numEnumVals(); ++i)
		printf("\"%s\", ", def->enumVal(i));
	printf("\n");

	printf("%s  %d refVariants: ", tab, def->numRefVariants());
	for(physx::PxI32 i = 0; i < def->numRefVariants(); ++i)
		printf("\"%s\", ", def->refVariantVal(i));
	printf("\n");

	printf("%s  %d children:\n", tab, def->numChildren());
	for(physx::PxI32 i = 0; i < def->numChildren(); ++i)
	{
		printf("%sChild %d:\n", tab, i);
		DumpDefinition(def->child(i), 0, 0, off + 1);
	}

	::free(tab);

	fflush(stdout);
}
#endif

Serializer::ErrorType BinSerializer::peekNumObjects(physx::PxFileBuf &stream, physx::PxU32 &numObjects)
{
	BinaryHeader hdr;
	BinaryHeaderExt ext;
	NX_ERR_CHECK_RETURN( peekFileHeader(stream, hdr, ext, false) );
	
	return peekNumObjectsInplace((const void *)&hdr, sizeof(hdr), numObjects);
}

Serializer::ErrorType BinSerializer::peekNumObjectsInplace(const void *data, physx::PxU32 dataLen, physx::PxU32 &numObjects)
{
	NX_BOOL_ERR_CHECK_RETURN( dataLen >= sizeof(BinaryHeader), Serializer::ERROR_INVALID_FILE_FORMAT );
	numObjects = Canonize( (physx::PxU32)((const BinaryHeader *)data)->numObjects );
	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::peekClassNames(physx::PxFileBuf &stream, char **classNames, physx::PxU32 &numClassNames)
{
	BinaryHeader hdr;
	BinaryHeaderExt ext;
	NX_ERR_CHECK_RETURN( peekFileHeader(stream, hdr, ext) );

	PlatformABI targetParams;
	NX_ERR_CHECK_RETURN(getPlatformInfo(
		hdr,
		hdr.version >= BinVersions::WithExtendedHeader ? &ext : 0,
		targetParams
	));

	PlatformInputStream objectTable(stream, targetParams, mTraits);
	objectTable.skipBytes(hdr.dataOffset);

	numClassNames = physx::PxMin(numClassNames, (physx::PxU32)hdr.numObjects);
	for(physx::PxU32 i = 0; i < numClassNames; ++i)
	{
		physx::PxU32 tmp;
		NX_ERR_CHECK_RETURN( objectTable.readPtr(tmp) ); // Pointer to object data

		physx::PxU32 classNameOff;
		NX_ERR_CHECK_RETURN( objectTable.readPtr(classNameOff) ); // Pointer to className

		const char *className;
		NX_ERR_CHECK_RETURN( objectTable.readString(classNameOff, className) );
		classNames[i] = mTraits->strdup(className); //User will call Traits::strfree
		mTraits->free((void *)className);

		NX_ERR_CHECK_RETURN( objectTable.readPtr(tmp) ); // Pointer to name
		NX_ERR_CHECK_RETURN( objectTable.readPtr(tmp) ); // Pointer to filename
	}

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::readMetadataInfo(const BinaryHeader &hdr, PlatformInputStream &s, DefinitionImpl *def)
{
	PX_PLACEMENT_NEW(def, DefinitionImpl)(*mTraits, false);

	s.beginStruct(s.getTargetABI().getMetaInfoAlignment());

	//type
	physx::PxU32 type;
	NX_ERR_CHECK_RETURN( s.read(type) );
	NX_BOOL_ERR_CHECK_WARN_RETURN( type < TYPE_LAST, Serializer::ERROR_INVALID_DATA_TYPE, "Unknown data type: %d", (int)type);

	//arraySize
	physx::PxI32 arraySize;
	NX_ERR_CHECK_RETURN( s.read(arraySize) );

	//shortName
	physx::PxU32 nameOff;
	NX_ERR_CHECK_RETURN( s.readPtr(nameOff) );

	const char *name;
	NX_ERR_CHECK_RETURN( s.readString(nameOff, name) ); //NOTE: we do not release name because DefinitionImpl does not copy it

	//structName
	physx::PxU32 structNameOff;
	NX_ERR_CHECK_RETURN( s.readPtr(structNameOff) );

	const char *structName;
	NX_ERR_CHECK_RETURN( s.readString(structNameOff, structName) ); //NOTE: we do not release name because DefinitionImpl does not copy it

	// We do not have metadata for older formats without alignment
	NX_BOOL_ERR_CHECK_WARN_RETURN( hdr.version >= BinVersions::WithAlignment, Serializer::ERROR_INVALID_FILE_VERSION, "Unable to deserialize metadata: file format does not support it" );

	//alignment
	physx::PxU32 alignment;
	NX_ERR_CHECK_RETURN( s.read(alignment) );

	//padding
	physx::PxU32 padding;
	NX_ERR_CHECK_RETURN( s.read(padding) );

	def->init(name, (DataType)type, structName);
	def->setArraySize(arraySize);
	if( alignment ) def->setAlignment(alignment);
	if( padding ) def->setPadding(padding);

	//numChildren
	physx::PxI32 numChildren;
	NX_ERR_CHECK_RETURN( s.read(numChildren) );

	//children
	for(physx::PxI32 i = 0; i < numChildren; ++i)
	{
		DefinitionImpl *childDef = (DefinitionImpl *)mTraits->alloc(sizeof(DefinitionImpl));

		physx::PxU32 off;
		NX_ERR_CHECK_RETURN( s.readPtr(off) );

		NX_ERR_CHECK_RETURN( s.pushPos(off) );
		NX_ERR_CHECK_RETURN( readMetadataInfo(hdr, s, childDef) );
		s.popPos();

		def->addChild(childDef);
	}

	//numHints
	physx::PxI32 numHints;
	NX_ERR_CHECK_RETURN( s.read(numHints) );

	//hints
	for(physx::PxI32 i = 0; i < numHints; ++i)
	{
		HintImpl *hint = (HintImpl *)mTraits->alloc(sizeof(HintImpl));
		PX_PLACEMENT_NEW(hint, HintImpl)();

		s.beginStruct(s.getTargetABI().getHintAlignment());

		//type
		physx::PxU32 hintType;
		NX_ERR_CHECK_RETURN( s.read(hintType) );

		//name
		physx::PxU32 hintNameOff;
		NX_ERR_CHECK_RETURN( s.readPtr(hintNameOff) );

		const char *hintName;
		NX_ERR_CHECK_RETURN( s.readString(hintNameOff, hintName) );
		Releaser releaseName((void *)hintName, mTraits);

		//val
		s.beginStruct(s.getTargetABI().getHintValueAlignment());
		switch( hintType )
		{
		case TYPE_U64:
			{
				physx::PxU64 val;
				NX_ERR_CHECK_RETURN( s.read(val) );
				hint->init(hintName, val, false);

				break;
			}
		case TYPE_F64:
			{
				physx::PxF64 val;
				NX_ERR_CHECK_RETURN( s.read(val) );
				hint->init(hintName, val, false);

				break;
			}
		case TYPE_STRING:
			{
				physx::PxU32 off;
				NX_ERR_CHECK_RETURN( s.readPtr(off) );

				const char *val;
				NX_ERR_CHECK_RETURN( s.readString(off, val) );
				Releaser releaseVal((void *)val, mTraits);

				hint->init(hintName, val, false);

				break;
			}
		default:
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_DATA_TYPE;
		}
		s.align(s.getTargetABI().getHintValueSize());
		s.closeStruct();

		s.closeStruct();

		def->addHint(hint);
	} //i

	//numEnumVals
	physx::PxI32 numEnumVals;
	NX_ERR_CHECK_RETURN( s.read(numEnumVals) );

	//enumVals
	for(physx::PxI32 i = 0; i < numEnumVals; ++i)
	{
		physx::PxU32 off;
		NX_ERR_CHECK_RETURN( s.readPtr(off) );

		const char *val;
		NX_ERR_CHECK_RETURN( s.readString(off, val) );
		Releaser releaseVal((void *)val, mTraits);

		def->addEnumVal(val);
	}

	//numRefVariants
	physx::PxI32 numRefVariants;
	NX_ERR_CHECK_RETURN( s.read(numRefVariants) );

	//refVariants
	for(physx::PxI32 i = 0; i < numRefVariants; ++i)
	{
		physx::PxU32 off;
		NX_ERR_CHECK_RETURN( s.readPtr(off) );

		const char *val;
		NX_ERR_CHECK_RETURN( s.readString(off, val) );
		Releaser releaseVal((void *)val, mTraits);

		def->addRefVariantVal(val);
	}

	s.closeStruct();

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::deserializeMetadata(physx::PxFileBuf &stream, DeserializedMetadata &desData)
{
	BinaryHeader hdr;
	BinaryHeaderExt ext;
	NX_ERR_CHECK_RETURN( peekFileHeader(stream, hdr, ext) );
	NX_ERR_CHECK_RETURN(verifyFileHeader(
		hdr,
		hdr.version >= BinVersions::WithExtendedHeader ? &ext : 0,
		hdr.fileLength
	));

	PlatformABI targetParams;
	NX_ERR_CHECK_RETURN(getPlatformInfo(
		hdr,
		hdr.version >= BinVersions::WithExtendedHeader ? &ext : 0,
		targetParams
	));

	PlatformInputStream s(stream, targetParams, mTraits);
	s.skipBytes(hdr.metadataOffset);

	physx::PxU32 numMetadata = hdr.numMetadata;

	desData.init(mTraits, numMetadata);

	for(physx::PxU32 i = 0; i < numMetadata; ++i)
	{
		physx::PxU32 classNameOff;
		NX_ERR_CHECK_RETURN( s.readPtr(classNameOff) );

		const char *className;
		NX_ERR_CHECK_RETURN( s.readString(classNameOff, className) );

		physx::PxU32 version;
		NX_ERR_CHECK_RETURN( s.read(version) );

		physx::PxU32 entryOff;
		NX_ERR_CHECK_RETURN( s.readPtr(entryOff) );

		s.pushPos(entryOff);

		DefinitionImpl *def = (DefinitionImpl *)mTraits->alloc(sizeof(DefinitionImpl));
		NX_ERR_CHECK_WARN_RETURN( readMetadataInfo(hdr, s, def), "Failed to read metadata info" );

//		DumpDefinition(def, className, version);

		desData[i].def = def;
		desData[i].version = version;
		desData[i].className = className;

		s.popPos();
	}

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::internalDeserialize(physx::PxFileBuf &stream, Serializer::DeserializedData &res, bool & /*doesNeedUpdate*/)
{
	NX_BOOL_ERR_CHECK_WARN_RETURN(
		PlatformABI::VerifyCurrentPlatform(),
		Serializer::ERROR_INVALID_PLATFORM,
		"Current platform is not supported, aborting" );

	NX_BOOL_ERR_CHECK_RETURN( BinaryHeader::CheckAlignment(), Serializer::ERROR_INVALID_PLATFORM );

	BinaryHeader hdr;
	BinaryHeaderExt ext;
	NX_ERR_CHECK_RETURN( peekFileHeader(stream, hdr, ext) );

	PlatformABI abi;
	NX_ERR_CHECK_RETURN(getPlatformInfo(
		hdr,
		hdr.version >= BinVersions::WithExtendedHeader ? &ext : 0,
		abi
	));

	physx::PxU32 fileOff = stream.tellRead();

	PlatformInputStream data(stream, abi, mTraits);
	NX_ERR_CHECK_RETURN( data.skipBytes(hdr.dataOffset) );

	res.init(mTraits, (physx::PxU32)hdr.numObjects);
	for(physx::PxU32 i = 0; i < (physx::PxU32)hdr.numObjects; ++i)
	{
		physx::PxU32 objOff;
		NX_ERR_CHECK_RETURN( data.readPtr(objOff) );

		if( !objOff )
			res[i] = 0u;
		else
		{
			NX_ERR_CHECK_RETURN( data.pushPos(objOff) );

			Serializer::ErrorType error = readObject(res[i], data);
			if( Serializer::ERROR_NONE != error )
			{
				for(physx::PxU32 j = 0; j < i; ++j) //Last object was destroyed in readObject
					res[j]->destroy();

				res.init(0, 0);

				return error;
			}

			data.popPos();
		}

		physx::PxU32 tmp;
		NX_ERR_CHECK_RETURN( data.readPtr(tmp) ); // className
		NX_ERR_CHECK_RETURN( data.readPtr(tmp) ); // name
		NX_ERR_CHECK_RETURN( data.readPtr(tmp) ); // filename
	} //i

	stream.seekRead(fileOff + hdr.fileLength); //Goto end of stream

	return Serializer::ERROR_NONE;
}

static BinaryReloc *getRelocationTable(const BinaryHeader &hdr, char *start, physx::PxU32 &nrelocs)
{
	char *ptrTable = start + hdr.relocOffset;
	nrelocs = *reinterpret_cast<physx::PxU32 *>(ptrTable);

	ptrTable += PlatformABI::align(4, GetAlignment<BinaryReloc>::value);

	return reinterpret_cast<BinaryReloc *>(ptrTable); //Alignment is ok
}

Serializer::ErrorType BinSerializer::internalDeserializeInplace(void *mdata, physx::PxU32 dataLen, Serializer::DeserializedData &res, bool & doesNeedUpdate)
{
	doesNeedUpdate = false;

	NX_BOOL_ERR_CHECK_WARN_RETURN(
		PlatformABI::VerifyCurrentPlatform(),
		Serializer::ERROR_INVALID_PLATFORM,
		"Current platform is not supported, aborting" );

	char *start = (char *)mdata;

	//Header
	BinaryHeader &hdr = *reinterpret_cast<BinaryHeader *>(start);
	hdr.decanonize(); //Make platform-dependent

	//Extended header
	BinaryHeaderExt *ext = hdr.version >= BinVersions::WithExtendedHeader
		? reinterpret_cast<BinaryHeaderExt *>(start + sizeof(BinaryHeader))
		: 0;
	if( ext ) ext->decanonize();

	//This particular updates can be done on the fly
	//Probably there is no need to set isUpdated because updates can be done on the fly

	if( hdr.version == BinVersions::Initial )
		NX_ERR_CHECK_WARN_RETURN( updateInitial2AllCounted(hdr, start), "Failed to update binary asset from 1.0 to 1.1" );

	if( hdr.version == BinVersions::AllRefsCounted )
		NX_ERR_CHECK_WARN_RETURN( updateAllCounted2WithAlignment(hdr, start), "Failed to update binary asset from 1.1 to 1.2" );

	NX_ERR_CHECK_RETURN( verifyFileHeader(hdr, ext, dataLen) );

	NX_BOOL_ERR_CHECK_RETURN( IsAligned(mdata, hdr.alignment), Serializer::ERROR_UNALIGNED_MEMORY);

	//Verify platform
	SerializePlatform filePlatform;
	NX_ERR_CHECK_WARN_RETURN( hdr.getPlatform(filePlatform), "Unknown platform" );
	NX_BOOL_ERR_CHECK_WARN_RETURN(
		GetCurrentPlatform() == filePlatform,
		Serializer::ERROR_INVALID_PLATFORM,
		"Failed to inplace deserialize: platforms do not match" );

	//Fix pointers

	physx::PxU32 nrelocs;
	BinaryReloc *relocs = getRelocationTable(hdr, start, nrelocs);

	//Using refOffs-array causes a memory allocation hence it's bad.
	//But we have to fix all pointers prior to calling finishNxParameterized().
	//Basically we have 3 opportunities:
	//	- reorder relocs s.t. NxParameterized-objects go after all other relocs (strings, etc.)
	//		(but this limits our binary file format + it turns out to be tricky to implement)
	//	- walk through array once again
	//		(but this may have a significant time penalty)
	//	- reuse space in relocation table to store offsets of references
	//		(this sounds nice and easy enough!)
	//	- or maybe just process relocations in reverse order
	//		(this one should be the fastest but once again this limits our format)
	//For now we just keep the dynamic array as it seems to be pretty fast.

	physx::Array<physx::PxU32, Traits::Allocator> refOffs((Traits::Allocator(mTraits)));
	refOffs.reserve(nrelocs / 10);

	for(physx::PxU32 i = 0; i < nrelocs; ++i)
	{
		BinaryReloc &reloc = relocs[i];

		NX_BOOL_ERR_CHECK_RETURN(
			reloc.type < static_cast<physx::PxU32>(RELOC_LAST),
			Serializer::ERROR_INVALID_RELOC_TYPE );

		switch( reloc.type )
		{
		case physx::PxU32(RELOC_ABS_RAW):
			//Raw data, do nothing
			break;
		case physx::PxU32(RELOC_ABS_REF):
			//Reference, init later
			refOffs.pushBack(reloc.off);
			break;
		default:
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_INTERNAL_PTR;
		}

		char *&ptr = *reinterpret_cast<char **>(start + reloc.off);

		size_t off = reinterpret_cast<size_t>(ptr); //size_t == uintptr_t
		NX_BOOL_ERR_CHECK_RETURN( off < dataLen, Serializer::ERROR_INVALID_INTERNAL_PTR );

		ptr = start + off;
	}

	physx::PxI32 *refCount = const_cast<physx::PxI32 *>(&hdr.numObjects);

	//Init references (vtables and stuff)
	for(physx::PxU32 i = 0; i < refOffs.size(); ++i)
	{
		char *&p = *reinterpret_cast<char **>(start + refOffs[i]);

		const ObjHeader &objHdr = *reinterpret_cast<const ObjHeader *>(p);

		const char *className = objHdr.className;
		NX_BOOL_ERR_CHECK_RETURN( className, Serializer::ERROR_INVALID_INTERNAL_PTR );

		p += objHdr.dataOffset;

		if( !objHdr.isIncluded )
		{
			Interface *obj = PX_PLACEMENT_NEW(p, NxParameters)(mTraits, start, refCount);
			PX_UNUSED(obj);

			//We do not want objHdr.verify() here
		}
		else
		{
			Interface *obj = mTraits->finishNxParameterized(className, objHdr.version, p, start, refCount);
			NX_BOOL_ERR_CHECK_RETURN( obj, Serializer::ERROR_OBJECT_CREATION_FAILED );
			NX_ERR_CHECK_RETURN( verifyObjectHeader(objHdr, obj, mTraits) );

			if( objHdr.version != mTraits->getCurrentVersion(className) )
				doesNeedUpdate = true;
		} //if( !objHdr.isIncluded )
	} //i

	// Table of root references
	ObjectTableEntry *objectTable = reinterpret_cast<ObjectTableEntry *>(start + hdr.dataOffset); //At start of data section

	//Init results
	res.init(mTraits, (physx::PxU32)hdr.numObjects);
	for(physx::PxU32 i = 0; i < (physx::PxU32)hdr.numObjects; ++i)
		res[i] = objectTable[i].obj;

	*refCount = (physx::PxI32)refOffs.size(); //Release after all inplace references are destroyed

	// Conversion will be done after deserialization to avoid releasing inplace memory if all objects are destroyed
	//(otherwise we may have data corruption)

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::updateAllCounted2WithAlignment(BinaryHeader &hdr, char * /*start*/)
{
	hdr.version = BinVersions::WithAlignment;
	hdr.alignment = 8;
	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::updateInitial2AllCounted(BinaryHeader &hdr, char *start)
{
	// versionAllRefsCounted counts all references instead of just top-level references;
	// to achieve this we simply set mDoDeallocateSelf to true in all references

	physx::PxU32 nrelocs;
	BinaryReloc *relocs = getRelocationTable(hdr, start, nrelocs);

	for(physx::PxU32 i = 0; i < nrelocs; ++i)
	{
		BinaryReloc &reloc = relocs[i];

		if( reloc.type != static_cast<physx::PxU32>(RELOC_ABS_REF) )
			continue;

		size_t targetOff = *reinterpret_cast<size_t *>(start + reloc.off); //size_t == uintptr_t

		const ObjHeader &objHdr = *reinterpret_cast<const ObjHeader *>(start + targetOff);

		targetOff += objHdr.dataOffset; //Points to start of NxParameters

		// MyNxParameters has the same layout as NxParameters
		class MyNxParameters
		{
		public:
			// These methods are public to suppress compiler warnings about unused private members
			void *vtable;
			void *mParameterizedTraits;
			void *mSerializationCb;
			void *mCbUserData;
			void *mBuffer;
			void *mRefCount;
			void *mName;
			void *mClassName;
			bool mDoDeallocateSelf;
			bool mDoDeallocateName;
			bool mDoDeallocateClassName;

		public:
			void setDoDeallocateSelf() { mDoDeallocateSelf = true; }
		};

		MyNxParameters *pAsNxParameters = reinterpret_cast<MyNxParameters *>(start + targetOff);
		pAsNxParameters->setDoDeallocateSelf();
	}

	hdr.version = BinVersions::AllRefsCounted;

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::readArraySlow(Handle &handle, PlatformInputStream &s)
{
	physx::PxI32 n;
	NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(n), Serializer::ERROR_INVALID_ARRAY );

	for(physx::PxI32 i = 0; i < n; ++i)
	{
		handle.set(i);
		NX_ERR_CHECK_RETURN( readBinaryData(handle, s) );
		handle.popIndex();
	}

	return Serializer::ERROR_NONE;
}

static bool IsSimpleStruct(const Definition *pd)
{
	// Empty structs are special
	if( 0 == pd->numChildren() )
		return false;

	for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
	{
		switch( pd->child(i)->type() )
		{
		case TYPE_STRING:
		case TYPE_ENUM:
		case TYPE_ARRAY:
		case TYPE_REF:
		case TYPE_STRUCT:
			return false;
		default:
			break;
		}
	}

	return true;
}

#ifndef WITHOUT_APEX_SERIALIZATION

Serializer::ErrorType BinSerializer::internalSerialize(physx::PxFileBuf &stream, const Interface **objs, physx::PxU32 nobjs, bool doMetadata)
{
	// This is not yet working (this is not trivial and needs extensive testing!)
	NX_BOOL_ERR_CHECK_RETURN( SerializePlatform::OS_MACOSX != mPlatform.osType, Serializer::ERROR_NOT_IMPLEMENTED );

	NX_BOOL_ERR_CHECK_WARN_RETURN(
		PlatformABI::VerifyCurrentPlatform(),
		Serializer::ERROR_INVALID_PLATFORM ,
		"Current platform is not supported, aborting" );

	NX_BOOL_ERR_CHECK_RETURN( BinaryHeader::CheckAlignment(), Serializer::ERROR_INVALID_PLATFORM );

	PlatformABI targetParams;
	NX_ERR_CHECK_WARN_RETURN( PlatformABI::GetPredefinedABI(mPlatform, targetParams), "Unknown platform" );
	NX_BOOL_ERR_CHECK_WARN_RETURN(
		targetParams.isNormal(),
		Serializer::ERROR_INVALID_PLATFORM,
		"Target platform is not supported, aborting" );

	PlatformABI curParams;
	NX_ERR_CHECK_WARN_RETURN( PlatformABI::GetPredefinedABI(GetCurrentPlatform(), curParams), "Unknown platform" );
	NX_BOOL_ERR_CHECK_RETURN( curParams.isNormal(), Serializer::ERROR_INVALID_PLATFORM );

	Dictionary dict(mTraits); //Dictionary is updated in other routines

	// Compute object data

	typedef physx::Array<PlatformOutputStream, Traits::Allocator> StreamArray;
	StreamArray dataStreams((Traits::Allocator(mTraits)));
	dataStreams.reserve(nobjs);
	for(physx::PxU32 i = 0; i < nobjs; ++i)
	{
		Handle handle(*objs[i], "");
		NX_BOOL_ERR_CHECK_RETURN( handle.isValid(), Serializer::ERROR_UNKNOWN );

		dataStreams.pushBack(PlatformOutputStream(targetParams, mTraits, dict));
		dataStreams.back().setAlignment(16); // 16 for safety

		NX_ERR_CHECK_WARN_RETURN( storeBinaryData(*objs[i], handle, dataStreams.back()), "Failed to serialize data for object %d", i );
	}

	// Compute metadata

	PlatformOutputStream metadataStream(targetParams, mTraits, dict);
	metadataStream.setAlignment(16); // 16 for safety
	physx::PxU32 numMetadata = 0;
	if( doMetadata )
	{
		typedef physx::Pair<const char *, physx::PxU32> VersionedClass;
		typedef physx::HashMap<VersionedClass, const Definition *, physx::Hash<VersionedClass>, Traits::Allocator> MapClass2Def;
		MapClass2Def mapClass2Def((Traits::Allocator(mTraits)));

		//We track all classes that were saved to avoid duplicate metadata info
		class MyRefCallback: public ReferenceInterface
		{
			MapClass2Def &mapClass2Def;

			//Silence warnings on unable to generate assignment operator
			void operator =(MyRefCallback) {}
		public:
			MyRefCallback(MapClass2Def &mapClass2Def_): mapClass2Def(mapClass2Def_) {}

			void referenceCallback(Handle &handle)
			{
				Interface *iface;
				handle.getParamRef(iface);
				if( iface )
					mapClass2Def[ VersionedClass(iface->className(), iface->version()) ] = iface->rootParameterDefinition();
			}
		} cb(mapClass2Def);

		for(physx::PxU32 i = 0; i < nobjs; ++i)
		{
			if( !objs[i] )
				continue;

			mapClass2Def[ VersionedClass(objs[i]->className(), objs[i]->version()) ] = objs[i]->rootParameterDefinition();
			getReferences(*objs[i], cb, false, true, true);
		}

		numMetadata = mapClass2Def.size();

		for(MapClass2Def::Iterator i = mapClass2Def.getIterator(); !i.done(); ++i)
		{
			metadataStream.beginStruct(targetParams.getMetaEntryAlignment());

			metadataStream.storeStringPtr(i->first.first); //className
			metadataStream.storeSimple<physx::PxU32>(i->first.second); //classVersion

			//info
			Reloc &reloc = metadataStream.storePtr(RELOC_ABS_RAW, targetParams.getMetaInfoAlignment());
			NX_ERR_CHECK_RETURN( storeMetadataInfo(i->second, *reloc.ptrData) );

			metadataStream.closeStruct();
		} //i
	}

	// Init header

	BinaryHeader hdr;

	hdr.magic = Magic;
	hdr.type = BINARY_TYPE_PLAIN;
	hdr.version = Version;
	hdr.numObjects = (physx::PxI32)nobjs;
	hdr.fileLength = 0;
	hdr.dictOffset = 0;
	hdr.dataOffset = 0;
	hdr.relocOffset = 0;
	hdr.metadataOffset = 0;

	hdr.archType = static_cast<physx::PxU32>(mPlatform.archType);
	hdr.compilerType = static_cast<physx::PxU32>(mPlatform.compilerType);
	hdr.compilerVer = mPlatform.compilerVer;
	hdr.osType = static_cast<physx::PxU32>(mPlatform.osType);
	hdr.osVer = mPlatform.osVer;

	hdr.numMetadata = numMetadata;
	hdr.alignment = 0;

	BinaryHeaderExt ext;
	memset(&ext, 0, sizeof(ext));
	memcpy(&ext.vcsSafetyFlags, VCS_SAFETY_FLAGS, 4);

	// Resulting stream

	PlatformOutputStream joinStream(targetParams, mTraits, dict);

	// Merge header

	joinStream.storeBytes(reinterpret_cast<const char *>(&hdr), sizeof(hdr));
	joinStream.storeBytes(reinterpret_cast<const char *>(&ext), sizeof(ext));
	joinStream.align(16);

	// Merge dictionary

	physx::PxU32 dictOffset = joinStream.size();
	joinStream.mergeDict();

	// Merge data

	physx::PxU32 dataOffset = 0;
	{
		joinStream.align(joinStream.getTargetABI().aligns.pointer);
		dataOffset = joinStream.size();

		//Create object table
		for(physx::PxU32 i = 0; i < nobjs; ++i)
		{
			dataStreams[i].flatten();

			Reloc &reloc = joinStream.storePtr(RELOC_ABS_REF, dataStreams[i].alignment());
			reloc.ptrData->merge(dataStreams[i]);

			//This was requested for faster peeking of contained assets
			joinStream.storeStringPtr(objs[i]->className());
			joinStream.storeStringPtr(objs[i]->name());

			//UE3 was once using this
			joinStream.storeNullPtr();
		}

		joinStream.flatten(); //Merge all data-related pointers
	}

	// Merge metadata

	physx::PxU32 metadataOffset = 0;
	if( doMetadata )
	{
		metadataOffset = joinStream.merge(metadataStream);
		joinStream.flatten(); //Merge all metadata-related pointers
	}

	// Merge relocation data

	physx::PxU32 relocOffset = joinStream.writeRelocs();

	// Update offsets in header

	void *data = joinStream.getData();

	BinaryHeader &hdrFinal = *reinterpret_cast<BinaryHeader *>(data);
	hdrFinal.fileLength = joinStream.size();
	hdrFinal.dictOffset = dictOffset;
	hdrFinal.dataOffset = dataOffset;
	hdrFinal.metadataOffset = metadataOffset;
	hdrFinal.relocOffset = relocOffset;
	hdrFinal.alignment = joinStream.alignment();

	BinaryHeaderExt &extFinal = *reinterpret_cast<BinaryHeaderExt *>((char *)data + sizeof(BinaryHeader));

	//Make platform-independent
	hdrFinal.canonize();
	extFinal.canonize();

	// Write data to stream

	if( stream.tellWrite() % joinStream.alignment() )
	{
		NX_PARAM_TRAITS_WARNING(
			mTraits,
			"Input stream is unaligned, "
				"you will not be able to inplace deserialize it "
				"without copying to intermediate buffer"
		);
	}

	NX_BOOL_ERR_CHECK_RETURN(
		joinStream.size() == stream.write(data, joinStream.size()),
		Serializer::ERROR_STREAM_ERROR );

	return Serializer::ERROR_NONE;
}

Serializer::ErrorType BinSerializer::storeMetadataInfo(const Definition *def, PlatformOutputStream &s)
{
	s.beginStruct(s.getTargetABI().getMetaInfoAlignment());

	//type
	s.storeSimple((physx::PxU32)def->type());

	//arraySize
	s.storeSimple(TYPE_ARRAY == def->type() ? (physx::PxI32)def->arraySize() : -1);

	//shortName
	s.storeStringPtr(def->name());

	//structName
	s.storeStringPtr(def->structName());

	//alignment
	s.storeSimple(def->alignment());

	//padding
	s.storeSimple(def->padding());

	//numChildren
	s.storeSimple(def->numChildren());

	//children
	for(physx::PxI32 i = 0; i < def->numChildren(); ++i)
	{
		Reloc &childrenReloc = s.storePtr(RELOC_ABS_RAW, s.getTargetABI().getMetaInfoAlignment());
		NX_ERR_CHECK_RETURN( storeMetadataInfo(def->child(i), *childrenReloc.ptrData) );
	}

	//numHints
	s.storeSimple(def->numHints());

	//hints
	for(physx::PxI32 i = 0; i < def->numHints(); ++i)
	{
		const Hint *hint = def->hint(i);

		s.beginStruct(s.getTargetABI().getHintAlignment());

		s.storeSimple((physx::PxU32)hint->type());
		s.storeStringPtr(hint->name());

		s.beginStruct(s.getTargetABI().getHintValueAlignment());
		switch( hint->type() )
		{
		case TYPE_U64:
			s.storeSimple(hint->asUInt());
			break;
		case TYPE_F64:
			s.storeSimple(hint->asFloat());
			break;
		case TYPE_STRING:
			s.storeStringPtr(hint->asString());
			break;
		default:
			DEBUG_ALWAYS_ASSERT();
			return Serializer::ERROR_INVALID_DATA_TYPE;
		}
		s.align(s.getTargetABI().getHintValueSize());
		s.closeStruct();

		s.closeStruct();
	}

	//numEnumVals
	s.storeSimple(def->numEnumVals());

	//enumVals
	for(physx::PxI32 i = 0; i < def->numEnumVals(); ++i)
		s.storeStringPtr(def->enumVal(i));

	//numRefVariantVals
	s.storeSimple(def->numRefVariants());

	//refVariants
	for(physx::PxI32 i = 0; i < def->numRefVariants(); ++i)
		s.storeStringPtr(def->refVariantVal(i));

	s.closeStruct();

	return Serializer::ERROR_NONE;
}

//Store array of arbitrary type (slow version)
Serializer::ErrorType BinSerializer::storeArraySlow(Handle &handle, PlatformOutputStream &s)
{
	physx::PxI32 size;
	NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(size), Serializer::ERROR_INVALID_ARRAY );

	const Interface &obj = *handle.getConstInterface();
	for(physx::PxI32 i = 0; i < size; ++i)
	{
		handle.set(i);
		NX_ERR_CHECK_RETURN( storeBinaryData(obj, handle, s) );
		handle.popIndex();
	}

	return Serializer::ERROR_NONE;
}

//Print binary data for part of NxParameterized object addressed by handle
Serializer::ErrorType BinSerializer::storeBinaryData(const Interface &obj, Handle &handle, PlatformOutputStream &res, bool isRootObject)
{
	bool isObject = !handle.numIndexes();

	if( isObject )
		res.storeObjHeader(obj);

	const Definition *pd = handle.parameterDefinition();

	bool doNotSerialize = 0 != pd->hint("DONOTSERIALIZE"),
		isDynamicArray = pd->type() == TYPE_ARRAY && !pd->arraySizeIsFixed();

	// Alignment in dynamic array means alignment of dynamic memory
	// so we do not insert it here
	if( pd->alignment() && !isDynamicArray )
		res.align(pd->alignment());

	//Dynamic arrays are special because we need some of their fields for correct inplace deserialization
	if( doNotSerialize && !isDynamicArray )
	{
		//Simply skip bytes
		res.align(res.getTargetAlignment(pd));
		res.skipBytes(res.getTargetSize(pd));

		return Serializer::ERROR_NONE;
	}

	switch( pd->type() )
	{
	case TYPE_ARRAY:
		{
			if( pd->arraySizeIsFixed() )
			{
				res.beginArray(pd);
				storeArraySlow(handle, res);
				res.closeArray();
			}
			else
			{
				res.beginStruct(pd); //Dynamic arrays are implemented as structs

				bool isEmpty = false;
				if( doNotSerialize )
					isEmpty = true;
				else
				{
					for(physx::PxI32 i = 0; i < pd->arrayDimension(); ++i)
					{
						physx::PxI32 size = 0;
						NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(size, i), Serializer::ERROR_INVALID_ARRAY );

						if( 0 == size )
						{
							isEmpty = true;
							break;
						}
					}
				}

				const Definition *elemPd = pd;
				for(physx::PxI32 i = 0; i < pd->arrayDimension(); ++i)
					elemPd = elemPd->child(0);

				//Pointer-to-data
				Reloc *reloc = 0;
				if( isEmpty )
					res.storeNullPtr();
				else
				{
					reloc = &res.storePtr(RELOC_ABS_RAW, elemPd);
					if( pd->alignment() )
						reloc->ptrData->setAlignment(physx::PxMax(reloc->ptrData->alignment(), pd->alignment()));
				}

				//isAllocated
				res.storeSimple(false);

				//elementSize
				res.storeSimple<physx::PxI32>((physx::PxI32)res.getTargetSize(elemPd));

				//arraySizes
				const Definition *subArrayPd = pd;
				res.beginArray(res.getTargetABI().getAlignment<physx::PxI32>());
				for(physx::PxI32 dim = 0; dim < pd->arrayDimension(); ++dim)
				{
					// Store size of static dimensions even if doNotSerialize is on
					physx::PxI32 size = 0;
					if( !doNotSerialize || subArrayPd->arraySizeIsFixed() )
						NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(size, dim), Serializer::ERROR_INVALID_ARRAY );

					res.storeSimple(size);

					NX_BOOL_ERR_CHECK_RETURN( 1 == subArrayPd->numChildren(), Serializer::ERROR_UNKNOWN );
					subArrayPd = subArrayPd->child(0);
				}
				res.closeArray();

				//Data
				if( !isEmpty )
				{
					reloc->ptrData->beginArray(elemPd);

					physx::PxI32 size = -1;
					NX_PARAM_ERR_CHECK_RETURN( handle.getArraySize(size), Serializer::ERROR_INVALID_ARRAY );

					const Definition *child = pd->child(0);
					switch( child->type() )
					{
#					define NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#					define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#					define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#					define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
					case TYPE_##enum_name: \
					{ \
						reloc->ptrData->storeSimpleArray<c_type>(handle); \
						break; \
					}
#					include "NxParameterized_types.h"

					case TYPE_STRUCT:
						if( IsSimpleStruct(child) )
						{
							//Fast path for simple structs
							reloc->ptrData->storeSimpleStructArray(handle);
							break;
						}

						//Else fall through to default

					default:
						storeArraySlow(handle, *reloc->ptrData);
						break;
					}

					reloc->ptrData->closeArray();
				}

				res.closeStruct();
			}
			break;
		}

	case TYPE_STRUCT:
		{
			physx::PxU32 oldSize = isObject ? res.beginObject(obj, isRootObject, pd) : res.beginStruct(pd);

			for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
			{
				handle.set(i);
				NX_ERR_CHECK_RETURN( storeBinaryData(obj, handle, res) );
				handle.popIndex();
			}

			//Empty structs are at least 1 char long
			if( res.size() == oldSize )
				res.storeSimple((physx::PxU8)0);

			if( isObject )
				res.closeObject();
			else
				res.closeStruct();

			break;
		}

	case TYPE_STRING:
		{
			const char *s = 0;
			handle.getParamString(s);

			res.beginString();

			//buf
			res.storeStringPtr(s);

			//isAllocated
			res.storeSimple(false);

			res.closeString();

			break;
		}

	case TYPE_ENUM:
		{
			const char *s = 0;
			handle.getParamEnum(s);

			res.storeStringPtr(s);

			break;
		}

	case TYPE_REF:
		{
			Interface *refObj = 0;
			handle.getParamRef(refObj);

			if( !refObj )
				res.storeNullPtr();
			else if( pd->isIncludedRef() )
			{
				Handle refHandle(*refObj, "");
				NX_BOOL_ERR_CHECK_RETURN( refHandle.isValid(), Serializer::ERROR_UNKNOWN );

				Reloc &reloc = res.storePtr(RELOC_ABS_REF, 16); //16 for safety
				NX_ERR_CHECK_RETURN( storeBinaryData(*refObj, refHandle, *reloc.ptrData, false) );
			}
			else //Named reference
			{
				Reloc &reloc = res.storePtr(RELOC_ABS_REF, 16); //16 for safety
				reloc.ptrData->storeObjHeader(*refObj, false);
				reloc.ptrData->beginObject(*refObj, false, 0);
				reloc.ptrData->closeObject(); //Named references are instances of NxParameters => no additional fields
			}

			break;
		}

	case TYPE_POINTER:
		{
			res.storeNullPtr();
			break;
		}

	case TYPE_MAT34:
		{
			physx::PxMat44 val;
			NX_PARAM_ERR_CHECK_RETURN( handle.getParamMat34(val), Serializer::ERROR_INVALID_VALUE );
			res.storeSimple(physx::PxMat34Legacy(val));
			break;
		}

#	define NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#	define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#	define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#	define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
	case TYPE_##enum_name: \
	{ \
		c_type val; \
		NX_PARAM_ERR_CHECK_RETURN( handle.getParam##type_name(val), Serializer::ERROR_INVALID_VALUE ); \
		res.storeSimple<c_type>(val); \
		break; \
	}
#	include "NxParameterized_types.h"

	default:
		DEBUG_ASSERT(0 && "Unknown type");
		return Serializer::ERROR_INVALID_DATA_TYPE;
	}

	return Serializer::ERROR_NONE;
}

#endif

//Read NxParameterized object data
Serializer::ErrorType BinSerializer::readObject(Interface *&obj, PlatformInputStream &data)
{
	ObjHeader objHdr;
	NX_ERR_CHECK_WARN_RETURN( data.readObjHeader(objHdr), "Failed to deserialize object header" );
	Releaser releaseClassName((void *)objHdr.className, mTraits),
		releaseName((void *)objHdr.name, mTraits),
		releaseChecksum((void *)objHdr.checksum, mTraits);

	if( !objHdr.isIncluded )
	{
		void *buf = mTraits->alloc(sizeof(NxParameters));
		NX_BOOL_ERR_CHECK_RETURN( buf, Serializer::ERROR_MEMORY_ALLOCATION_FAILURE );

		obj = PX_PLACEMENT_NEW(buf, NxParameters)(mTraits);
		obj->setClassName(objHdr.className);

		if( objHdr.name )
			obj->setName(objHdr.name);

		//We do not want objHdr.verify() here
	}
	else
	{
		NX_BOOL_ERR_CHECK_RETURN( objHdr.className, Serializer::ERROR_OBJECT_CREATION_FAILED );

		obj = mTraits->createNxParameterized(objHdr.className, objHdr.version);
		NX_BOOL_ERR_CHECK_RETURN( obj, Serializer::ERROR_OBJECT_CREATION_FAILED );

		if( objHdr.name )
			obj->setName(objHdr.name);

		NX_ERR_CHECK_RETURN( verifyObjectHeader(objHdr, obj, mTraits) );

		Handle handle(*obj, "");
		NX_BOOL_ERR_CHECK_RETURN( handle.isValid(), Serializer::ERROR_UNKNOWN );

		NX_ERR_CHECK_RETURN( data.pushPos(objHdr.dataOffset) );
		NX_ERR_CHECK_RETURN( readBinaryData(handle, data) );
		data.popPos();
	}

	return Serializer::ERROR_NONE;
}

//Read binary data of NxParameterized object addressed by handle
Serializer::ErrorType BinSerializer::readBinaryData(Handle &handle, PlatformInputStream &data)
{
	const Definition *pd = handle.parameterDefinition();

	bool isDynamicArray = TYPE_ARRAY == pd->type() && !pd->arraySizeIsFixed();

	// See comment in storeBinaryData
	if( pd->alignment() && !isDynamicArray )
		data.align(pd->alignment());

	if( pd->hint("DONOTSERIALIZE") )
	{
		//Simply skip DONOTSERIALIZE
		data.align(data.getTargetAlignment(pd));
		NX_ERR_CHECK_RETURN( data.skipBytes(data.getTargetSize(pd)) );
		return Serializer::ERROR_NONE;
	}

	switch( pd->type() )
	{
	case TYPE_ARRAY:
		{
			if( pd->arraySizeIsFixed() )
			{
				data.beginArray(pd);
				for(physx::PxI32 i = 0; i < pd->arraySize(); ++i)
				{
					handle.set(i);
					NX_ERR_CHECK_RETURN( readBinaryData(handle, data) );
					handle.popIndex();
				}
				data.closeArray();
			}
			else
			{
				data.beginStruct(pd); //Dynamic arrays are implemented as structs

				physx::PxU32 elemOff;
				NX_ERR_CHECK_RETURN( data.readPtr(elemOff) );

				bool isAllocated;
				NX_ERR_CHECK_RETURN( data.read(isAllocated) );

				physx::PxI32 elementSize;
				NX_ERR_CHECK_RETURN( data.read(elementSize) ); //elementSize

				data.beginStruct(data.getTargetABI().aligns.i32); //Start array of sizes

				physx::PxI32 arraySize;
				NX_ERR_CHECK_RETURN( data.read(arraySize) ); //We need only the first size

				const Definition *subArrayPd = pd->child(0);
				for(physx::PxI32 dim = 1; dim < pd->arrayDimension(); ++dim)
				{
					physx::PxI32 size;
					NX_ERR_CHECK_RETURN( data.read(size) );

					// We do not support nested dynamic arrays
					if( subArrayPd->arraySizeIsFixed() )
						NX_BOOL_ERR_CHECK_RETURN( size == subArrayPd->arraySize(), Serializer::ERROR_INVALID_ARRAY );

					subArrayPd = subArrayPd->child(0);
				}
				data.closeStruct(); //End array of sizes

				NX_PARAM_ERR_CHECK_RETURN( handle.resizeArray(arraySize), Serializer::ERROR_INVALID_ARRAY );

				if( elemOff )
				{
					// Check alignment
					physx::PxU32 align = pd->alignment();
					if( align && elemOff % align ) {
						char longName[256];
						handle.getLongName(longName, sizeof(longName));
						NX_PARAM_TRAITS_WARNING(
							mTraits,
							"%s: array is unaligned, "
								"you will not be able to inplace deserialize it "
								"without copying to intermediate buffer.",
							longName
						);
					}

					NX_ERR_CHECK_RETURN( data.pushPos(elemOff) );

					switch( pd->child(0)->type() )
					{
#					define NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#					define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#					define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#					define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
					case TYPE_##enum_name: \
					{ \
						NX_ERR_CHECK_RETURN( data.readSimpleArray<c_type>(handle) ); \
						break; \
					}
#					include "NxParameterized_types.h"

					case TYPE_STRUCT:
						{
							const Definition *child = pd->child(0);
							if( IsSimpleStruct(child) )
							{
								//Fast path for simple structs
								NX_ERR_CHECK_RETURN( data.readSimpleStructArray(handle) );
								break;
							}

							//Else fall through to default
						}

					default:
						NX_ERR_CHECK_RETURN( readArraySlow(handle, data) );
						break;
					}

					data.popPos();
				}

				data.closeStruct();
			}
			break;
		}

	case TYPE_STRUCT:
		{
			physx::PxU32 oldPos = data.getPos();

			data.beginStruct(pd);
			for(physx::PxI32 i = 0; i < pd->numChildren(); ++i)
			{
				handle.set(i);
				NX_ERR_CHECK_RETURN( readBinaryData(handle, data) );
				handle.popIndex();
			}

			//Empty structs are at least 1 char long
			if( data.getPos() == oldPos )
			{
				physx::PxU8 tmp;
				NX_ERR_CHECK_RETURN( data.read(tmp) );
			}

			data.closeStruct();

			break;
		}

	case TYPE_STRING:
		{
			data.beginString();

			physx::PxU32 off;
			NX_ERR_CHECK_RETURN( data.readPtr(off) );

			const char *s;
			NX_ERR_CHECK_RETURN( data.readString(off, s) );

			NX_PARAM_ERR_CHECK_RETURN( handle.setParamString(s), Serializer::ERROR_INVALID_VALUE );
			mTraits->free((char *)s);

			bool isAllocated;
			NX_ERR_CHECK_RETURN( data.read(isAllocated) );

			data.closeString();

			break;
		}

	case TYPE_ENUM:
		{
			physx::PxU32 off;
			NX_ERR_CHECK_RETURN( data.readPtr(off) );

			const char *s;
			NX_ERR_CHECK_RETURN( data.readString(off, s) );

			NX_BOOL_ERR_CHECK_RETURN( s, Serializer::ERROR_INVALID_VALUE );
			NX_PARAM_ERR_CHECK_RETURN( handle.setParamEnum(s), Serializer::ERROR_INVALID_VALUE );

			mTraits->free((char *)s);

			break;
		}

	case TYPE_REF:
		{
			physx::PxU32 objPos;
			NX_ERR_CHECK_RETURN( data.readPtr(objPos) );

			Interface *refObj = 0;

			if( objPos )
			{
				NX_ERR_CHECK_RETURN( data.pushPos(objPos) );
				NX_ERR_CHECK_RETURN( readObject(refObj, data) );
				data.popPos();
			}

			if( refObj && (-1 == handle.parameterDefinition()->refVariantValIndex(refObj->className())) )
			{
				char longName[256];
				handle.getLongName(longName, sizeof(longName));
				NX_PARAM_TRAITS_WARNING(
					mTraits,
					"%s: setting reference of invalid class %s",
					longName,
					refObj->className()
				);
			}

			NX_PARAM_ERR_CHECK_RETURN( handle.setParamRef(refObj), Serializer::ERROR_INVALID_REFERENCE );

			break;
		}

	case TYPE_POINTER:
		{
			void *tmp;
			NX_ERR_CHECK_RETURN( data.read(tmp) );
			break;
		}

	case TYPE_MAT34:
		{
			physx::PxMat34Legacy val;
			NX_ERR_CHECK_RETURN(data.read(val));
			NX_PARAM_ERR_CHECK_RETURN( handle.setParamMat34((physx::PxMat44)val), Serializer::ERROR_INVALID_VALUE );
			break;
		}

#	define NX_PARAMETERIZED_TYPES_NO_LEGACY_TYPES
#	define NX_PARAMETERIZED_TYPES_ONLY_SIMPLE_TYPES
#	define NX_PARAMETERIZED_TYPES_NO_STRING_TYPES
#	define NX_PARAMETERIZED_TYPE(type_name, enum_name, c_type) \
	case TYPE_##enum_name: \
	{ \
		c_type val; \
		NX_ERR_CHECK_RETURN(data.read(val)); \
		NX_PARAM_ERR_CHECK_RETURN( handle.setParam##type_name(val), Serializer::ERROR_INVALID_VALUE ); \
		break; \
	}
#	include "NxParameterized_types.h"

	default:
		DEBUG_ASSERT(0 && "Unknown type");
		return Serializer::ERROR_INVALID_DATA_TYPE;
	}

	return Serializer::ERROR_NONE;
}

bool isBinaryFormat(physx::PxFileBuf &stream)
{
	physx::PxU32 magic;
	stream.peek(&magic, 4);
	return Canonize(magic) == BinSerializer::Magic;
}

Serializer::ErrorType peekBinaryPlatform(physx::PxFileBuf &stream, SerializePlatform &platform)
{
	BinaryHeader hdr;
	BinaryHeaderExt ext;
	NX_ERR_CHECK_RETURN( peekFileHeader(stream, hdr, ext) );
	NX_ERR_CHECK_RETURN( hdr.getPlatform(platform) );
	return Serializer::ERROR_NONE;
}

} // namespace NxParameterized
