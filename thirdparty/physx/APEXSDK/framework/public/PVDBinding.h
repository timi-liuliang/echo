/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#ifndef PVDBINDING_H
#define PVDBINDING_H

#include "foundation/Px.h"
#include "physxvisualdebuggersdk/PvdConnectionFlags.h"
#include "NxApexDefs.h"


#define DEFAULT_PVD_BINDING_PORT 5425
#define DEFAULT_PVD_BINDING_TIMEOUT_MS 100

#define APEX_PVD_NAMESPACE "Apex"


namespace NxParameterized
{
	class Interface;
	class Definition;
	class Handle;
}

namespace physx 
{
	class PxProfileZoneManager;

	namespace debugger 
	{
		namespace comm 
		{
			class PvdConnectionManager;
			class PvdConnection;
			class PvdDataStream;
		}
		namespace renderer
		{
			class PvdUserRenderer;
		}
	}
}

namespace PVD
{
	/**
	\brief Define what action needs to be done when updating pvd with an NxParameterized object.
	*/
	struct PvdAction
	{
		/**
		\brief Enum
		*/
		enum Enum
		{
			/**
			\brief Create instances and update properties.
			*/
			UPDATE,

			/**
			\brief Destroy instances.
			*/
			DESTROY
		};
	};



	/**
	\brief The PvdBinding class allows APEX and PhysX to both connect to the PhysX Visual Debugger (PVD)
	*/
	class PvdBinding
	{
	protected:
		virtual ~PvdBinding(){}
	public:
		/**
		\brief Retrieve the PvdConnectionManager
		*/
		virtual physx::debugger::comm::PvdConnectionManager& getConnectionManager() = 0;

		/**
		\brief Retrieve the PxProfileZoneManager
		*/
		virtual physx::PxProfileZoneManager& getProfileManager() = 0;

		/**
		\brief Check if the PVD connection is active
		*/
		virtual bool isConnected() = 0;

		/**
		\brief Returns the type of the connection.
		*/
		virtual physx::debugger::TConnectionFlagsType getConnectionType() = 0;

		/**
		\brief Terminate the PVD connection
		*/
		virtual void disconnect() = 0;

		/**
		\brief Start the profiling frame
		\note inInstanceId must *not* be used already by pvd
		*/
		virtual void beginFrame( void* inInstanceId ) = 0;

		/**
		\brief End the profiling frame
		*/
		virtual void endFrame( void* inInstanceId ) = 0;
		
		/**
		\brief Destroy this instance
		*/
		virtual void release() = 0;
		/**
		 *	Connect call either connects through the legacy debugger interface and tunnels through
		 *	it if you want debug information *or* connects purely through the newer SDK connection
		 *	if you just want to send profile and memory information across.
		 */
		virtual void connect( const char* inHost
							, int inPort = DEFAULT_PVD_BINDING_PORT
							, unsigned int inTimeoutInMilliseconds = DEFAULT_PVD_BINDING_TIMEOUT_MS
							//If we are using the debug connection type, we tunnel through the
							//older debugger.  Else we don't and connect directly to pvd without
							//using the older debugger.
							, physx::debugger::TConnectionFlagsType inConnectionType = physx::debugger::defaultConnectionFlags() ) = 0;

		/**
		\brief Returns the data stream if Pvd is connected.
		*/
		virtual physx::debugger::comm::PvdDataStream* getDataStream() = 0;

		/**
		\brief Returns the PvdUserRenderer if Pvd is connected.
		*/
		virtual physx::debugger::renderer::PvdUserRenderer* getRenderer() = 0;

		/**
		\brief Initializes the classes sent to pvd.
		*/
		virtual void initPvdClasses() = 0;

		/**
		\brief Sends the existing instances to pvd.
		*/
		virtual void initPvdInstances() = 0;

		/**
		\brief Adds properties of an NxParameterized object to the provided class and creates necessary subclasses for structs.

		\note The pvd class pvdClassName must already exist. Pvd classes for structs are being created, but not for references.
		*/
		virtual void initPvdClasses(const NxParameterized::Definition& paramsHandle, const char* pvdClassName) = 0;

		/**
		\brief Creates or destroys pvdInstances and/or updates properties.
		*/
		virtual void updatePvd(const void* pvdInstance, NxParameterized::Interface& params, PvdAction::Enum pvdAction = PvdAction::UPDATE) = 0;
		
		/**
		\brief Lock access to pvd.

		*/
		virtual void lock() = 0;

		/**
		\brief Unlock access to pvd.
		*/
		virtual void unlock() = 0;

		/**
		 *	Assumes foundation is already booted up.
		 */
		static PvdBinding* create( physx::debugger::comm::PvdConnectionManager& inConnectionManager, physx::PxProfileZoneManager& inProfileManager );

		/**
		 *	Assumes foundation is already booted up, boots up the connection and profile manager and releases
		 *	them itself.
		 *	\param inRecordMemoryEvents true if you would like the system to record outstanding allocations.
		 *			This is necessary for PVD to accurately reflect what is going on w/r/t to memory allocations
		 *			but doesn't help or hurt per-frame allocation statistics.  It only serves to ensure that when
		 *			a new PVD connection is made, it is booted up with accurate oustanding memory information.
		 */
		static PvdBinding* create( bool inRecordMemoryEvents );
	};

}



#endif // PVDBINDING_H
