#include "pvd_command.h"
#include "../physx_module.h"
#include "engine/core/log/Log.h"

namespace Echo
{
	PvdCommand::PvdCommand()
	{

	}

	PvdCommand::~PvdCommand()
	{

	}

	void PvdCommand::bindMethods()
	{

	}

	bool PvdCommand::exec(const StringArray& args)
	{
		pvdConnect();

		return true;
	}

	void PvdCommand::pvdConnect()
	{
		physx::PxPvd* pvd = PhysxModule::instance()->getPxPvd();
		if (pvd)
		{
			// make sure we're disconnected first
			pvd->disconnect();

			physx::PxPvdTransport* transport = physx::PxDefaultPvdSocketTransportCreate("localhost", 5425, 10);
			pvd->connect(*transport, physx::PxPvdInstrumentationFlag::eALL);

			if (pvd->isConnected())
			{
				EchoLogInfo("Physx Visual Debugger connect succeed.");
			}
			else
			{
				EchoLogError("Physx Visual Debugger connect failed.");
			}
		}
	}
}