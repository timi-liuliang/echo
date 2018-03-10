//**************************************************************************/
// Copyright (c) 1998-2006 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Automatic pointer supporting resource-acquisition is 
//    initialization technique.
// AUTHOR: Nicolas Desjardins
// DATE: 2006/03/20 
//***************************************************************************/

#pragma once

#ifndef NULL
#define NULL 0
#endif

namespace MaxSDK
{

/*!
 * \brief Helper class used to implement destructive copy semantics and allow
 * for AutoPtrs to be used as return values.
 *
 * You don't really need to know this since the compiler will use 
 * AutoPtrRefs silently and everything will just work out the way you'd 
 * expect, but here's an explanation of why AutoPtrRef is needed.
 *
 * Values returned by functions are temporaries (unnamed objects that you 
 * never actually see in your source code).  These then get copied into 
 * whatever variable is set up to receive them. (See More Effective C++, 
 * items 19 and 20.)
 *
 * Since these temporaries are inaccessible to normal source code, it is
 * usually a design error for them to be modified, so the compiler makes
 * them const.  They can't be bound to non-const references, which are
 * needed for the AutoPtr copy constructor and assignment operator.
 * Attempting to return and receive an AutoPtr from a function would then
 * generate a compiler error.
 * 
 * The AutoPtrRef and cast operations below exist to allow the compiler 
 * to implicitly convert from the const AutoPtr returned from a function 
 * to an AutoPtrRef, which can then be assigned to the receiving AutoPtr 
 * using the AutoPtr(AutoPtrRef&) constructor.
 *
 * (Okay, there's a bit more going on to get around the double-conversion
 * rule, which Herb Sutter explains as: "[A] subtle interaction in the
 * overload resolution rules allows the selection of single viable
 * constructor function, followed by the manufacture of just the right
 * temporary which lets us slip in a second user-defined conversion (to
 * auto_ptr_ref<Z>) that is normally forbidden by 12.3/4." From
 * the thread "What is auto_ptr_ref for?" on comp.lang.c++.moderated.)
 */
template<typename OtherType> struct AutoPtrRef
{
	OtherType *mPtr;
	AutoPtrRef(OtherType *p)  : mPtr(p) {}		
};


/**
 * \brief DestructorPolicy template for pointer to single object types.
 */
template <typename Type> class SinglePointerDestructor
{
public:
	static void Delete(Type *ptr) 
	{ 
		delete ptr; 
	}
};

/*!
 * \brief Automatic memory-ownership pointer supporting "resource acquisition
 * is initialization."
 *
 * This is a standardized implementation of std::auto_ptr from the STL, allowing
 * us to portably include AutoPtrs in the 3ds Max SDK without forcing a compiler
 * or runtime dll version on third-party developers.  std::auto_ptrs are 
 * described in your favourite STL documentation, in Scott Meyers's More 
 * Effective C++ and on pp. 367-368 in Bjarne Stroustrup's The C++ Programming 
 * Language.  This implementation is based on Stroustrup and Meyers.
 *
 * AutoPtrs grant the same mechanics as a regular pointer, so they can be
 * created, assigned and dereferenced much in the same way as a plain old 
 * pointer.  Additionally, AutoPtrs provide "resource acquisition is 
 * initialization" and automatic memory ownership management in two ways:
 * <ol>
 * <li>The object pointed to by the AutoPtr will be implicitly deleted when 
 * the AutoPtr goes out of scope</li>
 * <li>Through "destructive copying" that causes a copied AutoPtr to surrender 
 * ownership to the pointed to object.</li>
 * </ol>
 *
 * This is a very useful technique for making the ownership semantics of a 
 * function very clear.  A function that takes an AutoPtr to an object as a 
 * parameter is stating that it assumes ownership of the pointed to object and
 * that the caller must surrender ownership.  A function that returns an
 * AutoPtr states that the caller must assume ownership of the pointed-to 
 * object.
 *
 * AutoPtrs help prevent memory leaks and lead to exception safe code by 
 * guaranteeing that dynamically allocated objects will be deleted when the
 * AutoPtr goes out of scope.  The following is unsafe code:
 * \code
 * void foo()
 * {
 *    A *a = new A();
 *    // ...
 *    delete a;
 * }
 * \endcode
 *
 * Will leak if any exception is thrown in the "..." part.  Replacing with
 * an AutoPtr guarantees that the memory is cleaned up, since leaving the
 * function through an exception will cause the AutoPtr to go out of scope.
 * \code
 * void foo()
 * {
 *    AutoPtr a(new A());
 *    // ...
 * }
 * \endcode
 *
 * Since not all objects have the same destruction requirements, the AutoPtr 
 * template can be parameterized to support creating various types 
 * of AutoPtrs differentiated by their DestructorPolicy. Three standard forms,
 * the default AutoPtr&lt;Type&gt;, ArrayAutoPtr&lt;Type&gt;, and 
 * DeleteThisAutoPtr&lt;Type&gt are available.  AutoPtr's default destructor 
 * policy is for use with a pointer pointing to a single object (one that uses 
 * new and delete).  ArrayAutoPtr is for use with a pointer pointing to an 
 * array (one that uses new [] and delete []).  DeleteThisAutoPtr is for use
 * with Max types requiring clean up through a DeleteThis method.
 *
 * The two standard implementations are probably enough to cover most cases.
 * Other derivations would be used for, say, pointers using malloc and free or
 * to hide the destructor implementation details by using an explicitly typed, 
 * non-templated DestructorPolicy to avoid having to inline the Delete method.
 * This is necessary to implement explicit template instantiation for a forward
 * declared type.  The standard templates can't be explicitly instantiated on a 
 * forward declared type since they require that the destructor be defined.
 *
 * NB: The destructive copy semantics mean that AutoPtrs are 
 * inappropriate for use in any of the STL's container classes. 
 * Classes with data members of type AutoPtr must be non-copyable (derive from 
 * class MaxSDK::Util::Noncopyable), or implement their own copy constructor and 
 * assignment operator where they deep-copy the object being owned by the AutoPtr. 
 *
 * NB2: Finally, be careful about passing AutoPtrs across DLL boundaries for
 * DLLs compiled with different compilers or different memory allocation 
 * schemes.  Objects traversing such boundaries must be deallocated on the same 
 * side as they were allocated.  One way to guarantee this is to use it with objects
 * derived from MaxHeapOperators, or to override a class's delete operator so
 * that the code that actually frees the memory is on the correct side of the DLL boundary.
 */
template <typename Type, typename DestructorPolicy = SinglePointerDestructor<Type> >
class AutoPtr
{
private:
	/*!
	 * \brief This prohibits the casual allocation of AutoPtrs on the heap.
	 *
	 * Trying to do so will result in a compile-time error. Not implemented,
	 * so it will also result in a link-time error if a class somehow has
	 * access to it.
	 */
	void * operator new(size_t size);	
	
	/*!
	 * \brief This prohibits the placement initialization of AutoPtrs on the 
	 * heap. 
	 *
	 * Trying to do so will result in a compile-time error. Not 
	 * implemented, so it will also result in a link-time error if a class 
	 * somehow has access to it.
	 */	
	void * operator new(size_t size, void * placement);
	
	/*!
	 * \brief The underlying plain-old pointer.
	 */
	Type *mPtr;

public:
	/*!
	 * \brief Typedef to make the element type a member of the class.
	 */
	typedef Type element_type;

	/*!
	 * \brief Typedef to make the DestructionPolicy type a member of the class.
	 */
	typedef DestructorPolicy destructor;

	/*!
	 * \brief Construct, assuming ownership of the pointed-to object.
	 *
	 * \param p Plain pointer to an object - this AutoPtr will assume ownership
	 * of that object.
	 */
	explicit AutoPtr(Type *p = NULL)  : mPtr(p) {}

	/*!
	 * \brief Destructor - automatically cleans up the pointed-to object.
	 */
	~AutoPtr()  { DestructorPolicy::Delete(mPtr); }

	/*!
	 * \brief Copy construct, assuming ownership from the source AutoPtr.
	 *
	 * \param a Another AutoPtr.  This newly constructed AutoPtr will take
	 * ownership of a's pointed-to object, and subsequently, a will point to NULL.
	 * AutoPtr a may no longer be used to access the pointed-to object after 
	 * being copied.
	 */
	AutoPtr(AutoPtr &a)  : mPtr(a.Release()) {}

	/*!
	 * \brief Conversion copy constructor.  Assumes ownership of any type 
	 * assignable to Type.
	 *
	 * \param a Another AutoPtr.  This newly constructed AutoPtr will take
	 * ownership of a's pointed-to object, and subsequently, a will point to NULL.
	 * AutoPtr a may no longer be used to access the pointed-to object after 
	 * being copied.
	 */
	template <typename OtherType> AutoPtr(AutoPtr<OtherType, DestructorPolicy> &a) : 
		mPtr(a.Release()) 
	{}

	/*!
	 * \brief Assignment, takes over ownership from the source AutoPtr.  Any
	 * existing pointer in this AutoPtr will be deleted.
	 *
	 * \param a Another AutoPtr.  This newly constructed AutoPtr will take
	 * ownership of a's pointed-to object, and subsequently, a will point to NULL.
	 * AutoPtr a may no longer be used to access the pointed-to object after 
	 * being copied.
	 */
	AutoPtr& operator=(AutoPtr &a) 
	{
		if(Get() != a.Get())
		{
			DestructorPolicy::Delete(mPtr);
			mPtr = a.Release();
		}
		return *this;
	}

	/*!
	 * \brief Conversion assignment, takes over ownership of any type assignable
	 * to type from the source AutoPtr.  Any existing pointer in this AutoPtr
	 * will be deleted.
	 *
	 * \param a Another AutoPtr.  This newly constructed AutoPtr will take
	 * ownership of a's pointed-to object, and subsequently, a will point to NULL.
	 * AutoPtr a may no longer be used to access the pointed-to object after 
	 * being copied.
	 */
	template<typename OtherType> AutoPtr& operator=(AutoPtr<OtherType, DestructorPolicy> &a) 
	{
		if(Get() != a.Get())
		{
			DestructorPolicy::Delete(mPtr);
			mPtr = a.Release();
		}
		return *this;
	}

	/*!
	 * \brief Dereferencing operator - works exactly like a plain pointer's 
	 * operator*.
	 *
	 * \return A reference to the pointed-to object.
	 */
	Type& operator*() const  { return *mPtr; }

	/*!
	 * \brief Pointer-to-member dereferencing operator - works exactly like a 
	 * plain pointer's operator->.
	 *
	 * \return Pointer to member.
	 */
	Type* operator->() const  { return mPtr; }

	/*!
	 * \brief Get the plain pointer back.  This does not affect the ownership.
	 *
	 * This AutoPtr will retain ownership of the pointer, the client must not
	 * delete the pointer.
	 *
	 * \return The contained, plain pointer.
	 */
	Type* Get() const { return mPtr; }

	/*!
	 * \brief Relinquish ownership of the pointed-to object to the caller.
	 *
	 * After a call to release, this AutoPtr will no longer own the pointed-to
	 * object and will point to NULL.  This AutoPtr may no longer be used
	 * to access the pointed-to object.
	 * \return the contained, plain pointer, for which the caller is now
	 * responsible.
	 */
	Type* Release() 
	{
		Type *temp = mPtr;
		mPtr = NULL;
		return temp;
	}

	/*!
	 * \brief Assume ownership of a new object, any existing pointer will be
	 * deleted.
	 *
	 * \param p New pointer whose object this AutoPtr will now own.
	 */
	void Reset(Type *p = NULL) 
	{
		if(p != mPtr)
		{
			DestructorPolicy::Delete(mPtr);
			mPtr = p;
		}
	}

	/*!
	 * \brief Addition to the textbook interface.  Equivalent to get() == NULL.
	 *
	 *	\return true if this AutoPtr points to NULL.  False, otherwise.
	 */
	bool IsNull() const  { return (NULL == mPtr); }

	/*!
	 * \brief Construct from an AutoPtrRef.
	 *
	 * This may be done implicitly or explicitly.  The Ref object exists to 
	 * avoid temporarily needing to have two AutoPtrs own the same object.
	 * \param ref helper object.
	 */
	AutoPtr(const AutoPtrRef<Type> &ref) : mPtr(ref.mPtr) {}

	/*!
	 * \brief Convert to an AutoPtrRef.
	 */
	template<typename OtherType> operator AutoPtrRef<OtherType>() 
	{
		return AutoPtrRef<OtherType>(this->Release());
	}

	/*!
	 * \brief Destructive copy-convert allowing for cast of the pointer type.
	 */
	template<typename OtherType> operator AutoPtr<OtherType, DestructorPolicy>() 
	{
		return AutoPtr<OtherType, DestructorPolicy>(this->Release());
	}
};

/**
 * \brief DestructorPolicy template for pointer to array types.
 */
template <typename Type> class ArrayPointerDestructor
{
public:
	static void Delete(Type *ptr) 
	{ 
		delete [] ptr; 
	}
};

/**
 * \brief Standard implementation of a AutoPtr for pointer to array types.
 *
 * This ArrayAutoPtr template is appropriate for any pointer to a dynamically
 * allocated array (using new [] and delete []).  To manage a single dynamically
 * allocated object, use AutoPtr instead.
 *
 * Please refer to AutoPtr's documentation for more information about 
 * AutoPtrs.
 */
template <typename Type> class ArrayAutoPtr : public AutoPtr<Type, ArrayPointerDestructor<Type> > 
{
public:
	/*!
	 * \brief Construct, assuming ownership of the pointed-to object.
	 *
	 * \param p Plain pointer to an array - this AutoPtr will assume ownership
	 * of that object.
	 */
	explicit ArrayAutoPtr(Type *p = NULL) : 
		AutoPtr<Type, ArrayPointerDestructor<Type> >(p)
	{ };

	/*!
	 * \brief Construct from an AutoPtrRef.
	 *
	 * This may be done implicitly or explicitly.  The Ref object exists to 
	 * avoid temporarily needing to have two AutoPtrs own the same object.
	 * \param ref helper object.
	 */
	ArrayAutoPtr(const AutoPtrRef<Type> &ref) :
		AutoPtr<Type, ArrayPointerDestructor<Type> >(ref.mPtr)
	{ }

	/*!
	 * \brief Destructive copy-convert allowing for cast of the pointer type.
	 */
	template<typename OtherType> operator ArrayAutoPtr<OtherType>() 
	{
		return ArrayAutoPtr<OtherType>(this->Release());
	}

	/**
	 * \brief Convenience array-index operator for ArrayAutoPtrs.
	 *
	 * Using this operator is the same as calling Get()[index].  Note that this
	 * operator does not check if the index is out of bounds.  This is not a 
	 * bounds checking smart pointer, it's just a handy guard object for dealing
	 * with memory ownership.
	 *
	 * \pre index must be within the underlying array's bounds.
	 * \param index The index within the array
	 * \return a reference to the element at position index in the array.
	 */
	Type& operator[](int index) const
	{ 
		return Get()[index];
	}
};

/**
 * \brief DestructorPolicy template for types requiring destruction through 
 * DeleteThis.
 */
class DeleteThisDestructor
{
public:
	template <typename Type> static void Delete(Type *ptr)
	{ 
		if(NULL != ptr)
		{
			ptr->DeleteThis(); 
		}
	}
};


/**
 * \brief Partial template specialization of AutoPtr for types requiring 
 * destruction through a DeleteThis method.
 *
 * Please refer to AutoPtr's documentation for more information about 
 * AutoPtrs.
 */
template <typename Type> class DeleteThisAutoPtr : public AutoPtr<Type, DeleteThisDestructor > 
{
public:
	/*!
	 * \brief Construct, assuming ownership of the pointed-to object.
	 *
	 * \param p Plain pointer to an object - this AutoPtr will assume ownership
	 * of that object.
	 */
	explicit DeleteThisAutoPtr(Type *p = NULL) : 
		AutoPtr<Type, DeleteThisDestructor >(p)
	{ };

	/*!
	 * \brief Construct from an AutoPtrRef.
	 *
	 * This may be done implicitly or explicitly.  The Ref object exists to 
	 * avoid temporarily needing to have two AutoPtrs own the same object.
	 * \param ref helper object.
	 */
	DeleteThisAutoPtr(const AutoPtrRef<Type> &ref) :
		AutoPtr<Type, DeleteThisDestructor >(ref.mPtr)
	{ }
};

namespace Util
{

/**
 * \brief Old AutoPtr class, maintained to support backwards compatibility.  
 *
 * The old single-pointer AutoPtr was removed from the Util namespace, and
 * has been replaced with MaxSDK::AutoPtr&lt;Type&gt; using its default
 * destruction policy template parameter.
 */
template<typename Type> class AutoPtr : public MaxSDK::AutoPtr<Type>
{
public:
	/*!
	 * \brief Construct, assuming ownership of the pointed-to object.
	 *
	 * \param p Plain pointer to an object - this AutoPtr will assume ownership
	 * of that object.
	 */
	explicit AutoPtr(Type *p = NULL) : 
		MaxSDK::AutoPtr<Type>(p)
	{ };

	/*!
	 * \brief Construct from an AutoPtrRef.
	 *
	 * This may be done implicitly or explicitly.  The Ref object exists to 
	 * avoid temporarily needing to have two AutoPtrs own the same object.
	 * \param ref helper object.
	 */
	AutoPtr(const AutoPtrRef<Type> &ref) :
		MaxSDK::AutoPtr<Type>(ref.mPtr)
	{ }

	/*!
	 * \brief Destructive copy-convert allowing for cast of the pointer type.
	 */
	template<typename OtherType> operator AutoPtr<OtherType>() 
	{
		return AutoPtr<OtherType>(this->Release());
	}
};

}

}

