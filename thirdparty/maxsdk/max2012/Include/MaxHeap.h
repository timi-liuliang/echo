/**********************************************************************
 *<
	FILE: MaxHeap.h

	DESCRIPTION:  A base class to control memory management for all 
				for all Max classes

	CREATED BY: Justin  Yong

	HISTORY: created 11/29/05

 *>	Copyright (c) 2005, All Rights Reserved.
 **********************************************************************/

#pragma once

#include <new>

#include "utilexp.h"

// Undefine possible new and delete macros, so they won't break our operators declarations. For instance, a debug
// macro for new could look like this: "#define new _debug_new(__FILE__, __LINE__)". In the MaxHeapOperators class,
// we would obtain something like this: "UtilExport static void* _new_debug_new(__FILE, __LINE__)( size_t size );",
// and it wouldn't compile.
#undef new
#undef delete

//! \brief The base class in Max that controls the memory management for any class that derives from it.
/*! The sole purpose of this class is to provide various overloads of the new and delete operators. By doing this, any 
class that derives from this class and doesn't implement it's own new and delete operators will see all of its memory
operation go through the Max Core. Doing this permits plugins to easily benefit from the same memory allocation
optimizations the Max Core has or will receive in the future. It also permits plugins linked with a different CRT than 
the Max Core to safely pass memory pointers to heap blocks allocated on one side of the plugin dll boundary and 
deallocated on the other.

If a plugin class has to implement its own new and delete operators but also derive indirectly from MaxHeapOperators,
access to the new and delete operators will have to be disambiguated. For an example on how to do this with a "using" 
declaration, see the class MaxWrapper in maxsdk\\include\\maxscrpt\\maxobj.h.
*/
class MaxHeapOperators
{
public:

	///  \brief 
	/// Standard new operator used to allocate objects
	/// If there is insufficient memory, an exception will be thrown
	/// 
	/// \param[in] size: The size of the object to be allocated  
	UtilExport static void* operator new( size_t size ); 

	///  \brief 
	/// Standard new operator used to allocate objects 
	/// if there is insufficient memory, NULL will be returned
	/// 
	/// \param[in] size: The size of the object to be allocated  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	UtilExport static void* operator new( size_t size, const std::nothrow_t & e );

	///  \brief 
	/// New operator used to allocate objects that takes the filename and line number where the new was called
	/// If there is insufficient memory, an exception will be thrown
	/// 
	/// \param[in] size: The size of the object to be allocated  
	/// \param[in] filename: The name of the file that contains the call to new - may be NULL
	/// \param[in] line: The line number in the file that contains the call to new
	UtilExport static void* operator new( size_t size, const char* filename, int line );

	///  \brief 
	/// New operator used to allocate objects that takes the filename and line number where the new was called
	/// If there is insufficient memory, NULL will be returned
	/// 
	/// \param[in] size: The size of the object to be allocated  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] filename: The name of the file that contains the call to new - may be NULL
	/// \param[in] line: The line number in the file that contains the call to new
	UtilExport static void* operator new( size_t size, const std::nothrow_t & e, const char * filename, int line );

	///  \brief 
	/// New operator used to allocate objects that takes extra flags to specify special operations
	/// If there is insufficient memory, an exception will be thrown
	/// 
	/// \param[in] size: The size of the object to be allocated  
	/// \param[in] flags: Flags specifying whether any special operations should be done when allocating memory
	UtilExport static void* operator new( size_t size, unsigned long flags );

	///  \brief 
	/// New operator used to allocate objects that takes extra flags to specify special operations
	/// If there is insufficient memory, NULL will be returned
	/// 
	/// \param[in] size: The size of the object to be allocated  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] flags: Flags specifying whether any special operations should be done when allocating memory
	UtilExport static void* operator new( size_t size, const std::nothrow_t & e, unsigned long flags );

	///  \brief 
	/// New operator used to allocate arrays of objects
	/// If there is insufficient memory, an exception will be thrown
	/// 
	/// \param[in] size: The size of the array of objects to be allocated  
	UtilExport static void* operator new[]( size_t size );

	///  \brief 
	/// New operator used to allocate arrays of objects
	/// If there is insufficient memory, NULL will be returned
	/// 
	/// \param[in] size: The size of the array of objects to be allocated  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	UtilExport static void* operator new[]( size_t size, const std::nothrow_t & e );

	///  \brief 
	/// New operator used to allocate arrays of objects
	/// If there is insufficient memory, an exception will be thrown
	/// 
	/// \param[in] size: The size of the array of objects to be allocated  
	/// \param[in] filename: The name of the file that contains the call to new - may be NULL
	/// \param[in] line: The line number in the file that contains the call to new
	UtilExport static void* operator new[]( size_t size, const char* filename, int line );

	///  \brief 
	/// New operator used to allocate arrays of objects
	/// If there is insufficient memory, NULL will be returned
	/// 
	/// \param[in] size: The size of the array of objects to be allocated  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] filename: The name of the file that contains the call to new - may be NULL
	/// \param[in] line: The line number in the file that contains the call to new
	UtilExport static void* operator new[]( size_t size, const std::nothrow_t & e, const char * filename, int line );

	///  \brief 
	/// New operator used to allocate arrays of objects
	/// If there is insufficient memory, an exception will be thrown
	/// 
	/// \param[in] size: The size of the array of objects to be allocated  
	/// \param[in] flags: Flags specifying whether any special operations should be done when allocating memory
	UtilExport static void* operator new[]( size_t size, unsigned long flags );

	///  \brief 
	/// New operator used to allocate arrays of objects
	/// If there is insufficient memory, NULL will be returned
	/// 
	/// \param[in] size: The size of the array of objects to be allocated  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] flags: Flags specifying whether any special operations should be done when allocating memory
	UtilExport static void* operator new[]( size_t size, const std::nothrow_t & e, unsigned long flags );


	///  \brief 
	/// Standard delete operator used to deallocate an object
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	UtilExport static void operator delete( void * ptr );

	///  \brief 
	/// Standard delete operator used to deallocate an object
	/// If the pointer is invalid, nothing will happen
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	UtilExport static void operator delete( void * ptr, const std::nothrow_t& e );
	
		///  \brief 
	/// Delete operator used to deallocate an object that takes the filename and line number where the delete was called
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] filename: The name of the file that contains the call to delete - may be NULL
	/// \param[in] line: The line number in the file that contains the call to delete 
	UtilExport static void operator delete( void * ptr, const char * filename, int line );
	
	///  \brief 
	/// Delete operator used to deallocate an object that takes the filename and line number where the delete was called
	/// If the pointer is invalid, nothing will happen
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] filename: The name of the file that contains the call to delete - may be NULL
	/// \param[in] line: The line number in the file that contains the call to delete 
	UtilExport static void operator delete( void * ptr, const std::nothrow_t & e, const char * filename, int line );
	
	///  \brief 
	/// Delete operator used to deallocate an object that takes extra flags to specify special operations
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] flags: Flags specifying wheter any special operations should be done when allocating memory.
	UtilExport static void operator delete( void * ptr, unsigned long flags );
	
	///  \brief 
	/// Delete operator used to deallocate an object that takes extra flags to specify special operations
	/// If the pointer is invalid, nothing will happen
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] flags: Flags specifying wheter any special operations should be done when allocating memory.
	UtilExport static void operator delete( void * ptr, const std::nothrow_t & e, unsigned long flags );

	///  \brief 
	/// Standard delete operator used to deallocate an array of objects
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the array of objects to be deleted  
	UtilExport static void operator delete[]( void * ptr );

	///  \brief 
	/// Standard delete operator used to deallocate an array of objects
	/// If the pointer is invalid, nothing will happen
	/// 
	/// \param[in] ptr: The void pointer to the array of objects to be deleted  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	UtilExport static void operator delete[]( void * ptr, const std::nothrow_t& e );
	
		///  \brief 
	/// Delete operator used to deallocate an array of objects that takes the filename and line number where the delete was called
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] filename: The name of the file that contains the call to delete - may be NULL
	/// \param[in] line: The line number in the file that contains the call to delete 
	UtilExport static void operator delete[]( void * ptr, const char * filename, int line );
	
		///  \brief 
	/// Delete operator used to deallocate an array of objects that takes the filename and line number where the delete was called
	/// If the pointer is invalid, nothing will happen
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] filename: The name of the file that contains the call to delete - may be NULL
	/// \param[in] line: The line number in the file that contains the call to delete 
	UtilExport static void operator delete[]( void * ptr, const std::nothrow_t& e, const char * filename, int line );
	
	///  \brief 
	/// Delete operator used to deallocate an array of objects that takes extra flags to specify special operations
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] flags: Flags specifying wheter any special operations should be done when allocating memory.
	UtilExport static void operator delete[]( void * ptr, unsigned long flags );
	
	///  \brief 
	/// Delete operator used to deallocate an array of objects that takes extra flags to specify special operations
	/// If the pointer is invalid, an exception will be thrown
	/// 
	/// \param[in] ptr: The void pointer to the object to be deleted  
	/// \param[in] e: A std::nothrow_t to specify that no exception should be thrown
	/// \param[in] flags: Flags specifying wheter any special operations should be done when allocating memory.
	UtilExport static void operator delete[]( void * ptr, const std::nothrow_t&e, unsigned long flags );
	
	/// \brief
	/// Placement new operator. 
	/// The placement new operator should only be called with a pointer that already has been allocated in the Max' 
	/// heap. You can use maxheapdirect.h to allocate yourself memory on the Max' heap.
	/// \param[in] size: The amount of memory to allocate
	/// \param[in] placement_ptr: The location where the initialization should take place
	UtilExport static void* operator new( size_t size, void * placement_ptr );
	
	/// \brief
	/// Placement delete operator. 
	/// Corresponds to the placement new operator. Doesn't affect memory.
	/// \param[in] ptr: The location of the memory to release.
	/// \param[in] placement_ptr: Location where the destruction should take place. Should be the same as ptr.
	UtilExport static void  operator delete( void * ptr, void * placement_ptr );

};

