/*	
 *		MAX_time_functions.h - recursive animatable time functions
 *
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)

/* time operations */

	def_time_fn  ( deleteTime );
	def_time_fn  ( reverseTime );
	def_time_fn  ( scaleTime );
	def_time_fn  ( insertTime );
	def_time_fn  ( setTimeRange );

/* key operations */

	def_time_fn  ( addNewKey );
	def_time_fn  ( deleteKeys );
//	def_time_fn  ( appendKey ); // RK: 6/19/02, Commenting these, breaks the SDK
//	def_time_fn  ( assignKey ); // RK: 6/19/02, Commenting these, breaks the SDK
	def_time_fn  ( selectKeys );
	def_time_fn  ( deselectKeys );
	def_time_fn  ( moveKeys );
	def_time_fn  ( mapKeys );
	def_time_fn  ( sortKeys );
	def_time_fn  ( reduceKeys );

/* ORT, ease/multiplier curves */

	def_time_fn  ( addEaseCurve );
	def_time_fn  ( deleteEaseCurve );
	def_time_fn  ( setBeforeORT );
	def_time_fn  ( setAfterORT );
	def_time_fn  ( enableORTs );

#pragma warning(pop)
