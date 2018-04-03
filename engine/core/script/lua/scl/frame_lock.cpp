#include "scl/frame_lock.h"
#include "scl/time.h"

namespace scl {

void frame_lock::start()
{
	m_start = SCL_TICK;
}

void frame_lock::wait(const int locktime)
{
	uint64 elapsed = SCL_TICK - m_start;
	if (elapsed < static_cast<uint64>(locktime)) 
		scl::sleep(static_cast<int>(locktime - elapsed));
	else
		scl::sleep(0);
}

} //namespace scl 
