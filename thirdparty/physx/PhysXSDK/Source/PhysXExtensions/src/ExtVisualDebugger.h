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


#ifndef EXT_VISUAL_DEBUGGER_H
#define EXT_VISUAL_DEBUGGER_H

#include "pvd/PxVisualDebugger.h"

#if PX_SUPPORT_VISUAL_DEBUGGER

#include "CmPhysXCommon.h"
#include "PsUserAllocated.h"
#include "PxVisualDebuggerExt.h"
#include "PxJoint.h"
#include "PvdDataStream.h"
#include "PxExtensionMetaDataObjects.h"
#include "PvdTypeNames.h"
#include "PvdObjectModelBaseTypes.h"

namespace physx
{

class PxJoint;
class PxD6Joint;
class PxDistanceJoint;
class PxFixedJoint;
class PxPrismaticJoint;
class PxRevoluteJoint;
class PxSphericalJoint;

#define JOINT_GROUP 3

namespace debugger {
	#define DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP( type ) DEFINE_PVD_TYPE_NAME_MAP( type, "physx3", #type )

	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxFixedJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxFixedJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxDistanceJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxDistanceJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPrismaticJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxPrismaticJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRevoluteJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxRevoluteJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSphericalJoint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxSphericalJointGeneratedValues)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxD6Joint)
	DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP(PxD6JointGeneratedValues)

#undef DEFINE_NATIVE_PVD_PHYSX3_TYPE_MAP

}

using namespace physx::debugger::comm;

namespace Ext
{
	class VisualDebugger: public PxVisualDebuggerExt, public Ps::UserAllocated
	{
		VisualDebugger& operator=(const VisualDebugger&);
	public:
		class PvdNameSpace
		{
		
		public:
			PvdNameSpace(physx::debugger::comm::PvdDataStream& conn, const char* name);
			~PvdNameSpace();
		private:
			PvdNameSpace& operator=(const PvdNameSpace&);
			physx::debugger::comm::PvdDataStream& mConnection;
		};

		static void setActors( physx::debugger::comm::PvdDataStream& PvdDataStream, const PxJoint& inJoint, const PxConstraint& c, const PxActor* newActor0, const PxActor* newActor1 );
		
		template<typename TObjType>
		static void createInstance( physx::debugger::comm::PvdDataStream& inStream, const PxConstraint& c, const TObjType& inSource )
		{
			const PxJoint* theJoint = &inSource;			
			inStream.createInstance( &inSource );
			inStream.pushBackObjectRef( c.getScene(), "Joints", (const void*)theJoint );

			class ConstraintUpdateCmd : public PvdDataStream::PvdCommand
			{
				ConstraintUpdateCmd &operator=(const ConstraintUpdateCmd&) { PX_ASSERT(0); return *this; } //PX_NOCOPY doesn't work for local classes
			public:

				const PxConstraint& mConstraint;
				const PxJoint& mJoint;

				PxRigidActor* actor0, *actor1;
				ConstraintUpdateCmd(const PxConstraint& constraint, const PxJoint& joint):PvdDataStream::PvdCommand(), mConstraint(constraint), mJoint(joint)
				{
					mConstraint.getActors( actor0, actor1 ); 
				}

							//Assigned is needed for copying
				ConstraintUpdateCmd(const ConstraintUpdateCmd& cmd)
					:PvdDataStream::PvdCommand(), mConstraint(cmd.mConstraint), mJoint(cmd.mJoint)
				{					
				}

				virtual bool canRun(PvdInstanceDataStream &inStream_ )
				{
					PX_ASSERT(inStream_.isInstanceValid(&mJoint));
					//When run this command, the constraint maybe buffer removed
					return ((actor0 == NULL) || inStream_.isInstanceValid(actor0))
						&&  ((actor1 == NULL) || inStream_.isInstanceValid(actor1));
				}
				virtual void run( PvdInstanceDataStream &inStream_ )
				{
					//When run this command, the constraint maybe buffer removed
					if(!inStream_.isInstanceValid(&mJoint))
						return;

					PxRigidActor* actor0_, *actor1_;
					mConstraint.getActors( actor0_, actor1_ );

					if ( actor0_ && (inStream_.isInstanceValid(actor0_)) )
						inStream_.pushBackObjectRef( (PxActor*)actor0_, "Joints", &mJoint );
					if ( actor1_ && (inStream_.isInstanceValid(actor1_)) )
						inStream_.pushBackObjectRef( (PxActor*)actor1_, "Joints", &mJoint );
					const void* parent = actor0_ ? (const void*)actor0_ : (const void*) actor1_;
					inStream_.setPropertyValue( &mJoint, "Parent", parent );
				}
			};

			ConstraintUpdateCmd* cmd = PX_PLACEMENT_NEW(inStream.allocateMemForCmd(sizeof(ConstraintUpdateCmd)),
				ConstraintUpdateCmd)(c, *theJoint);
			
			if(cmd->canRun( inStream ))
				cmd->run( inStream );
			else
				inStream.pushPvdCommand( *cmd );
		}

		template<typename jointtype, typename structValue>
		static void updatePvdProperties(physx::debugger::comm::PvdDataStream& pvdConnection, const jointtype& joint)
		{
			structValue theValueStruct( &joint );
			pvdConnection.setPropertyMessage( &joint, theValueStruct );
		}
		
		template<typename jointtype>
		static void simUpdate(physx::debugger::comm::PvdDataStream& /*pvdConnection*/, const jointtype& /*joint*/) {}		
		
		template<typename jointtype>
		static void createPvdInstance(physx::debugger::comm::PvdDataStream& pvdConnection, const PxConstraint& c, const jointtype& joint)
		{
			createInstance<jointtype>( pvdConnection, c, joint );		
		}

		static void releasePvdInstance(physx::debugger::comm::PvdDataStream& pvdConnection, const PxConstraint& c, const PxJoint& joint);
		static void sendClassDescriptions(physx::debugger::comm::PvdDataStream& pvdConnection);
	};
}

}

#endif // PX_SUPPORT_VISUAL_DEBUGGER
#endif // EXT_VISUAL_DEBUGGER_H
