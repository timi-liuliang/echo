/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */



#ifndef PVD_OBJECT_MODEL_H
#define PVD_OBJECT_MODEL_H
#include "PsBasicTemplates.h"
#include "PvdObjectModelMetaData.h"

namespace physx { namespace debugger {

#if defined(PX_VC11) || defined(PX_VC12) || defined(PX_VC14)
#pragma warning(push)
#pragma warning(disable: 4435)	// 'class1' : Object layout under /vd2 will change due to virtual base 'class2'
#endif

    class PvdInputStream;
	class PvdOutputStream;

	struct InstanceDescription
	{
		NonNegativeInteger	mId;
		NonNegativeInteger	mClassId;
		void*				mInstPtr;
		bool				mAlive;

		InstanceDescription( NonNegativeInteger id, NonNegativeInteger classId, void* inst, bool alive )
			: mId( id )
			, mClassId( classId )
			, mInstPtr( inst )
			, mAlive( alive )
		{
		}
		InstanceDescription()
			: mInstPtr( NULL )
			, mAlive( false )
		{
		}
		operator void* () { PX_ASSERT( mAlive ); if ( mAlive ) return mInstPtr; return NULL; }
		operator NonNegativeInteger () { return mId; }
	};

	typedef physx::shdfnd::Pair<PxI32, PxI32> InstancePropertyPair;

	class PvdObjectModelBase
	{
	protected:
		virtual ~PvdObjectModelBase(){}
	public:
		virtual void addRef() = 0;
		virtual void release() = 0;
		virtual void* idToPtr( NonNegativeInteger instId ) const = 0;
		virtual NonNegativeInteger ptrToId( void* instPtr ) const = 0;
		virtual InstanceDescription idToDescriptor( NonNegativeInteger instId ) const = 0;
		virtual InstanceDescription ptrToDescriptor( void* instPtr ) const = 0;
		virtual Option<ClassDescription> getClassOf( void* instId ) const = 0;
		virtual const PvdObjectModelMetaData& getMetaData() const = 0;
	};

	class PvdObjectModelMutator : public virtual PvdObjectModelBase
	{
	protected:		
		virtual ~PvdObjectModelMutator(){}
	public:
		//if the instance is alive, this destroyes any arrays and sets the instance back to its initial state.
		virtual InstanceDescription createInstance( NonNegativeInteger clsId, NonNegativeInteger instId ) = 0;
		virtual InstanceDescription createInstance( NonNegativeInteger clsId ) = 0;
		//Instances that are pinned are not removed from the system, ever.
		//This means that createInstance, pinInstance, deleteInstance
		//can be called in this order and you can still call getClassOf, etc. on the instances.
		//The instances will never be removed from memory if they are pinned, so use at your
		//careful discretion.
		virtual void pinInstance( void* instId ) = 0;
		virtual void unPinInstance( void* instId ) = 0;
        //when doing capture, should update all events in a section at once, otherwis there possible prase data incompltely.
		virtual void recordCompletedInstances() = 0;
	
		virtual void destroyInstance( void* instId ) = 0;
		virtual PxI32 getNextInstanceHandleValue() const = 0;
		//reserve a set of instance handle values by getting the current, adding an amount to it
		//and setting the value.  You can never set the value lower than it already is, it only climbs.
		virtual void setNextInstanceHandleValue( PxI32 hdlValue ) = 0;
		//If incoming type is provided, then we may be able to marshal simple types
		//This works for arrays, it just completely replaces the entire array.
		//Because if this, it is an error of the property identifier
		virtual bool setPropertyValue( void* instId, NonNegativeInteger propId, const PxU8* data, PxU32 dataLen, NonNegativeInteger incomingType ) = 0;
		//Set a set of properties defined by a property message
		virtual bool setPropertyMessage( void* instId, NonNegativeInteger msgId, const PxU8* data, PxU32 dataLen ) = 0;
		//insert an element(s) into array index.  If index > numElements, element(s) is(are) appended.
		virtual bool insertArrayElement( void* instId, NonNegativeInteger propId, NonNegativeInteger index, const PxU8* data, PxU32 dataLen, NonNegativeInteger incomingType = -1 ) = 0;
		virtual bool removeArrayElement( void* instId, NonNegativeInteger propId, NonNegativeInteger index ) = 0;
		//Add this array element to end end if it doesn't already exist in the array.
		//The option is false if there was an error with the function call.
		//The integer has no value if nothing was added, else it tells you the index
		//where the item was added.  Comparison is done using memcmp.
		virtual Option<NonNegativeInteger> pushBackArrayElementIf( void* instId, NonNegativeInteger propId, const PxU8* data, PxU32 dataLen, NonNegativeInteger incomingType = -1 ) = 0;
		//Remove an array element if it exists in the array.
		//The option is false if there was an error with the function call.
		//the integer has no value if the item wasn't found, else it tells you the index where
		//the item resided.  Comparison is memcmp.
		virtual Option<NonNegativeInteger> removeArrayElementIf( void* instId, NonNegativeInteger propId, const PxU8* data, PxU32 dataLen, NonNegativeInteger incomingType = -1 ) = 0;
		virtual bool setArrayElementValue( void* instId, NonNegativeInteger propId, NonNegativeInteger propIdx, const PxU8* data, PxU32 dataLen, NonNegativeInteger incomingType ) = 0;
		
		virtual void originShift( void* instId, PxVec3 shift ) = 0;

		InstanceDescription createInstance( const NamespacedName& name )
		{
			return createInstance( getMetaData().findClass( name )->mClassId );
		}
		template<typename TDataType>
		bool setPropertyValue( void* instId, const char* propName, const TDataType* dtype, PxU32 count )
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return false; }
			const PropertyDescription& prop( descOpt );
			Option<ClassDescription> incomingCls( getMetaData().findClass( getPvdNamespacedNameForType<TDataType>() ) );
			if ( incomingCls.hasValue() )
				return setPropertyValue( instId, prop.mPropertyId, reinterpret_cast< const PxU8* >( dtype ), sizeof( *dtype ) * count, incomingCls.getValue().mClassId );
			return false;
		}

		//Simplest possible setPropertyValue
		template<typename TDataType>
		bool setPropertyValue( void* instId, const char* propName, const TDataType& dtype )
		{
			return setPropertyValue( instId, propName, &dtype, 1 );
		}

		template<typename TDataType>
		bool setPropertyMessage( void* instId, const TDataType& msg )
		{
			Option<PropertyMessageDescription> msgId = getMetaData().findPropertyMessage( getPvdNamespacedNameForType<TDataType>() );
			if ( msgId.hasValue() == false ) return false;
			return setPropertyMessage( instId, msgId.getValue().mMessageId, reinterpret_cast<const PxU8*>( &msg ), sizeof( msg ) );
		}
		template<typename TDataType>
		bool insertArrayElement( void* instId, const char* propName, PxI32 idx, const TDataType& dtype )
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return false; }
			const PropertyDescription& prop( descOpt );
			Option<ClassDescription> incomingCls( getMetaData().findClass( getPvdNamespacedNameForType<TDataType>() ) );
			if ( incomingCls.hasValue() )
			{
				return insertArrayElement( instId, prop.mPropertyId, idx, reinterpret_cast< const PxU8* >( &dtype ), sizeof( dtype ), incomingCls.getValue().mClassId );
			}
			return false;
		}

		bool removeArrayElement( void* instId, const char* propName, PxI32 idx )
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return false; }
			const PropertyDescription& prop( descOpt );
			return removeArrayElement( instId, prop.mPropertyId, idx );
		}
		template<typename TDataType>
		Option<NonNegativeInteger> pushBackArrayElementIf( void* instId, const char* pname, const TDataType& item )
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, pname ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return None(); }
			const PropertyDescription& prop( descOpt );
			Option<ClassDescription> incomingCls( getMetaData().findClass( getPvdNamespacedNameForType<TDataType>() ) );
			if ( incomingCls.hasValue() 
				&& ( incomingCls.getValue().mClassId == prop.mDatatype ) )
			{
				return pushBackArrayElementIf( instId, prop.mPropertyId, reinterpret_cast<const PxU8* >( &item ), sizeof( item ), incomingCls.getValue().mClassId );
			}
			return None();
		}
		template<typename TDataType>
		Option<NonNegativeInteger> removeArrayElementIf( void* instId, const char* propId, const TDataType& item )
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propId ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return None(); }
			const PropertyDescription& prop( descOpt );
			Option<ClassDescription> incomingCls( getMetaData().findClass( getPvdNamespacedNameForType<TDataType>() ) );
			if ( incomingCls.hasValue() 
				&& ( incomingCls.getValue().mClassId == prop.mDatatype ) )
			{
				return removeArrayElementIf( instId, prop.mPropertyId, reinterpret_cast<const PxU8* >( &item ), sizeof( item ), incomingCls.getValue().mClassId );
			}
			return None();
		}
		template<typename TDataType>
		bool setArrayElementValue( void* instId, const char* propName, NonNegativeInteger propIdx, TDataType& item )
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return false; }
			const PropertyDescription& prop( descOpt );
			Option<ClassDescription> incomingCls( getMetaData().findClass( getPvdNamespacedNameForType<TDataType>() ) );
			if ( incomingCls.hasValue() && ( incomingCls.getValue().mClassId == prop.mDatatype ) )
				return setArrayElementValue( instId, prop.mPropertyId, propIdx, reinterpret_cast<const PxU8*>( &item ), sizeof( item ), incomingCls.getValue().mClassId );
			PX_ASSERT( false ); return false;
		}

	};

	class PvdObjectModelReader : public virtual PvdObjectModelBase
	{
	protected:
		virtual ~PvdObjectModelReader(){}
	public:
		
		//Return the byte size of a possible nested property
		virtual PxU32 getPropertyByteSize( void* instId, NonNegativeInteger propId ) const = 0;
		PxU32 getPropertyByteSize( void* instId, String propName )
		{
			PxI32 propId = getMetaData().findProperty( getClassOf( instId )->mClassId, propName )->mPropertyId;
			return getPropertyByteSize( instId, propId );
		}
		//Return the value of a possible nested property
		virtual PxU32 getPropertyValue(void* instId, NonNegativeInteger propId, PxU8* outData, PxU32 outDataLen ) const = 0;
		//Get the actual raw database memory.  This is subject to change drastically if the object gets deleted.
		virtual DataRef<PxU8> getRawPropertyValue( void* instId, NonNegativeInteger propId ) const = 0;

		DataRef<PxU8> getRawPropertyValue( void* instId, const char* propName ) const
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return 0; }
			return getRawPropertyValue( instId, descOpt->mPropertyId );
		}

		template<typename TDataType>
		DataRef<TDataType> getTypedRawPropertyValue( void* instId, NonNegativeInteger propId ) const 
		{
			DataRef<PxU8> propVal = getRawPropertyValue( instId, propId );
			return DataRef<TDataType>( reinterpret_cast<const TDataType*>( propVal.begin() ), propVal.size() / sizeof( TDataType ) );
		}

		template<typename TDataType>
		DataRef<TDataType> getTypedRawPropertyValue( void* instId, const char* propName ) const
		{
			DataRef<PxU8> propVal = getRawPropertyValue( instId, propName );
			return DataRef<TDataType>( reinterpret_cast<const TDataType*>( propVal.begin() ), propVal.size() / sizeof( TDataType ) );
		}

		template<typename TDataType>
		PxU32 getPropertyValue( void* instId, const char* propName, TDataType* outBuffer, PxU32 outNumBufferItems ) const
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return 0; }
			const PropertyDescription& prop( descOpt );
			PxU32 desired = outNumBufferItems * sizeof( TDataType );
			return getPropertyValue( instId, prop.mPropertyId, reinterpret_cast< PxU8* >( outBuffer ), desired ) / sizeof( TDataType );
		}

		template<typename TDataType>
		Option<TDataType> getPropertyValue( void* instId, const char* propName ) const
		{
			TDataType retval;
			if ( getPropertyValue( instId, propName, &retval, 1 ) == 1 )
				return retval;
			return None();
		}
		
		//Get this one item out of the array
		//return array[idx]
		virtual PxU32 getPropertyValue(void* instId, NonNegativeInteger propId, int inArrayIndex, PxU8* outData, PxU32 outDataLen ) const = 0;
		//Get this sub element of one item out of the array
		//return array[idx].a
		virtual PxU32 getPropertyValue(void* instId, NonNegativeInteger propId, int inArrayIndex, int nestedProperty, PxU8* outData, PxU32 outDataLen ) const = 0;

		//Get a set of properties defined by a property message
		virtual bool getPropertyMessage( void* instId, NonNegativeInteger msgId, PxU8* data, PxU32 dataLen ) const = 0;

		template< typename TDataType>
		bool getPropertyMessage( void* instId, TDataType& msg )
		{
			Option<PropertyMessageDescription> msgId ( getMetaData().findPropertyMessage( getPvdNamespacedNameForType<TDataType>() ) );
			if ( msgId.hasValue() == false ) return false;
			return getPropertyMessage( instId, msgId.getValue().mMessageId, reinterpret_cast<PxU8*>( &msg ), sizeof( msg ) );
		}

		//clearing the array is performed with a set property value call with no data.
		virtual PxU32 getNbArrayElements( void* instId, NonNegativeInteger propId ) const = 0;
		PxU32 getNbArrayElements( void* instId, const char* propName ) const
		{
			ClassDescription cls( getClassOf( instId ) );
			Option<PropertyDescription> descOpt ( getMetaData().findProperty( cls.mClassId, propName ) );
			if ( !descOpt.hasValue() ) { PX_ASSERT( false ); return false; }
			const PropertyDescription& prop( descOpt );
			return getNbArrayElements( instId, prop.mPropertyId );
		}

		//Write this instance out.  Offset is set as the instances last write offset.
		//This offset is cleared if the object is changed.
		//If offset doesn't have a value, then the instance isn't changed.
		virtual void writeInstance( void* instId, PvdOutputStream& stream ) = 0;

		virtual PxU32 getNbInstances() const = 0;
		virtual PxU32 getInstances( InstanceDescription* outBuffer, PxU32 count, PxU32 startIndex = 0 ) const = 0;

		//Get the list of updated objects since the last time someone cleared the updated instance list.
		virtual PxU32 getNbUpdatedInstances() const = 0;
		virtual PxU32 getUpdatedInstances( InstanceDescription* outBuffer, PxU32 count, PxU32 startIndex = 0 ) = 0;
		//Must be called for instances to be released.  Only instances that aren't live nor are they updated
		//are valid.	
		virtual void clearUpdatedInstances() = 0;

	};

	class PvdObjectModel : public PvdObjectModelMutator, public PvdObjectModelReader
	{
	protected:
		virtual ~PvdObjectModel(){}
	public:

		virtual void destroyAllInstances() = 0;
		virtual bool setPropertyValueToDefault( void* instId, NonNegativeInteger propId ) = 0;
		//Read an instance data and put a copy of the data in the output stream.
		static bool readInstance( PvdInputStream& inStream, PvdOutputStream& outStream );
		virtual InstanceDescription readInstance( DataRef<const PxU8> writtenData ) = 0;
		//Set just this property from this serialized instance.
		//Expects the instance to be alive, just like setPropertyValue
		virtual bool readInstanceProperty( DataRef<const PxU8> writtenData, NonNegativeInteger propId ) = 0;

		virtual void recordCompletedInstances() = 0;
		
		//OriginShift seekback support
		virtual PxU32 getNbShifted() = 0;
		virtual void getShiftedPair( InstancePropertyPair* outData, PxU32 count ) = 0;
		virtual void clearShiftedPair() = 0;
		virtual void shiftObject( void* instId, NonNegativeInteger propId, PxVec3 shift ) = 0;
		static PvdObjectModel& create( PxAllocatorCallback& callback, PvdObjectModelMetaData& metaData, bool isCapture = false);
	};

#if defined(PX_VC11) || defined(PX_VC12)
#pragma warning(pop)
#endif
}}

#endif
