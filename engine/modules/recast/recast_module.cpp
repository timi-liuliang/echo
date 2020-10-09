#include "recast_module.h"
#include "recast_crowd_agent.h"
#include "recast_nav_convex_volume.h"
#include "recast_nav_input_geom.h"
#include "recast_nav_mesh.h"
#include "recast_nav_temp_obstacle.h"
#include "recast_off_mesh_link.h"

namespace Echo
{
	DECLARE_MODULE(RecastModule)

	RecastModule::RecastModule()
	{
	}

	RecastModule* RecastModule::instance()
	{
		static RecastModule* inst = EchoNew(RecastModule);
		return inst;
	}

	void RecastModule::bindMethods()
	{

	}

	void RecastModule::registerTypes()
	{
		Class::registerType<RecastCrowdAgent>();
		Class::registerType<RecastNavConvexVolume>();
		Class::registerType<RecastNavInputGeom>();
		Class::registerType<RecastNavMesh>();
		Class::registerType<RecastNavTempObstacle>();
		Class::registerType<RecastOffMeshLink>();
	}
}