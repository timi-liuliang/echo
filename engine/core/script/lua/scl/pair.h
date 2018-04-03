////////////////////////////////////////////////////////////////////////////////
//	pair
//	2011.03.18 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

namespace scl {

template<typename key_T, typename value_T>
class pair
{
public:
	key_T		first;
	value_T		second;
	pair()  {}
	pair(const key_T& k, const value_T& v) : first(k), second(v) {  }
};

template<typename T1, typename T2>
pair<T1, T2> make_pair(const T1& t1, const T2& t2) 
{ 
	return pair<T1, T2>(t1, t2); 
}

} //namespace scl
