////////////////////////////////////////////////////////////////////////////////
//	math.h
//	数学库
//	2010.05.01 caolei
////////////////////////////////////////////////////////////////////////////////

#include "scl/math.h"
#include "scl/type.h"
#include "scl/assert.h"
#include "scl/time.h"

#include <math.h>		// system <math.h>

namespace scl {

rander __scl_inner_global_unsafe_rander;

//判断一个int是否为素数
bool is_prime(uint n)
{
	int find = static_cast<int>(sqrt(static_cast<float>(n)));
	assert(find > 0);
	for (int i = 2; i <= find; ++i)
	{
		if (n % i == 0)
		{
			return false;
		}
	}
	return true;
}

//找出小于n的最大素数
uint min_prime(uint n)
{
	while (n)
	{
		if (is_prime(n))
		{
			return n;
		}
		n--;
	}
	return 0;
}

//求base的times次方
int pow(int base, int times)
{
	int result = base;
	while (--times) 
	{
		result *= base;
	}
	return result;
}


float lerpf(float from, float to, float t)
{
	return from + t * (to - from);
}

//float lerpf(float from, float to, float t_from, float t_to, float t_value)
//{
//	float t = (t_value - t_from) / (t_to - t_from);
//	return lerpf(from, to, t);
//}
//
//float lerpf(float from, float to, uint t_from, uint t_to, uint t_value)
//{
//	float t = (t_value - t_from) / static_cast<float>(t_to - t_from);
//	return lerpf(from, to, t);
//}

int lerp(int from, int to, float t)
{
	return static_cast<int>(from + t * (to - from));
}


rander::rander()
{
	//这里不能使用SCL_TICK的原因是，如果rander是全局变量，那么SCL_TICK中可能先创建线程，然后才执行到thread_time_holder的构造函数，冲掉先前创建的线程信息
	//rander的初始化也并非一个频繁调用的地方，所以也没有必要使用SCL_TICK提高效率
	m_seed = 0;// static_cast<unsigned int>(get_realtime_tick());
}

unsigned int rander::rand16()
{
	m_seed = m_seed * 214013L + 2531011L;

	//正常线性同余法是取seed % 32767，即seed的低16位，
	//这里直接取seed高16位,结果是一样的
	return ((m_seed >> 16) & 0x7fff);
}

unsigned int rander::rand32()
{
	m_seed = m_seed * 214013L + 2531011L;
	return (m_seed & 0x7fffffff);
}

bool rander::rand_percent(const float percent)	//rand percent, return is success;
{
	const unsigned int percent_int = static_cast<unsigned int>(percent * MAX32);
	unsigned int r = rand32();
	return (r < percent_int);
}

int rander::rand(const int min, const int max)
{
	//const float max_from_0 = static_cast<float>(max - min);
	//const unsigned int r = static_cast<unsigned int>((max_from_0 / MAX32) * rand32() + min);
	if (max < min)
	{
		assert(false);
		return 0;
	}
	const int start_from_0 = max - min + 1; //note: why +1, (max - min) is used for [min,max-1),so (max-min+1) for [min,max]

	//const int r = static_cast<int>(rand32() % (start_from_0 + 1) + min);
	double r32 = rand32();
	//const double dr = (r32 / double(0x7fffffff)) * start_from_0 + min;
	const double dr = (r32 / double(0x80000000)) * start_from_0 + min;
	const int r = static_cast<int>(dr);

	return r;
}

void rander::srand(const int seed)
{
	m_seed = seed;
}

unsigned int rand(const int min, const int max) //全局rand，线程不安全
{
	return __scl_inner_global_unsafe_rander.rand(min, max);
}

unsigned int rand16()
{
	return __scl_inner_global_unsafe_rander.rand16();
}

unsigned int rand32()
{
	return __scl_inner_global_unsafe_rander.rand32();
}

void srand(int t)
{
	__scl_inner_global_unsafe_rander.srand(t);
}


}	//namespace scl

