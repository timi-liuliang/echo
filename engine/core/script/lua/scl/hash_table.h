////////////////////////////////////////////////////////////////////////////////
//	hash table
//	2010.07.11 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/varray.h"
#include "scl/array.h"
#include "scl/math.h"
#include "scl/string.h"

namespace scl
{


////////////////////////////////////////////////////////////////////////////////
//	class HashNodeType
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T>
class hash_node
{
public:
	enum NODE_STATE
	{
		EMPTY,	//该槽位为空
		USING,	//该槽位正在使用
		TOMB,	//该槽位已经被释放	
	};
	key_T		key;
	value_T		value;
	char		state;
	hash_node() : state(EMPTY) {}
};

////////////////////////////////////////////////////////////////////////////////
//	class hash_table
//	处理冲突的方法:开放寻址法,二次探测再散列
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T>
class hash_table
{
public:
	typedef hash_node<key_T, value_T>	node_T;
	static const int DEFAULT_CONFLICT_COUNT = 16;
public:
	hash_table() : m_minPrime(0), m_maxConflictTime(0) {}
	void			init	(const int maxSizeWithConflict);
	void			reserve_default_conflict(const int maxSize) { init(maxSize * DEFAULT_CONFLICT_COUNT); }
	int				capacity() const { return m_hashTable.capacity(); }
	void			clear	();

	void			add		(const key_T& key, const value_T& value);
	void			erase	(const key_T& key);
	const value_T&	find	(const key_T& key) const;
	bool			count	(const key_T& key) const;
	int				find_index(const key_T& key) const; //return -1 if find failed
	const value_T&	get_value(const int index) const { return m_hashTable[index].value; }
	bool			is_init	() { return m_hashTable.begin() != NULL; }

	value_T&		operator[](const key_T& key)			{ return const_cast<value_T&>(find(key)); }
	const value_T&	operator[](const key_T& key) const	{ return find(key); }

	int				__for_debug_state_count(int state, int* maxConflict);
	void			get_values(varray<value_T>& output);

private:
	bool			_try_add(const key_T& key, const value_T& value, const uint hashValue);
	void			_grow();

private:
	varray<node_T>	m_hashTable;
	int				m_minPrime;
	int				m_maxConflictTime;
};


//hash functions
inline uint hash_function(const int&		key);
inline uint hash_function(const uint&		key);
inline uint hash_function(const int64&		key);
inline uint hash_function(const uint64&	key);
inline uint hash_function(void* const& key);

template<int N> inline uint hash_function(const string<N>& s);
inline uint	hash_function(const pstring&	s);
template<typename T, int N>		inline uint	hash_function(const array<T, N>&		a);

////////////////////////////////////
//class hash_table
////////////////////////////////////
template<typename key_T, typename value_T>
void hash_table<key_T, value_T>::init(const int maxSize)
{
	assert(maxSize > 1);
	m_minPrime = min_prime(maxSize);
	m_hashTable.reserve(m_minPrime);
	m_hashTable.resize(m_minPrime);
}


template<typename key_T, typename value_T>
void scl::hash_table<key_T, value_T>::_grow()
{
	varray<node_T> temp;
	temp.reserve(m_hashTable.capacity());
	temp.resize(m_hashTable.capacity());
	for (int i = 0; i < m_hashTable.capacity(); ++i)
	{
		temp[i] = m_hashTable[i];
	}

	static const int GROW_STEP_MULTIPLE = 16;
	init(GROW_STEP_MULTIPLE * m_hashTable.capacity());
	clear();

	for (int i = 0; i < temp.capacity(); ++i)
	{
		node_T& node = temp[i];
		if (node.state != node_T::USING)
			continue;
		uint hashValue = hash_function(node.key);
		bool succeed = _try_add(node.key, node.value, hashValue);
		assert(succeed && "hash table is full for some key!");
	}
}

template<typename key_T, typename value_T>
void hash_table<key_T, value_T>::add(const key_T& key, const value_T& value)
{
	const uint	hashValue		= hash_function(key);
	bool		succeed			= false;
	int			growTimes		= 0;
	const int	MAX_GROW_TIMES	= 4;

	while (!succeed)
	{
		succeed = _try_add(key, value, hashValue);
		if (!succeed)
		{
			++growTimes;
			_grow();
		}
		if (growTimes > MAX_GROW_TIMES)
		{
			break;
		}
	}
	assert(succeed && "hash table is full for some key!");

	/*bool succeed = _try_add(key, value, hashValue);
	if (succeed)
		return;*/

	//_grow();
	//succeed = _try_add(key, value, hashValue);
	//assert(succeed && "hash table is full for some key!");
}

template<typename key_T, typename value_T>
bool hash_table<key_T, value_T>::_try_add(const key_T& key, const value_T& value, const uint hashValue)
{
	bool probeSuccess = false;
	//二次探测再散列
	int tombIndex = -1;
	int conflict = 0;
	for (int i = 0; i < m_minPrime; ++i)
	{
		const int probeIndex = (hashValue + (i * i)) % m_minPrime;
		node_T& node = m_hashTable[probeIndex];
		if (node.state == node_T::EMPTY)
		{
			if (tombIndex == -1)
			{
				node.key	= key;
				node.value	= value;
				node.state	= node_T::USING;
				probeSuccess = true;
			}
			break;
		}
		else if (node.state == node_T::TOMB) //找到一个坟墓位置，记录这个位置，并继续查找以保证后续没有重复的key
		{
			if (tombIndex == -1)
				tombIndex = probeIndex;
		}
		else
		{
			if (tombIndex == -1)
				++conflict;
			assert(!(node.key == key)); //key already exists
		}
	}
	if (tombIndex >= 0)
	{
		node_T& node	= m_hashTable[tombIndex];
		node.key		= key;
		node.value		= value;
		node.state		= node_T::USING;
		probeSuccess	= true;
	}
	if (conflict > m_maxConflictTime)
		m_maxConflictTime = conflict;
	return probeSuccess;
	//assert(probeSuccess && "hash table is full for some key!");
}

template<typename key_T, typename value_T>
const value_T& hash_table<key_T, value_T>::find(const key_T& key) const
{
	const uint hashValue = hash_function(key);
	//bool probeSuccess = false;
	//二次探测再散列
	for (int i = 0; i < m_minPrime; ++i)
	{
		const int probeIndex = (hashValue + (i * i)) % m_minPrime;
		const node_T& node = m_hashTable[probeIndex];
		if (node.state == node_T::EMPTY)
			break; //没找到
		else if (node.state == node_T::TOMB)
			continue;
		else if (node.state == node_T::USING)
		{
			if (node.key == key)
				return node.value;
		}
		else
			assertf(0, "invalid node status %d", node.state);
	}
	//没找到要返回什么?
	assert(false);
	throw 1;
	//return m_hashTable[0].value;
}

template<typename key_T, typename value_T>
int hash_table<key_T, value_T>::find_index(const key_T& key) const
{
	const uint hashValue = hash_function(key);
	//bool probeSuccess = false;
	//二次探测再散列
	for (int i = 0; i < m_minPrime; ++i)
	{
		const int probeIndex = (hashValue + (i * i)) % m_minPrime;
		const node_T& node = m_hashTable[probeIndex];
		if (node.state == node_T::EMPTY)
			break; //没找到
		else if (node.state == node_T::TOMB)
			continue;
		else if (node.state == node_T::USING)
		{
			if (node.key == key)
				return probeIndex;
		}
		else
		{
			assertf(0, "invalid node status %d", node.state);
			return -1;
		}
	}
	return -1;
}


template<typename key_T, typename value_T>
bool hash_table<key_T, value_T>::count(const key_T& key) const
{
	const uint hashValue = hash_function(key);
	//bool probeSuccess = false;
	for (int i = 0; i < m_minPrime; ++i)
	{
		const int probeIndex = (hashValue + (i * i)) % m_minPrime;
		const node_T& node = m_hashTable[probeIndex];
		if (node.state == node_T::EMPTY)
			break; //没找到
		else if (node.state == node_T::TOMB)
			continue;
		else if (node.state == node_T::USING)
		{
			if (node.key == key)
				return true;
		}
		else
			assertf(0, "invalid node status %d", node.state);
	}	
	return false;
}


template<typename key_T, typename value_T>
void hash_table<key_T, value_T>::clear()
{
	for (int i = 0; i < m_hashTable.capacity(); ++i)
		m_hashTable[i].state = node_T::EMPTY;
}


template<typename key_T, typename value_T>
void hash_table<key_T, value_T>::erase(const key_T& key)
{
	const uint hashValue = hash_function(key);
	//bool probeSuccess = false;
	for (int i = 0; i < m_minPrime; ++i)
	{
		const int probeIndex = (hashValue + (i * i)) % m_minPrime;
		node_T& node = m_hashTable[probeIndex];
		if (node.state == node_T::EMPTY)
			break;
		else if (node.state == node_T::TOMB)
			continue;
		else if (node.state == node_T::USING)
		{
			if (node.key == key)
			{
				node.state = node_T::TOMB;
				return;
			}
		}
	}
}

template<typename key_T, typename value_T>
void hash_table<key_T, value_T>::get_values(varray<value_T>& output)
{
	for (int i = 0; i < m_hashTable.capacity(); ++i)
	{
		node_T& node = m_hashTable[i];
		if (node.state == node_T::USING)
			output.push_back(node.value);
	}
}


template<typename key_T, typename value_T>
int hash_table<key_T, value_T>::__for_debug_state_count(int state, int* maxConflict)
{
	int c = 0;
	for (int i = 0; i < m_hashTable.capacity(); ++i)
	{
		node_T& node = m_hashTable[i];
		if (node.state == state)	
			++c;
	}
	if (NULL != maxConflict)
		*maxConflict = m_maxConflictTime;
	return c;
}

////////////////////////////////////
//hash_value functions
////////////////////////////////////
inline uint hash_function(const int& key)
{
	return static_cast<uint>(key);
}

inline uint hash_function(const uint& key)
{
	return key;
}

inline uint hash_function(const int64& key)
{
	return static_cast<uint>(key);
}

inline uint hash_function(const uint64& key)
{
	return static_cast<uint>(key);
}

inline uint hash_function(void* const& key)
{
	return static_cast<uint>(reinterpret_cast<uint64>(key));
}


template<int N>
inline uint hash_function(const string<N>& s)
{
	uint result = 0;
	const int max_size = s.max_size();
	for (int i = 0; ; ++i)
	{
		const char c = s[i];
		if (c == 0 || i >= max_size)
			break;
		//time 33 : hashValue * 33 = ((hashValue << 5) + hashValue);
		result = ((result << 5) + result) + c;

		//time 131 : hashValue * 131 = ((hashValue << 5) + hashValue);
		//result = ((result << 7) + (result << 1) + result) + c;
	}
	return result;
}

inline uint hash_function(const pstring& s)
{
	uint result = 0;
	const int max_size = s.max_size();
	for (int i = 0; ; ++i)
	{
		const char c = s[i];
		if (c == 0 || i >= max_size)
			break;

		//time 33 : hashValue * 33 = ((hashValue << 5) + hashValue);
		result = ((result << 5) + result) + s[i];
	}
	return result;
}

template<typename T, int N>
inline uint hash_function(const array<T, N>& a)
{
	uint result = 0;
	for (int i = 0; i < a.size(); ++i)
	{
		//time 33 : hashValue * 33 = ((hashValue << 5) + hashValue);
		result = ((result << 5) + result) + hash_function(a[i]);
	}
	return result;
}


} //namespace scl

