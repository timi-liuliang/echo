#include "physx_shape_mesh.h"
#include "../physx_module.h"
#include "engine/core/io/io.h"

namespace Echo
{
	PhysxShapeMesh::PhysxShapeMesh()
	{
		// set default rotation
		Quaternion quat; quat.fromAxisAngle(Vector3::UNIT_Z, Math::PI_DIV2);
		setLocalOrientation(quat);
	}

	PhysxShapeMesh::~PhysxShapeMesh()
	{

	}

	void PhysxShapeMesh::bindMethods()
	{
		CLASS_BIND_METHOD(PhysxShapeMesh, getDataPath);
		CLASS_BIND_METHOD(PhysxShapeMesh, setDataPath);

		CLASS_REGISTER_PROPERTY(PhysxShapeMesh, "Data", Variant::Type::ResourcePath, getDataPath, setDataPath);
	}

	void PhysxShapeMesh::setDataPath(const ResourcePath& path)
	{
		if (m_dataPath.setPath(path.getPath()))
		{
			String heightmapPath = m_dataPath.getPath() + "heightmap.png";
			if (IO::instance()->isExist(heightmapPath))
			{
				m_heightmapImage = Image::loadFromFile(heightmapPath);
			}
		}
	}

	physx::PxShape* PhysxShapeMesh::createPxShape()
	{
		using namespace physx;

		PxPhysics* physics = PhysxModule::instance()->getPxPhysics();
		if (physics && m_heightmapImage)
		{
			i32 bytes = sizeof(physx::PxHeightFieldSample) * m_heightmapImage->getWidth() * m_heightmapImage->getHeight();
			physx::PxHeightFieldSample* samples = (physx::PxHeightFieldSample*)EchoMalloc(bytes);
			for (i32 r = 0; r < m_heightmapImage->getHeight(); r++)
			{
				for (i32 w = 0; w < m_heightmapImage->getWidth(); w++)
				{
					i32 offset = r * m_heightmapImage->getWidth() + w;
					samples[offset].height = m_heightmapImage->getColor(w, r, 0).r * 65535;
					samples[offset].materialIndex0 = 0;
					samples[offset].materialIndex1 = 0;
				}
			}

			physx::PxHeightFieldDesc hfDesc;
			hfDesc.format = physx::PxHeightFieldFormat::eS16_TM;
			hfDesc.nbColumns = m_heightmapImage->getWidth();
			hfDesc.nbRows = m_heightmapImage->getHeight();
			hfDesc.samples.data = samples;
			hfDesc.samples.stride = sizeof(physx::PxHeightFieldSample);

			m_pxHeightField = PhysxModule::instance()->getPxCooking()->createHeightField(hfDesc, physics->getPhysicsInsertionCallback());

			physx::PxTransform pxTransform((physx::PxVec3&)getLocalPosition(), (physx::PxQuat&)getLocalOrientation());
			PxShape* shape = physics->createShape(PxHeightFieldGeometry(m_pxHeightField, PxMeshGeometryFlags(), 1.f, 1.f, 1.f), *m_pxMaterial);
			shape->setLocalPose(pxTransform);

			return shape;
		}

		return nullptr;
	}
}