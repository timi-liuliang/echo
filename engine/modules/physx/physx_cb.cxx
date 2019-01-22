#include <engine/core/log/Log.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>

namespace Echo
{
	class PhysxAllocatorCb : public physx::PxAllocatorCallback
	{
	public:
		PhysxAllocatorCb() : PxAllocatorCallback() {}
		virtual ~PhysxAllocatorCb() {}

		// allocate
		virtual void* allocate(size_t size, const char* typeName, const char* file, int line) override
		{
			return ECHO_MALLOC_ALIGN(size, 16);
		}

		// dellocate
		virtual void deallocate(void* ptr) override
		{
			ECHO_FREE_ALIGN(ptr, 16);
		}
	};

	class PhysxErrorReportCb : public physx::PxErrorCallback
	{
	public:
		PhysxErrorReportCb() : PxErrorCallback() {}
		~PhysxErrorReportCb() {}

		// reporter erro
		virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
		{
			switch (code)
			{
			case physx::PxErrorCode::eINVALID_PARAMETER:
			case physx::PxErrorCode::eINVALID_OPERATION:
			case physx::PxErrorCode::eOUT_OF_MEMORY:
			case physx::PxErrorCode::eINTERNAL_ERROR:
			case physx::PxErrorCode::eABORT:
			case physx::PxErrorCode::eNO_ERROR:			EchoLogError("Physx [%s]", message);	break;
			case physx::PxErrorCode::eDEBUG_INFO:		EchoLogInfo("Physx [%s]", message);		break;
			case physx::PxErrorCode::ePERF_WARNING:
			case physx::PxErrorCode::eDEBUG_WARNING:	EchoLogWarning("Physx [%s]", message);	break;
			default:									EchoLogInfo("Physx [%s]", message);		break;
			}
		}
	};
}