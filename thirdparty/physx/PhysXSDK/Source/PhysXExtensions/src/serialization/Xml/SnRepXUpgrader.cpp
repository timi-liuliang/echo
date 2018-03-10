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

#include "PxProfileMemoryEvents.h" 
#include "SnXmlImpl.h" 
#include "SnXmlReader.h"
#include "SnXmlMemoryAllocator.h"
#include "PsFoundation.h"
#include "SnRepXCollection.h"
#include "SnRepXUpgrader.h"

using namespace physx::profile;

namespace physx { namespace Sn {

	#define DEFINE_REPX_DEFAULT_PROPERTY( name, val ) RepXDefaultEntry( name, val ),

	static RepXDefaultEntry gRepX1_0Defaults[] = {
	#include "SnRepX1_0Defaults.h"
	};
	static PxU32 gNumRepX1_0Default = sizeof( gRepX1_0Defaults ) / sizeof ( *gRepX1_0Defaults );

	static RepXDefaultEntry gRepX3_1Defaults[] = {
	#include "SnRepX3_1Defaults.h"
	};
	static PxU32 gNumRepX3_1Defaults = sizeof( gRepX3_1Defaults ) / sizeof ( *gRepX3_1Defaults );

	static RepXDefaultEntry gRepX3_2Defaults[] = {
	#include "SnRepX3_2Defaults.h"
	};
	static PxU32 gNumRepX3_2Defaults = sizeof( gRepX3_2Defaults ) / sizeof ( *gRepX3_2Defaults );
	
	inline const char* nextPeriod( const char* str )
	{
		for( ++str; str && *str && *str != '.'; ++str ); //empty loop intentional
		return str;
	}
	
	typedef ProfileHashMap<const char*, PxU32> TNameOffsetMap;
	
	void setMissingPropertiesToDefault( XmlNode* topNode, XmlReaderWriter& editor, const RepXDefaultEntry* defaults, PxU32 numDefaults, TNameOffsetMap& map )
	{
		for ( XmlNode* child = topNode->mFirstChild; child != NULL; child = child->mNextSibling )
			setMissingPropertiesToDefault( child, editor, defaults, numDefaults, map );

		const TNameOffsetMap::Entry* entry( map.find( topNode->mName ) );
		if ( entry )
		{
			XmlReaderWriter& theReader( editor );
			theReader.setNode( *topNode );
			char nameBuffer[512] = {0};
			size_t nameLen = strlen( topNode->mName );
			//For each default property entry for this node type.
			for ( const RepXDefaultEntry* item = defaults + entry->second; strncmp( item->name, topNode->mName, nameLen ) == 0; ++item )
			{
				bool childAdded = false;
				const char* nameStart = item->name + nameLen;
				++nameStart;
				theReader.pushCurrentContext();
				const char* str = nameStart;
				while( *str )
				{
					 const char *period = nextPeriod( str );
					 size_t len = (size_t)PxMin( period - str, ptrdiff_t(1023) ); //can't be too careful these days.
					 memcpy( nameBuffer, str, len );
					 nameBuffer[len] = 0;
					 if ( theReader.gotoChild( nameBuffer ) == false )
					 {
						 childAdded = true;
						 theReader.addOrGotoChild( nameBuffer );
					 }
					 if (*period )
						 str = period + 1;
					 else
						 str = period;
				}
				if ( childAdded )
					theReader.setCurrentItemValue( item->value );
				theReader.popCurrentContext();
			}
		}
	}

	
	static void setMissingPropertiesToDefault( RepXCollection& collection, XmlReaderWriter& editor, const RepXDefaultEntry* defaults, PxU32 numDefaults )
	{
		FoundationWrapper wrapper( collection.getAllocator() );
		//Release all strings at once, instead of piece by piece
		XmlMemoryAllocatorImpl alloc( collection.getAllocator() );
		//build a hashtable of the initial default value strings.
		TNameOffsetMap nameOffsets( wrapper );
		for ( PxU32 idx = 0; idx < numDefaults; ++idx )
		{
			const RepXDefaultEntry& item( defaults[idx] );
			size_t nameLen = 0;
			const char* periodPtr = nextPeriod (item.name);
			for ( ; periodPtr && *periodPtr; ++periodPtr ) if( *periodPtr == '.' )	break;
			if ( periodPtr == NULL || *periodPtr != '.' ) continue;
			nameLen = size_t(periodPtr - item.name);
			char* newMem = (char*)alloc.allocate( PxU32(nameLen + 1) );
			memcpy( newMem, item.name, nameLen );
			newMem[nameLen] = 0;
		
			if ( nameOffsets.find( newMem ) )
				alloc.deallocate( (PxU8*)newMem );
			else
				nameOffsets.insert( newMem, idx );
		}
		//Run through each collection item, and recursively find it and its children
		//If an object's name is in the hash map, check and add any properties that don't exist.
		//else return.
		for ( const RepXCollectionItem* item = collection.begin(), *end = collection.end(); item != end; ++ item )
		{
			RepXCollectionItem theItem( *item );
			setMissingPropertiesToDefault( theItem.descriptor, editor, defaults, numDefaults, nameOffsets );
		}
	}

	struct RecursiveTraversal
	{
		RecursiveTraversal(XmlReaderWriter& editor): mEditor(editor) {}
		void traverse()
		{
			mEditor.pushCurrentContext();
			updateNode();
			for(bool exists = mEditor.gotoFirstChild(); exists; exists = mEditor.gotoNextSibling())
				traverse();
			mEditor.popCurrentContext();
		}
		virtual void updateNode() = 0;
        virtual ~RecursiveTraversal() {}
		XmlReaderWriter& mEditor;
	protected:
		RecursiveTraversal& operator=(const RecursiveTraversal&){return *this;}
	};


	RepXCollection& RepXUpgrader::upgrade10CollectionTo3_1Collection(RepXCollection& src)
	{
		XmlReaderWriter& editor( src.createNodeEditor() );
		setMissingPropertiesToDefault(src, editor, gRepX1_0Defaults, gNumRepX1_0Default );

		
		RepXCollection* dest = &src.createCollection("3.1.1");
		
		for ( const RepXCollectionItem* item = src.begin(), *end = src.end(); item != end; ++ item )
		{
			//either src or dest could do the copy operation, it doesn't matter who does it.
			RepXCollectionItem newItem( item->liveObject, src.copyRepXNode( item->descriptor ) );
			editor.setNode( *const_cast<XmlNode*>( newItem.descriptor ) );
			//Some old files have this name in their system.
			editor.renameProperty( "MassSpaceInertia", "MassSpaceInertiaTensor" );
			editor.renameProperty( "SleepEnergyThreshold", "SleepThreshold" );

			if ( strstr( newItem.liveObject.typeName, "Joint" ) || strstr( newItem.liveObject.typeName, "joint" ) )
			{
				//Joints changed format a bit.  old joints looked like:
				/*
				<Actor0 >1627536</Actor0>
				<Actor1 >1628368</Actor1>
				<LocalPose0 >0 0 0 1 0.5 0.5 0.5</LocalPose0>
				<LocalPose1 >0 0 0 1 0.3 0.3 0.3</LocalPose1>*/
				//New joints look like:
				/*
				<Actors >
					<actor0 >58320336</actor0>
					<actor1 >56353568</actor1>
				</Actors>
				<LocalPose >
					<eACTOR0 >0 0 0 1 0.5 0.5 0.5</eACTOR0>
					<eACTOR1 >0 0 0 1 0.3 0.3 0.3</eACTOR1>
				</LocalPose>
				*/
				const char* actor0, *actor1, *lp0, *lp1;
				editor.readAndRemoveProperty( "Actor0", actor0 );
				editor.readAndRemoveProperty( "Actor1", actor1 ); 
				editor.readAndRemoveProperty( "LocalPose0", lp0 );
				editor.readAndRemoveProperty( "LocalPose1", lp1 );

				editor.addOrGotoChild( "Actors" );
				editor.writePropertyIfNotEmpty( "actor0", actor0 );
				editor.writePropertyIfNotEmpty( "actor1", actor1 );
				editor.leaveChild();

				editor.addOrGotoChild( "LocalPose" );
				editor.writePropertyIfNotEmpty( "eACTOR0", lp0 );
				editor.writePropertyIfNotEmpty( "eACTOR1", lp1 );
				editor.leaveChild();
			}



			//now desc owns the new node.  Collections share a single allocation pool, however,
			//which will get destroyed when all the collections referencing it are destroyed themselves.
			//Data on nodes is shared between nodes, but the node structure itself is allocated.
			dest->addCollectionItem( newItem );
		}
		editor.release();
		src.destroy();
		return *dest;
	}
	
	RepXCollection& RepXUpgrader::upgrade3_1CollectionTo3_2Collection(RepXCollection& src)
	{
		XmlReaderWriter& editor( src.createNodeEditor() );
		setMissingPropertiesToDefault(src, editor, gRepX3_1Defaults, gNumRepX3_1Defaults );

		RepXCollection* dest = &src.createCollection("3.2.0");

		for ( const RepXCollectionItem* item = src.begin(), *end = src.end(); item != end; ++ item )
		{
			//either src or dest could do the copy operation, it doesn't matter who does it.
			RepXCollectionItem newItem( item->liveObject, src.copyRepXNode( item->descriptor ) );
			editor.setNode( *const_cast<XmlNode*>( newItem.descriptor ) );
			
			if ( strstr( newItem.liveObject.typeName, "PxMaterial" ) )
			{
				editor.removeChild( "DynamicFrictionV" );
				editor.removeChild( "StaticFrictionV" );
				editor.removeChild( "dirOfAnisotropy" );	
			}
			//now desc owns the new node.  Collections share a single allocation pool, however,
			//which will get destroyed when all the collections referencing it are destroyed themselves.
			//Data on nodes is shared between nodes, but the node structure itself is allocated.
			dest->addCollectionItem( newItem );
		}
		editor.release();
		src.destroy();
		return *dest;
	}
	
	RepXCollection& RepXUpgrader::upgrade3_2CollectionTo3_3Collection(RepXCollection& src)
	{
		XmlReaderWriter& editor( src.createNodeEditor() );
		setMissingPropertiesToDefault(src, editor, gRepX3_2Defaults, gNumRepX3_2Defaults );

		RepXCollection* dest = &src.createCollection("3.3.0");

		

		struct RenameSpringToStiffness : public RecursiveTraversal
		{
			RenameSpringToStiffness(XmlReaderWriter& editor_): RecursiveTraversal(editor_) {}
		
			void updateNode()
			{
				mEditor.renameProperty("Spring", "Stiffness");
				mEditor.renameProperty("TangentialSpring", "TangentialStiffness");
			}
		};


		struct UpdateArticulationSwingLimit : public RecursiveTraversal
		{
			UpdateArticulationSwingLimit(XmlReaderWriter& editor_): RecursiveTraversal(editor_) {}
		
			void updateNode()
			{
				if(!PxStricmp(mEditor.getCurrentItemName(), "yLimit") && !PxStricmp(mEditor.getCurrentItemValue(), "0"))
					mEditor.setCurrentItemValue("0.785398");

				if(!PxStricmp(mEditor.getCurrentItemName(), "zLimit") && !PxStricmp(mEditor.getCurrentItemValue(), "0"))
					mEditor.setCurrentItemValue("0.785398");

				if(!PxStricmp(mEditor.getCurrentItemName(), "TwistLimit"))
				{
					mEditor.gotoFirstChild();
					PxReal lower = (PxReal)strtod(mEditor.getCurrentItemValue(), NULL);
					mEditor.gotoNextSibling();
					PxReal upper = (PxReal)strtod(mEditor.getCurrentItemValue(), NULL);
					mEditor.leaveChild();
					if(lower>=upper)
					{
						mEditor.writePropertyIfNotEmpty("lower", "-0.785398");
						mEditor.writePropertyIfNotEmpty("upper", "0.785398");
					}
				}
			}
		};


		for ( const RepXCollectionItem* item = src.begin(), *end = src.end(); item != end; ++ item )
		{
			//either src or dest could do the copy operation, it doesn't matter who does it.
			RepXCollectionItem newItem( item->liveObject, src.copyRepXNode( item->descriptor ) );	


			if ( strstr( newItem.liveObject.typeName, "PxCloth" ) || strstr( newItem.liveObject.typeName, "PxClothFabric" ) )
			{  
				physx::shdfnd::getFoundation().error(PxErrorCode::eDEBUG_WARNING, __FILE__, __LINE__, "Didn't suppot PxCloth upgrate from 3.2 to 3.3! ");
				continue;
			}

			if ( strstr( newItem.liveObject.typeName, "PxParticleSystem" ) || strstr( newItem.liveObject.typeName, "PxParticleFluid" ) )
			{
				editor.setNode( *const_cast<XmlNode*>( newItem.descriptor ) );
				editor.renameProperty( "PositionBuffer", "Positions" );
				editor.renameProperty( "VelocityBuffer", "Velocities" );
				editor.renameProperty( "RestOffsetBuffer", "RestOffsets" );
			}

			if(strstr(newItem.liveObject.typeName, "PxPrismaticJoint" ) 
			|| strstr(newItem.liveObject.typeName, "PxRevoluteJoint")
			|| strstr(newItem.liveObject.typeName, "PxSphericalJoint")
			|| strstr(newItem.liveObject.typeName, "PxD6Joint")
			|| strstr(newItem.liveObject.typeName, "PxArticulation"))
			{
				editor.setNode( *const_cast<XmlNode*>( newItem.descriptor ) );
				RenameSpringToStiffness(editor).traverse();
			}

			if(strstr(newItem.liveObject.typeName, "PxArticulation"))
			{
				editor.setNode( *const_cast<XmlNode*>( newItem.descriptor ) );
				UpdateArticulationSwingLimit(editor).traverse();
			}



			//now dest owns the new node.  Collections share a single allocation pool, however,
			//which will get destroyed when all the collections referencing it are destroyed themselves.
			//Data on nodes is shared between nodes, but the node structure itself is allocated.
			
			dest->addCollectionItem( newItem );
			
		}
		editor.release();
		src.destroy();
		
		return *dest;
	}

	RepXCollection& RepXUpgrader::upgradeCollection(RepXCollection& src)
	{
		const char* srcVersion = src.getVersion();
		if( safeStrEq( srcVersion, RepXCollection::getLatestVersion() ))
           return src;		

		static const char* oldVersions[] = {"1.0",
											"3.1",
											"3.1.1",
											"3.2.0",
											"3.3.0",
											"3.3.1"
		};//should be increase order
		
		PxU32 grade = PX_MAX_U16;
		const PxU32 count = sizeof(oldVersions)/sizeof(oldVersions[0]);
		for (PxU32 i=0; i<count; i++)
	    {
			if( safeStrEq( srcVersion, oldVersions[i] ))
			{
				grade = i;
				break;
			}
		}

		typedef RepXCollection& (*UPGRADE_FUNCTION)(RepXCollection& src);

		UPGRADE_FUNCTION procs[count] = 
		{
			upgrade10CollectionTo3_1Collection,
			NULL,
			upgrade3_1CollectionTo3_2Collection,
			upgrade3_2CollectionTo3_3Collection,
			NULL,
			NULL
		};

		RepXCollection* dest = &src;
		for( PxU32 j = grade; j < count; j++ )
		{
			if( procs[j] )
				dest = &(procs[j])(*dest);
		}

		return *dest;
	}
} }
