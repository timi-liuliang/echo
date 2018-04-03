////////////////////////////////////////////////////////////////////////////////
//	tree.h
//	Tree模板定义与实现，AVL树
//	2010.05.04 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/pair.h"
#include "scl/allocator.h"

namespace scl {

////////////////////////////////////////////////////////////////////////////////
// enum TreeChildType
////////////////////////////////////////////////////////////////////////////////
enum TREE_CHILD_TYPE
{
	TREE_CHILD_TYPE_RIGHT		= -1,
	TREE_CHILD_TYPE_INVALID		= 0,
	TREE_CHILD_TYPE_LEFT		= 1,
	TREE_CHILD_TYPE_ROOT		= 2,
};

////////////////////////////////////////////////////////////////////////////////
// class tree_node
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T>
class tree_node
{
public:
	key_T				key;
	value_T			value;
	tree_node*		left;
	tree_node*		right;
	tree_node*		parent;
	
	//平衡因子(blanceFactor)，定义为“右侧减左侧的值”，//即左侧增加为负值，右侧增加为正值
	int			bf;	
	
	tree_node() : left(NULL), right(NULL), parent(NULL), bf(0) {}
};

////////////////////////////////////////////////////////////////////////////////
// class tree
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T, typename Alloc = default_allocator<tree_node<key_T, value_T> > >
class tree
{
public:
	typedef tree_node<key_T, value_T>	node_T;
	typedef pair<key_T&, value_T&>		tree_value_T;
	typedef pair<key_T, value_T>		tree_value_insert_T;
	class	iterator;

public:
	tree(): m_pRoot(NULL), m_size(0), m_is_avl(true)  {};
	virtual ~tree() { clear(); };

	//stl兼容接口
	pair<iterator, bool>	insert	(const tree_value_insert_T& val);
	iterator				begin	()	const	{ return iterator(_left_most(m_pRoot));	}
	iterator				end		()	const	{ return iterator(NULL);				}
	iterator				rbegin	()	const	{ return iterator(_right_most(m_pRoot));}
	iterator				rend	()	const	{ return iterator(NULL);				}
	void					clear	()			{ _free_node_recursion(m_pRoot); m_pRoot = NULL;	}
	bool					empty	() const	{ return m_pRoot == NULL;				}
	void					erase	(iterator where)		{ _remove(where.p->key); }	//TODO 返回被删除节点的下一个节点
	void					erase	(const key_T& key)		{ _remove(key); _checkbf(m_pRoot); }					//TODO 返回被删除节点的下一个节点
	iterator				find	(const key_T& key) const	{ return iterator(find_node(key)); }
	int						count	(const key_T& key) const { return find(key) == end() ? 0 : 1; }
	int						size	() const { return m_size; }
	value_T&				operator[](const key_T& key);
	const value_T&			operator[](const key_T& key) const;
	
	//非stl兼容接口
	iterator	add			(const key_T& key, const value_T& value)	{ return iterator(add_node(key, value)); _checkbf(m_pRoot); }
	node_T*		add_node	(const key_T& key, const value_T& value);	//Add 返回新增加的节点
	node_T*		find_node	(const key_T& key) const;
	value_T&	find_value	(const key_T& key) const;
	node_T*		root		() const { return m_pRoot; }
	int			height		() const { return height(m_pRoot); }
	int			height		(const node_T* const pNode) const;
	void		set_is_avl	(bool avl) { m_is_avl = avl; }
	bool		is_avl		()	const { return m_is_avl; }

private:
	void		_checkbf	(node_T* node) const;

	//Copy
	//TODO COPY

public:
	class iterator
	{
	public:
		node_T* p;
		tree_value_T* _pair;
		
		iterator	() : p (NULL), _pair(NULL) {}
		iterator	(node_T* pNode) : _pair(NULL) { p = pNode; }
		~iterator	() { if (_pair) delete _pair; }

		iterator&			operator++();
		iterator&			operator++(const int) { return ++(*this); }
		iterator&			operator--();
		iterator&			operator--(const int) { return --(*this); }
		bool				operator==(const iterator& other) const { return this->p == other.p; }
		bool				operator!=(const iterator& other) const { return this->p != other.p; }
		tree_value_T		operator*() { assert(NULL != p); return tree_value_T(p->key, p->value); }
		const tree_value_T	operator*() const { assert(NULL != p); return tree_value_T(p->key, p->value); }
		tree_value_T*		operator->() 
		{ 
			assert(NULL != p); 
			if (NULL != _pair)
				delete _pair;
			_pair = new tree_value_T(p->key, p->value);
			return _pair;
		}
		const tree_value_T* operator->() const
		{ 
			assert(NULL != p); 
			if (NULL != _pair)
				delete _pair;
			_pair = new tree_value_T(p->key, p->value);
			return _pair;
		}
	};

private:
	//插入删除
	node_T*	_insert				(const key_T& key, const value_T& elem, node_T* pPosition, const TREE_CHILD_TYPE& child_type);
	node_T*	_insert_recursion	(const key_T& key, const value_T& elem, node_T* pPosition, bool& needChangeBlanceFactor);
	node_T*	_insert_recursion_not_avl(const key_T& key, const value_T& elem, node_T* pPosition);
	void	_remove				(const key_T& key);
	void	_free_node			(node_T* pNode);
	void	_free_node_recursion(node_T* pNode);

	//旋转函数
	int		_left_rotate		(node_T* const oldRoot);	//旋转函数，返回值为旋转后高度变化，只能为-1或0，即树的高度减1或者树的高度不变
	int		_right_rotate		(node_T* const oldRoot);
	int		_left_right_rotate	(node_T* const oldRoot);
	int		_right_left_rotate	(node_T* const oldRoot);

	//最左侧节点与最右侧节点
	node_T*	_left_most			(node_T* pNode) const	{ if (NULL != pNode && NULL != pNode->left)	return _left_most(pNode->left);		else return pNode;	}
	node_T*	_right_most			(node_T* pNode) const	{ if (NULL != pNode && NULL != pNode->right) return _right_most(pNode->right);	else return pNode;	}

	void	_adjust_remove_node	(node_T* pNode, int bfChanged);
	void	_attach_node		(node_T* pParent, node_T* pChild, const TREE_CHILD_TYPE childType);
	TREE_CHILD_TYPE _get_child_type(const node_T* const pNode) const; //返回Node是Node-parent的左子节点还是右子节点
	void	_swap_node			(node_T* n1, node_T* n2);
	
private:
	node_T*	m_pRoot;
	int		m_size;
	bool	m_is_avl;
};


////////////////////////////////////////////////////////////////////////////////
// tree实现
////////////////////////////////////////////////////////////////////////////////
template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>* tree<key_T, value_T, Alloc>::add_node(const key_T& key, const value_T& value)
{ 
	if (NULL != m_pRoot)
	{	
		if (is_avl())
		{
			bool needChangeBlanceFactor = false;
			return _insert_recursion(key, value, m_pRoot, needChangeBlanceFactor);
		}
		else
		{
			return _insert_recursion_not_avl(key, value, m_pRoot);
		}
	}
	else
	{
		m_pRoot = Alloc::alloc();
		m_pRoot->key	=	key;
		m_pRoot->value	=	value;
		m_size++;
		return m_pRoot;
	}
}

////////////////////////////////////////////////////////////
//	Remove实现：
//	找到需要删除的节点，然后根据该节点是否包含子节点来分别处理
//	1.如果该节点不包含子节点，那么直接删除即可
//	2.如果该节点包含左节点，但不包含右节点，则用左节点替代该节点
//	3.第2条反之亦然
//	4.如果该节点同时包含左右节点，那么要递归的找到“该节点的左节点的最靠右的叶子节点”rightMost，
//	  然后用rightMost替换该节点，最后删除该节点
//	5.将被删除节点的父节点传递给adjustRemoveNode，做avl平衡
////////////////////////////////////////////////////////////
template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_remove(const key_T& key)
{
	node_T* pRemove = find_node(key);
	if (NULL == pRemove) //没有找到需要删除的节点
		return;

	node_T*			pRemoveParent	= NULL;
	TREE_CHILD_TYPE	bfChanged		= TREE_CHILD_TYPE_INVALID;

	//step2.删除该节点
	if (NULL != pRemove->left && NULL != pRemove->right)
	{
		//step2.1 如果左右子节点均不为空，那么交换法删除该节点
		node_T* pRightMost = _right_most(pRemove->left);

		//pRightMost至少可以是pRemove->left，而pRemove->left != NULL，因此pRightMost不可能为NULL
		assert(NULL != pRightMost);	

		bool adjacent = (pRightMost->parent == pRemove);
		assert(pRightMost->parent);

		//如果pRemove和pRightMost是相邻节点，需要特殊处理
		if (adjacent)
		{
			//正常情况下，rightMost是个右侧节点
			//但是当删除节点的左子树只有一个节点时，这个节点就是rightMost，
			//此时该节点是pRemove的左子节点，需要特殊处理
			pRemoveParent = pRightMost;
			bfChanged = TREE_CHILD_TYPE_LEFT;

			_swap_node(pRemove, pRightMost);

			assert(pRightMost->left);

			//pRightMost->left = NULL;


			if (pRemove->left != NULL)
			{
				pRemove->parent->left = pRemove->left;
				pRemove->left->parent = pRemove->parent;
			}
			else
				pRemove->parent->left = NULL;
		}
		else
		{
			pRemoveParent = pRightMost->parent;
			bfChanged = TREE_CHILD_TYPE_RIGHT;

			_swap_node(pRemove, pRightMost);

			assert(pRemove->parent->right);
			//pRemove->parent->right = NULL;

			if (pRemove->left != NULL)
			{
				pRemove->parent->right = pRemove->left;
				pRemove->left->parent = pRemove->parent;
			}
			else
				pRemove->parent->right = NULL;
		}

		//删除RightMost节点
		_free_node(pRemove);
	}
	else if (NULL == pRemove->left)
	{
		//step2.2 如果左节点为空，则用右子节点代替当前节点
		bfChanged = _get_child_type(pRemove);
		_attach_node(pRemove->parent, pRemove->right, bfChanged);

		//保存需要调整平衡因子的父节点
		pRemoveParent = pRemove->parent;

		_free_node(pRemove);
	}
	else if (NULL == pRemove->right)
	{
		//step2.3 如果右节点为空，则用左子节点代替当前节点
		bfChanged = _get_child_type(pRemove);
		_attach_node(pRemove->parent, pRemove->left, bfChanged);

		//保存需要调整平衡因子的父节点
		pRemoveParent = pRemove->parent;

		_free_node(pRemove);
	}

	//step3 调整平衡因子
	if (is_avl() && NULL != pRemoveParent)
	{
		_adjust_remove_node(pRemoveParent, bfChanged);
	}
}

template<typename key_T, typename value_T, typename Alloc>
pair<typename tree<key_T, value_T, Alloc>::iterator, bool> tree<key_T, value_T, Alloc>::insert(const tree_value_insert_T& val) 
{  
	iterator findKey = find(val.first);
	if (findKey != end())
	{
		return pair<iterator, bool>(findKey, false);
	}
	iterator newNode = add_node(val.first, val.second);
	return pair<iterator, bool>(newNode, true);
}

template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>* tree<key_T, value_T, Alloc>::find_node(const key_T& key) const
{
	node_T* pNode = m_pRoot;
	node_T* pRemove = NULL;

	//step1.找到需要删除的节点
	while (pNode)
	{
		if (key < pNode->key)
			pNode = pNode->left;
		else if (pNode->key < key)
			pNode = pNode->right;
		else// if (pNode->key == key)
		{
			pRemove = pNode;
			break;
		}
	}
	return pRemove;
}

template<typename key_T, typename value_T, typename Alloc>
value_T& tree<key_T, value_T, Alloc>::find_value(const key_T& key) const
{
	node_T* pFoundNode = find_node(key);
	if (NULL == pFoundNode)
	{
		//return NULL;
		assert(0);
		throw 1;
	}
	return pFoundNode->value;
}


//调整一个被删除节点的父节点
template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_adjust_remove_node(node_T* pNode, int bfChanged)
{
	if (NULL == pNode)
	{
		assert(false);
		return;
	}

	node_T* pOldParent = pNode->parent;

	//被删除节点的父节点原来的bf为0，删除单个节点后，bf为1或者-1，因此无需进行调整，结束递归
	if (0 == pNode->bf)
	{
		pNode->bf += bfChanged;
		
		//结束递归
		return;
	} 
	//被删除节点的父节点原来的bf为1，则如果删除左侧节点后可能导致不平衡
	//同时还要递归的调整该节点的父节点
	else if (1 == pNode->bf)
	{
		pNode->bf += bfChanged;
		int newType = _get_child_type(pNode);
		int heightChanged = (bfChanged != 0);
		if (pNode->bf >= 2)
		{
			//需要进行旋转
			if (pNode->right->bf >= 0)
			{
				heightChanged = _left_rotate(pNode);
			}
			else
			{
				heightChanged = _right_left_rotate(pNode);
			}
		}
		if (0 != heightChanged && NULL != pOldParent)
		{
			_adjust_remove_node(pOldParent, newType);
		}
		else
		{
			return;
		}
	} 
	//被删除节点的父节点原来的bf为-1，则如果删除右侧节点后可能导致不平衡
	//同时还要递归的调整该节点的父节点
	else if (-1 == pNode->bf)
	{
		pNode->bf += bfChanged;
		int newType = _get_child_type(pNode);
		int heightChanged = (bfChanged != 0);
		if (pNode->bf <= -2)
		{
			//需要进行旋转
			if (pNode->left->bf <= 0)
			{
				heightChanged = _right_rotate(pNode);
			}
			else
			{
				heightChanged = _left_right_rotate(pNode);
			}
		}
		if (0 != heightChanged && NULL != pOldParent)
		{
			_adjust_remove_node(pOldParent, newType);
		}
		else
		{
			return;
		}
	}
	 //如果被删除节点父节点的bf不为1,-1或0，表示原来树已经不平衡，出错
	else
	{
		assert(false);
	}
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_right_rotate(node_T* const oldRoot)
{
	/*
	//				O (oldRoot) bf = -2
	//		h+2	   / \
	//	(newRoot) O   h
	//			 / \
	//		   h+1  a

	其中	a <= h + 1;

	// 旋转后：
	//		O (newRoot) 
	//	   / \
	//   h+1  O (oldRoot)   
	//		 / \
	//	    a   h

	当 newRoot->bf == 0 （即a = h + 1）时，旋转后高度不变
	否则旋转后高度减1
	*/

	//TODO 指针安全判断
	//TODO 处理parent指针
	assert(oldRoot);
	assert(oldRoot->left);

	//oldRoot的左节点作为该子树的新的顶点root
	node_T* const newRoot = oldRoot->left;

	//保存新root的旋转前的右节点
	node_T* const oldRight = newRoot->right;	

	//保存原来oldRoot的parent节点
	node_T* const oldParent = oldRoot->parent;

	//将oldRoot旋转到newRoot的右节点
	newRoot->right = oldRoot;
	oldRoot->parent = newRoot;
	newRoot->parent = oldParent;
	if (oldParent)
	{
		if (oldParent->left == oldRoot)
		{
			oldParent->left = newRoot;
		}
		else if (oldParent->right == oldRoot)
		{
			oldParent->right = newRoot;
		}
		else
		{
			assert(false);
		}
	}
	if (oldRoot == m_pRoot)
	{
		m_pRoot = newRoot;
	}

	//将newRoot的原有右节点移动到oldRoot的左节点
	oldRoot->left = oldRight;
	if (oldRight)
	{	
		oldRight->parent = oldRoot;
	}

	int heightChanged = 0;
	//调整平衡因子，具体分析比较复杂
	if (newRoot->bf == 0)
	{
		newRoot->bf = 1;
		oldRoot->bf = -1;
		heightChanged = 0;
	}
	else
	{
		newRoot->bf = 0;
		oldRoot->bf = 0;
		heightChanged = -1;
	}
	return heightChanged;
}

//返回值为：“以oldRoot所在的位置作为根节点的子树”在旋转之后高度的变化情况
template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_left_rotate(node_T* const oldRoot)
{
	/*
	//		O (oldRoot)
	//	   / \
	//	  口 O (newRoot) 
	//	     / \
	//		口 O  
	// 旋转后：
	//				 O (newRoot) 
	//				/ \
	//	(oldRoot)  O   O  
	//			  / \
	//           口 口

	关于平衡因子变化和高度变化的解释参见rightRotate

	*/

	//TODO 指针安全判断
	//TODO 处理parent指针
	assert(oldRoot);
	assert(oldRoot->right);

	//oldRoot的右节点作为该子树的新的顶点root
	node_T* const newRoot = oldRoot->right;

	//保存新root的旋转前的左节点
	node_T* const oldLeft = newRoot->left;	

	//保存原来oldRoot的parent节点
	node_T* const oldParent = oldRoot->parent;

	//将oldRoot旋转到newRoot的左节点
	newRoot->left = oldRoot;
	oldRoot->parent = newRoot;
	newRoot->parent = oldParent;
	if (oldParent)
	{
		if (oldParent->left == oldRoot)
		{
			oldParent->left = newRoot;
		}
		else if (oldParent->right == oldRoot)
		{
			oldParent->right = newRoot;
		}
		else
		{
			assert(false);
		}
	}
	if (oldRoot == m_pRoot)
	{
		m_pRoot = newRoot;
	}

	//将newRoot的原有左节点移动到oldRoot的右节点
	oldRoot->right = oldLeft;
	if (oldLeft)
	{	
		oldLeft->parent = oldRoot;
	}

	int heightChanged = 0;
	//调整平衡因子，具体分析比较复杂
	if (newRoot->bf == 0)
	{
		newRoot->bf = -1;
		oldRoot->bf = 1;
		heightChanged = 0;
	}
	else
	{
		newRoot->bf = 0;
		oldRoot->bf = 0;
		heightChanged = -1;
	}
	return heightChanged;
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_left_right_rotate(node_T* const oldRoot)
{
	/*
	//					O  (oldRoot)
	//				   / \
    //	   (oldLeft)  O   h
	//				 / \
	//				h	O	oldLeftRightBlanceFactor == 1
	//				   / \
	//				  h-1 h
	// 或者
	//					O (oldRoot)
	//				   / \
	//	   (oldLeft)  O   h
	//				 / \
	//				h	O	oldLeftRightBlanceFactor == -1
	//				   / \
	//				  h  h-1

	//					O  (oldRoot)
	//				   / \
    //	   (oldLeft)  O   h
	//				 / \
	//				h	O	oldLeftRightBlanceFactor == 1
	//				   / \
	//				  a   b

	其中	a = h 或 h-1
			b = h 或 h-1
			当插入操作时，不可能同时出现a = h, b = h，必然是a和b的值在h和h-1中各取一个
			当删除操作时，可能同时出现 a = h, b = h, 此时要注意重新评估平衡因子
			例外情况，当 a == b == 0的时候，无论插入和删除都可能出现 a == b

	旋转后：

	第一步:
					O  (oldRoot)
				   / \
			      O   h  oldLeftRightBlanceFactor == 1
				 / \
	(oldLeft)   O	b	
			   / \   
			  h	  a 

	 第二步：
			        O  oldLeftRightBlanceFactor == 1
				  /   \
	(oldLeft)   O		O	(oldRoot)
			   / \	   / \
			  h	  a    b  h
	

	*/

	assert(oldRoot);
	assert(oldRoot->left);

	//保存原状态节点，用于旋转后调整bf
	node_T* oldLeft				= oldRoot->left;
	node_T* oldLeftRight			= oldRoot->left->right;
	const int oldLeftRightBlanceFactor	= oldLeftRight->bf;
	assert(oldLeftRight);
	//当 a == b == 0 的时候，可能出现 oldLeftRightBlanceFactor == 0
	//assert(oldLeftRightBlanceFactor == 1 || oldLeftRightBlanceFactor == -1);
	
	_left_rotate(oldRoot->left);
	_right_rotate(oldRoot);

	//调整平衡因子
	if (oldLeftRightBlanceFactor == 1)
	{
		oldLeft->bf = -1;
		oldRoot->bf = 0;
		oldLeftRight->bf = 0;
	}
	else if (oldLeftRightBlanceFactor == -1)
	{
		oldLeft->bf = 0;
		oldRoot->bf = 1;
		oldLeftRight->bf = 0;
	}
	else if (oldLeftRightBlanceFactor == 0)
	{
		//caolei 2010.09.29
		//正常的添加逻辑不会有oldLeftRightBlacneFactor = 0的情况
		//正常的删除逻辑在oldLeftRightBlacneFactor = 0 的时候会单独采取左旋转或者右旋转，
		//所以目前不会执行这里的逻辑
		//caolei 2011.02.12当 a == b == 0 的时候，可能出现 oldLeftRightBlanceFactor == 0
		//assert(false);

		oldLeft->bf = 0;
		oldRoot->bf = 0;
		oldLeftRight->bf = 0;
	}
	//调整后子树的高度必然降低1
	int heightChange = -1;
	return heightChange;
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::_right_left_rotate(node_T* const oldRoot)
{
	/*
	//									O (oldRoot)
	//								   / \
	//								  h   O (oldRight)
	//									 / \
	//	oldLeftRightBlanceFactor = -1	O   h
	//								   / \
	//								  h	 h-1
	//或者
	//									O (oldRoot)
	//								   / \
	//								  h   O (oldRight)
	//									 / \
	//	oldLeftRightBlanceFactor = 1	O   h
	//								   / \
	//								 h-1  h

	具体过程可以参见leftRightRotate函数的注释
	*/

	assert(oldRoot);
	assert(oldRoot->right);

	//保存原状态节点，用于旋转后调整bf
	node_T* oldRight = oldRoot->right;
	node_T* oldRightLeft = oldRoot->right->left;
	const int oldRightLeftBlanceFactor = oldRightLeft->bf;
	assert(oldRightLeft);
	//caolei 2011.02.12当 a == b == 0 的时候，可能出现 oldRightLeftBlanceFactor == 0
	//assert(oldRightLeftBlanceFactor == 1 || oldRightLeftBlanceFactor == -1);

	_right_rotate(oldRoot->right);
	_left_rotate(oldRoot);

	//调整平衡因子
	if (oldRightLeftBlanceFactor == 1)
	{
		oldRight->bf = 0;
		oldRoot->bf = -1;
		oldRightLeft->bf = 0;
	}
	else if (oldRightLeftBlanceFactor == -1)
	{
		oldRight->bf = 1;
		oldRoot->bf = 0;
		oldRightLeft->bf = 0;
	}
	else if (oldRightLeftBlanceFactor == 0)
	{
		//caolei 2010.09.29
		//正常的添加逻辑不会有oldRightLeftBlanceFactor = 0的情况
		//正常的删除逻辑在oldRightLeftBlanceFactor = 0 的时候会单独采取左旋转或者右旋转，
		//所以目前不会执行这里的逻辑
		//caolei 2011.02.12当 a == b == 0 的时候，可能出现 oldRightLeftBlanceFactor == 0
		//assert(false);

		oldRight->bf = 0;
		oldRoot->bf = 0;
		oldRightLeft->bf = 0;
	}
	//调整后子树的高度必然降低1
	int heightChange = -1;
	return heightChange;
}

template<typename key_T, typename value_T, typename Alloc>
int tree<key_T, value_T, Alloc>::height(const node_T* const pNode) const
{
	if (!pNode)
		return 0;

	int _height = 1;
	int rightHeight = 0;
	int leftHeight = 0;
	if (pNode->left)
	{
		leftHeight = height(pNode->left);
	}

	if (pNode->right)
	{
		rightHeight = height(pNode->right);
	}
	_height += leftHeight > rightHeight ? leftHeight : rightHeight;

	return _height;
}

template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_free_node(node_T* pNode)
{
	if (NULL == pNode)
		return;

	m_size--;
	Alloc::free(pNode);
}

template<typename Key, typename Value, typename Alloc>
void tree<Key, Value, Alloc>::_free_node_recursion(node_T* pNode)
{
	if (NULL == pNode)
		return;
	if (pNode->left)
		_free_node_recursion(pNode->left);
	if (pNode->right)
		_free_node_recursion(pNode->right);
	m_size--;
	Alloc::free(pNode);
}

template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>*  tree<key_T, value_T, Alloc>::_insert(const key_T& key, const value_T& value, node_T* pPosition, const TREE_CHILD_TYPE& TreeChildType)
{
	if (NULL == pPosition)
	{
		pPosition = m_pRoot;
	}
	if (pPosition)
	{
		node_T* pNewNode = Alloc::alloc();
		pNewNode->key	= key;
		pNewNode->value = value;
		pNewNode->left = NULL;
		pNewNode->right = NULL;
		pNewNode->parent = pPosition;
		if (TreeChildType == TREE_CHILD_TYPE_LEFT)
			pPosition->left = pNewNode;
		else if (TreeChildType == TREE_CHILD_TYPE_RIGHT)
			pPosition->right = pNewNode;
		m_size++;
		return pNewNode;
	}
	return NULL;
}

//递归插入
template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>*  tree<key_T, value_T, Alloc>::_insert_recursion(const key_T& key, const value_T& elem, node_T* pCurrent, bool& needChangeBlanceFactor)
{
	//注意，当key值相等的时候，插入到右侧！
	if (key < pCurrent->key)
	{
		//左侧插入
		if (NULL != pCurrent->left)
		{
			node_T* pNewNode = _insert_recursion(key, elem, pCurrent->left, needChangeBlanceFactor);
			if (needChangeBlanceFactor)
			{
				pCurrent->bf--;	//左子树增高，bf减少（由于bf定义是“右子树高度”减“左子树高度”）

				if (pCurrent->bf == -1) // 原来的bf是 0, 左边和右边一样高，现在左边加了一个(bf = bf - 1)，bf变为-1了，pCurrent的整体高度增加了1，所以 needChangeBlanceFactor = true 
					needChangeBlanceFactor = true;
				else if (pCurrent->bf == 0) // 原来的bf是 1, 右边比左边高度多1，现在左边加了一个(bf = bf - 1)，bf变为0了，但是pCurrent的整体高度没变，所以 needChangeBlanceFactor = false
					needChangeBlanceFactor = false;
			}

			if (pCurrent->bf == -2)
			{
				if (key < pCurrent->left->key)
					//左左情况，进行单次右旋转
					_right_rotate(pCurrent);
				else
					//左右情况，进行左右旋转
					_left_right_rotate(pCurrent);

				//“在添加节点的情况下”，旋转后树的高度必然减1，这和导致旋转的树的高度加1互相抵消了，
				//因此向上的所有祖先节点都无需修改平衡因子
				needChangeBlanceFactor = false;
			}

			return pNewNode;
		}
		else
		{
			if (NULL == pCurrent->right)
				//插入节点没有右子树，在左侧插入后会导致高度变化，从而导致平衡因子变化
				needChangeBlanceFactor = true;
			else
				//插入节点已经有右子树，在左侧插入后不会导致高度变化，因此不用修改平衡因子
				needChangeBlanceFactor = false;

			pCurrent->bf--;
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_LEFT);
		}
	}
	else if (pCurrent->key < key)
	{
		//右侧插入
		if (NULL != pCurrent->right)
		{	
			node_T* pNewNode = _insert_recursion(key, elem, pCurrent->right, needChangeBlanceFactor);

			if (needChangeBlanceFactor)
			{
				pCurrent->bf++;
				if (pCurrent->bf == 1)  // 原来的bf是 0, 左边和右边一样高，现在右边加了一个(bf = bf + 1)，bf变为1了，pCurrent的整体高度增加了1，所以 needChangeBlanceFactor = true 
					needChangeBlanceFactor = true;
				else if (pCurrent->bf == 0) // 原来的bf是 -1, 左边比右边高度多1，现在右边加了一个(bf = bf + 1)，bf变为0了，但是pCurrent的整体高度没变，所以 needChangeBlanceFactor = false
					needChangeBlanceFactor = false;
			}
			if (pCurrent->bf == 2)
			{
				if (key < pCurrent->right->key)
				{
					//右左情况，进行右左旋转
					_right_left_rotate(pCurrent);
				}
				else
				{
					//右右情况，进行单次左旋转
					_left_rotate(pCurrent);
				}
				//“在添加节点的情况下”，旋转后树的高度必然减1，这和导致旋转的树的高度加1互相抵消了，
				//因此向上的所有祖先节点都无需修改平衡因子
				needChangeBlanceFactor = false;
			}

			return pNewNode;
		}
		else
		{
			if (NULL == pCurrent->left)
			{
				//插入节点没有左子树，在右侧插入后会导致高度变化，从而导致平衡因子变化
				needChangeBlanceFactor = true;
			}
			else
			{
				//插入节点已经有左子树，在右侧插入后不会导致高度变化，因此不用修改平衡因子
				needChangeBlanceFactor = false;
			}
			pCurrent->bf++;
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_RIGHT);
		}
	}
	else // if (pCurrent->key == key)
	{
		//已有重复值了
		assert(false);
		return NULL;
	}
}


//递归插入，但不进行avl旋转
template<typename key_T, typename value_T, typename Alloc>
tree_node<key_T, value_T>*  tree<key_T, value_T, Alloc>::_insert_recursion_not_avl(const key_T& key, const value_T& elem, node_T* pCurrent)
{
	if (key < pCurrent->key)
	{
		//左侧插入
		if (NULL != pCurrent->left)
			return _insert_recursion_not_avl(key, elem, pCurrent->left);
		else
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_LEFT);
	}
	else if (pCurrent->key < key)
	{
		//右侧插入
		if (NULL != pCurrent->right)
			return _insert_recursion_not_avl(key, elem, pCurrent->right);
		else
			return _insert(key, elem, pCurrent, TREE_CHILD_TYPE_RIGHT);
	}
	else // if (pCurrent->key == key)
	{
		//已有重复值了
		assert(false);
		return NULL;
	}
}

template<typename key_T, typename value_T, typename Alloc>
TREE_CHILD_TYPE tree<key_T, value_T, Alloc>::_get_child_type(const node_T* const pNode) const
{
	if (NULL == pNode->parent)
	{
		assert(pNode == m_pRoot);
		return TREE_CHILD_TYPE_ROOT;
	}
	else if (pNode->parent->left == pNode)
	{
		return TREE_CHILD_TYPE_LEFT;
	}
	else if (pNode->parent->right == pNode)
	{
		return TREE_CHILD_TYPE_RIGHT;
	}

	return TREE_CHILD_TYPE_INVALID;
}

template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_attach_node(node_T* pParent, node_T* pChild, const TREE_CHILD_TYPE childType)
{
	//处理parent节点
	if (pParent == NULL) //将pChild attach到root
		m_pRoot = pChild;
	else
	{
		if (childType == TREE_CHILD_TYPE_LEFT)
			pParent->left = pChild;
		else if (childType == TREE_CHILD_TYPE_RIGHT)
			pParent->right = pChild;
		else
			assert(false);
	}

	//处理child节点
	if (NULL != pChild)
		pChild->parent = pParent;
}

template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_swap_node(node_T* _n1, node_T* _n2)
{
	if (_n1 == _n2)
		return;
	assert(_n1->key < _n2->key || _n2->key < _n1->key);

	//make sure n1 is always on the right side of n2
	node_T* n1 = _n1;
	node_T* n2 = _n2;
	if (_n2->key < _n1->key)
	{
		n1 = _n2;
		n2 = _n1;
	}

	node_T* const n1_left	= n1->left;
	node_T* const n1_right	= n1->right;
	node_T* const n1_parent	= n1->parent;
	int		const n1_bf		= n1->bf;

	node_T* const n2_left	= n2->left;
	node_T* const n2_right	= n2->right;
	node_T* const n2_parent	= n2->parent;
	int		const n2_bf		= n2->bf;

	//swap n1 and n2
	n1->left		= n2_left;
	n1->right	= n2_right;
	n1->parent	= n2_parent;

	n2->left		= n1_left;
	n2->right	= n1_right;
	n2->parent	= n1_parent;

	//if n1 and n2 are adjacent, update special
	if (n1_right == n2)
	{
		n1->parent	= n2;
		n2->right	= n1;
	}
	else if (n2_left == n1)
	{
		n1->left		= n2;
		n2->parent	= n1;
	}

	//update n1.left/right and n2.left/right
	if (NULL != n1_left)
		n1_left->parent = n2;
	if (NULL != n1_right && n1_right != n2)	//only update n1_right when n1 and n2 are NOT adjacent  
		n1_right->parent = n2;
	if (NULL != n2_left && n2_left != n1)	//only update n2_left when n1 and n2 are NOT adjacent
		n2_left->parent = n1;
	if (NULL != n2_right)
		n2_right->parent = n1;

	// update n1.parent
	if (NULL != n1_parent && n2_left != n1) // only update n1.parent when n1.parent is NOT n2
	{
		if (n1_parent->left == n1)
			n1_parent->left = n2;
		else if (n1_parent->right = n1)
			n1_parent->right = n2;
		else
			assert(false);
	}
	// update n2.parent
	if (NULL != n2_parent && n1_right != n2) // only update n2.parent when n2.parent is NOT n1
	{
		if (n2_parent->left == n2)
			n2_parent->left = n1;
		else if (n2_parent->right = n2)
			n2_parent->right = n1;
		else
			assert(false);
	}

	n1->bf = n2_bf;
	n2->bf = n1_bf;

	// update root
	if (m_pRoot == n1)
		m_pRoot = n2;
	else if (m_pRoot == n2)
		m_pRoot = n1;
}

template<typename key_T, typename value_T, typename Alloc>
value_T& tree<key_T, value_T, Alloc>::operator[](const key_T& key) 
{ 
	if (count(key))
	{
		return find_value(key);
	}
	value_T v = value_T();
	pair<iterator, bool> insertResult = insert(make_pair(key, v));
	if (false == insertResult.second || NULL == insertResult.first.p)
	{
		assert(0);
		throw 1;
	}
	return insertResult.first.p->value;
}

template<typename key_T, typename value_T, typename Alloc>
const value_T& tree<key_T, value_T, Alloc>::operator[](const key_T& key) const
{
	return (*const_cast<tree<key_T, value_T, Alloc>*>(this))[key];
}



template<typename key_T, typename value_T, typename Alloc>
void tree<key_T, value_T, Alloc>::_checkbf(node_T* node) const
{
#ifdef SCL_DEBUG
	if (!is_avl())
		return;
	if (NULL == node)
		return;

	_checkbf(node->left);
	_checkbf(node->right);

	int lheight = height(node->left);
	int rheight = height(node->right);
	int bf = rheight - lheight;

	assert(bf == node->bf);
#else
	node = node;
#endif
}

template<typename key_T, typename value_T, typename Alloc>
typename tree<key_T, value_T, Alloc>::iterator& tree<key_T, value_T, Alloc>::iterator::operator++() 
{	
	if (NULL == p)
		return *this;

	node_T* next = NULL;
	if (NULL != p->right)
	{
		//如果有右子节点，则找到右子节点的子结点中最靠左的节点(即最大节点)
		next = p->right;
		while (next->left)
		{
			next = next->left;
		}
		p = next;
	}
	else if (NULL == p->right)
	{	
		//如果右子节点为空，则追溯祖先节点，直到找到祖先节点左节点为next的节点(next是递归查找节点)
		next = p;
		while (next->parent && next->parent->right == next)
		{
			next = next->parent;
		}
		p = next->parent;
	}
	return *this; 
}

template<typename key_T, typename value_T, typename Alloc>
typename tree<key_T, value_T, Alloc>::iterator& tree<key_T, value_T, Alloc>::iterator::operator--()
{	
	if (NULL == p)
		return *this;

	node_T* next = NULL;
	if (NULL != p->left)
	{
		//如果有左子节点，则找到左子节点的子结点中最靠右的节点(即最小节点)
		next = p->left;
		while (next->right)
		{
			next = next->right;
		}
		p = next;
	}
	else if (NULL == p->left)
	{	
		//如果左子节点为空，则追溯祖先节点，直到找到祖先节点右节点为next的节点(next是递归查找节点)
		next = p;
		while (next->parent && next->parent->left == next)
		{
			next = next->parent;
		}
		p = next->parent;
	}
	return *this; 
}

} //namespace scl
