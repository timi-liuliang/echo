#pragma once

#include "scl/type.h"

namespace scl {

class frame_lock
{
public:
	frame_lock() : m_start(0) {}

	void	start	();
	void	wait	(const int locktime = 20); //锁帧时间，单位：毫秒，默认值20毫秒，即50帧

private:
	uint64	m_start;
};

} //namespace scl 
