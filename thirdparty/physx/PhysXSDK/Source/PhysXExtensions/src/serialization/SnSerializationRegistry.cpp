/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "SnSerializationRegistry.h"
#include "PxPhysics.h"
#include "ExtSerialization.h"
#include "PxSerializer.h"
#include "foundation/PxString.h"
#include "CmCollection.h"
#include "PxArticulationLink.h"

using namespace physx;

namespace
{
	class CollectionSorter : public PxProcessPxBaseCallback
	{
		typedef  Ps::Pair<PxBase*, PxSerialObjectId> Object;

		class Element 
		{
		public:	
			Object			    object;
			Ps::Array<PxU32>	children;
			bool				isFinished;

			Element(PxBase* obj = NULL) : object(obj, PX_SERIAL_OBJECT_ID_INVALID), isFinished(false)	{}
		};

	public:
		CollectionSorter(Cm::Collection& collection, Sn::SerializationRegistry& sr, bool isRepx) : mCollection(collection), mSr(sr), mIsRepx(isRepx) {}
		virtual ~CollectionSorter(){}

		void process(PxBase& base)
		{
			addChild(&base);
			//ArticulationLink is not a repx serializer, so should require Articulation here
		    if( mIsRepx && PxConcreteType::eARTICULATION_LINK == base.getConcreteType() )
			{
				PxArticulationLink* link = static_cast<PxArticulationLink*>(&base);				
				PxBase* a = reinterpret_cast<PxBase*>(&link->getArticulation());
				if(mCurElement->object.first != a )  //don't require itself
					addChild(a);
			}
		}

		void sort()
		{		
			Element element;		
			PxU32 i;

			PxU32 nbObject = mCollection.internalGetNbObjects();
			const Cm::Collection::ObjectToIdMap::Entry* objectdatas = mCollection.internalGetObjects();
			for( i = 0; i < nbObject; ++i )
			{				
				element.object.first = objectdatas[i].first;
				element.object.second = objectdatas[i].second;
				mObjToIdMap.insert(objectdatas[i].first, mElements.size());
				mElements.pushBack(element);
			}

			for( i = 0; i < nbObject; ++i )
			{
				mCurElement = &mElements[i];
				const PxSerializer* serializer = mSr.getSerializer(mCurElement->object.first->getConcreteType());
				PX_ASSERT(serializer);
				serializer->requires(*mCurElement->object.first, *this);
			}  	

			for( i = 0; i < nbObject; ++i )
			{
				if( mElements[i].isFinished )
					continue;

				AddElement(mElements[i]);
			}

			mCollection.mObjects.clear();
			for(Ps::Array<Object>::Iterator o = mSorted.begin(); o != mSorted.end(); ++o )
			{				
				mCollection.internalAdd(o->first, o->second);
			}
		}

		void AddElement(Element& e)
		{
			if( !e.isFinished )
			{
				for( Ps::Array<PxU32>::Iterator child = e.children.begin(); child != e.children.end(); ++child )
				{
					AddElement(mElements[*child]);
				}				
				mSorted.pushBack(e.object);
				e.isFinished = true;
			}
		}

	private:
		PX_INLINE void addChild(PxBase* base)
		{
			PX_ASSERT(mCurElement);
			const Ps::HashMap<PxBase*, PxU32>::Entry* entry = mObjToIdMap.find(base);
			if(entry)					
				mCurElement->children.pushBack(entry->second);
		}

		CollectionSorter& operator=(const CollectionSorter&);
		Ps::HashMap<PxBase*, PxU32>	mObjToIdMap;
		Ps::Array<Element>			mElements;
		Cm::Collection&				mCollection;
		Sn::SerializationRegistry&  mSr;
		Ps::Array<Object>           mSorted;
		Element*                    mCurElement;
		bool                        mIsRepx;
	};
}

namespace physx { namespace Sn {

SerializationRegistry::SerializationRegistry(PxPhysics& physics)
	: mPhysics(physics)
{	
	PxRegisterPhysicsSerializers(*this);
	Ext::RegisterExtensionsSerializers(*this);

	registerBinaryMetaDataCallback(PxGetPhysicsBinaryMetaData);
	registerBinaryMetaDataCallback(Ext::GetExtensionsBinaryMetaData);
}

SerializationRegistry::~SerializationRegistry()
{
	PxUnregisterPhysicsSerializers(*this);
	Ext::UnregisterExtensionsSerializers(*this);

	if(mSerializers.size() > 0)
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::release(): some registered PxSerializer instances were not unregistered");	
	}

	if(mRepXSerializers.size() > 0)
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::release(): some registered PxRepXSerializer instances were not unregistered");	
	}
}

void SerializationRegistry::registerSerializer(PxType type, PxSerializer& serializer)
{
	if(mSerializers.find(type))
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::registerSerializer: Type %d has already been registered", type);		
	}

	mSerializers.insert(type, &serializer);	
}

PxSerializer* SerializationRegistry::unregisterSerializer(PxType type)
{
	const SerializerMap::Entry* e = mSerializers.find(type);
	PxSerializer* s = 	e ? e->second : NULL;

	if(!mSerializers.erase(type))
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::unregisterSerializer: failed to find PxSerializer instance for type %d", type);
	}
	return s;
}

const PxSerializer* SerializationRegistry::getSerializer(PxType type) const 
{
	const SerializerMap::Entry* e = mSerializers.find(type);
#if defined(PX_CHECKED)
	if (!e)
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::getSerializer: failed to find PxSerializer instance for type %d", type);
	}
#endif
	return e ? e->second : NULL;
}

PxType SerializationRegistry::getSerializerType(PxU32 index) const 
{ 
	PX_ASSERT(index < mSerializers.size());
	return mSerializers.getEntries()[index].first;
}

const char* SerializationRegistry::getSerializerName(PxU32 index) const 
{ 
	PX_ASSERT(index < mSerializers.size());
	return mSerializers.getEntries()[index].second->getConcreteTypeName();
}

void SerializationRegistry::registerBinaryMetaDataCallback(PxBinaryMetaDataCallback callback)
{
	mMetaDataCallbacks.pushBack(callback);
}

void SerializationRegistry::getBinaryMetaData(PxOutputStream& stream) const
{
	for(PxU32 i = 0; i < mMetaDataCallbacks.size(); i++)
	{
		mMetaDataCallbacks[i](stream);
	}
}

void SerializationRegistry::registerRepXSerializer(PxType type, PxRepXSerializer& serializer)
{
	if(mRepXSerializers.find(type))
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::registerRepXSerializer: Type %d has already been registered", type);	
	}

	mRepXSerializers.insert(type, &serializer);	
}

PxRepXSerializer* SerializationRegistry::getRepXSerializer(const char* typeName) const
{
	SerializationRegistry* sr = const_cast<SerializationRegistry*>(this);
	for( RepXSerializerMap::Iterator iter = sr->mRepXSerializers.getIterator(); !iter.done(); ++iter)
	{
		if ( physx::PxStricmp( iter->second->getTypeName(), typeName ) == 0 )
			return iter->second;
	}
	return NULL;
}

PxRepXSerializer* SerializationRegistry::unregisterRepXSerializer(PxType type)
{
	const RepXSerializerMap::Entry* e = mRepXSerializers.find(type);
	PxRepXSerializer* s = 	e ? e->second : NULL;

	if(!mRepXSerializers.erase(type))
	{
		shdfnd::getFoundation().error(physx::PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, 
			"PxSerializationRegistry::unregisterRepXSerializer: failed to find PxRepXSerializer instance for type %d", type);	
	}
	return s;
}

void sortCollection(Cm::Collection& collection, SerializationRegistry& sr, bool isRepx)
{
	CollectionSorter sorter(collection, sr, isRepx);	
	sorter.sort();	
}

} // Sn

PxSerializationRegistry* PxSerialization::createSerializationRegistry(PxPhysics& physics)
{
	return PX_NEW(Sn::SerializationRegistry)(physics);	
}

} // physx

