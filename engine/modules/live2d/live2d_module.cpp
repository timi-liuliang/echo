#include "live2d_module.h"
#include "live2d.h"

namespace Echo
{
	void Live2DModule::registerTypes()
	{
		Class::registerType<Live2d>();
	}
}