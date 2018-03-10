/*	
 *		HashTable.h - HashTable class for MAXScript
 *
 *			Copyright (c) John Wainwright 1996
 *
 */

#pragma once

#include "..\kernel\value.h"

struct binding 
{
	void*	key;
	void*	value;
};

struct secondary						/* secondary extent struct			*/
{
	UINT	size;				      /* size of secondary extent	*/	// SR NOTE64: was size_t
	binding*	bindings;			/* table of bindings				*/
};
	
#define KEY_IS_OBJECT	0x0001		/* init flags that indicate whether keys & values are full MXS collectable objects */
#define VALUE_IS_OBJECT	0x0002

ScripterExport int	default_eq_fn(const void* key1, const void* key2);  /* default comparator & hash fns */
ScripterExport DWORD_PTR default_hash_fn(const void* key);

class HashTabMapper;

visible_class_debug_ok (HashTable)

class HashTable : public Value
{
public:
   // SR NOTE64: hash functions returned "INT_PTR" (and previously I suppose they
   // used to return 'int'; in Win64, the hash values can get pretty huge, and negative,
   // and then hash_value % size will give a negative value -- not good.  I'd change it
   // to a DWORD, but then the default hash function, which just returns the pointer
   // casted to an INT_PTR, would have to do more computations and this might slow
   // things down too much.  So I'll just change it to an unsigned value instead and
   // introduce a typedef for it.
   typedef DWORD_PTR (*hash_fn_type)(const void*);
   typedef int       (*eq_fn_type)(const void*, const void*);

private:
	secondary	**table;			/* primary extent: tbl of second's	*/
	UINT        size;				/* table size						*/	// SR NOTE64: Was size_t
	int			n_entries;			/* no. entries in primary extent	*/
	eq_fn_type  eq_fn;            /* key equivalence function		*/
	hash_fn_type hash_fn;	      /* key hgashing function			*/
	int			cursor;				/* cursors used for sequencing...	*/
	int			secondCursor;			
	short		flags;
	HashTable*	inner;				/* links to next & prev tables when */
	HashTable*	outer;				/* used as a lexical scope table	*/
	int			level;				// scope nesting level

public:
	ScripterExport HashTable(UINT primary_size, eq_fn_type key_eq_fn, hash_fn_type key_hash_fn, int flags);
				HashTable() { init(17, default_eq_fn, default_hash_fn, KEY_IS_OBJECT + VALUE_IS_OBJECT); }
				HashTable(UINT primary_size) { init(primary_size, default_eq_fn, default_hash_fn, KEY_IS_OBJECT + VALUE_IS_OBJECT); }
	ScripterExport ~HashTable();
	ScripterExport void		init(UINT primary_size, eq_fn_type key_eq_fn, hash_fn_type key_hash_fn, int flags);
#	define		is_hashtable(v) ((DbgVerify(!is_sourcepositionwrapper(v)), (v))->tag == class_tag(HashTable))

	ScripterExport static CRITICAL_SECTION hash_update;	// for syncing allocation hashtable updates

	static void	setup();

				classof_methods (HashTable, Value);
	void		collect() { delete this;}
	ScripterExport void		gc_trace();

	ScripterExport Value*	get(const void* key);
	ScripterExport Value*	put(const void* key, const void* val);
	ScripterExport Value*	put_new(const void* key, const void* val);
	ScripterExport Value*	find_key(const void *val);
	ScripterExport Value*	set(const void* key, const void* val);
	ScripterExport void		remove(const void* key);
	ScripterExport void		map_keys_and_vals(void (*fn)(const void* key, const void* val, void* arg), void* arg);
	ScripterExport void		map_keys_and_vals(HashTabMapper* mapper);
	ScripterExport int		num_entries() { return n_entries; }


	HashTable*	enter_scope();
	HashTable*	leave_scope();
	HashTable*	next_scope();
	int			scope_level() { return level; }
};

class HashTabMapper 
{
public:
	virtual void map(const void* key, const void* val)=0;
};

#define SECONDARY_BUCKET	5

