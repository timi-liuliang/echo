/*	
 *		MAX_controller_protocol.h - def_generics for the operations on MAX controller objects
 *
 *			Copyright © John Wainwright 1996
 *
 */
#pragma warning(push)
#pragma warning(disable:4100)
/* controller operations */

	use_generic						( copy,						"copy");

/* time operations */

	def_visible_generic_debug_ok	( supportsTimeOperations,	"supportsTimeOperations");
	def_mapped_generic				( deleteTime,				"deleteTime");
	def_mapped_generic				( reverseTime,				"reverseTime");
	def_mapped_generic				( scaleTime,				"scaleTime");
	def_mapped_generic				( insertTime,				"insertTime");
	
	def_visible_generic_debug_ok	( getTimeRange,				"getTimeRange");
	def_mapped_generic				( setTimeRange,				"setTimeRange");

/* key operations */

	def_mapped_generic				( addNewKey,				"addNewKey");
	def_mapped_generic				( deleteKeys,				"deleteKeys");
	def_visible_generic				( deleteKey,				"deleteKey");
//	def_visible_generic				( appendKey,				"appendKey"); // RK: 6/19/02, Commenting these, breaks the SDK
//	def_visible_generic				( assignKey,				"assignKey"); // RK: 6/19/02, Commenting these, breaks the SDK
	def_mapped_generic				( selectKeys,				"selectKeys");
	def_visible_generic				( selectKey,				"selectKey");
	def_mapped_generic				( deselectKeys,				"deselectKeys");
	def_visible_generic				( deselectKey,				"deselectKey");
	def_visible_generic_debug_ok	( isKeySelected,			"isKeySelected");
	def_mapped_generic				( moveKeys,					"moveKeys");
	def_visible_generic				( moveKey,					"moveKey");
	def_visible_generic_debug_ok	( numKeys,					"numKeys");
	def_visible_generic_debug_ok	( getKey,					"getKey");
	def_visible_generic_debug_ok	( getKeyTime,				"getKeyTime");
	def_visible_generic_debug_ok	( getKeyIndex,				"getKeyIndex");
	def_visible_generic_debug_ok	( numSelKeys,				"numSelKeys");
	def_mapped_generic				( sortKeys,					"sortKeys");
	def_mapped_generic				( reduceKeys,				"reduceKeys");
	def_mapped_generic				( mapKeys,					"mapKeys");

/* ORT, ease curve functions */

	def_mapped_generic				( addEaseCurve,				"addEaseCurve");
	def_mapped_generic				( deleteEaseCurve,			"deleteEaseCurve");
	def_visible_generic_debug_ok	( numEaseCurves,			"numEaseCurves");
	def_visible_generic				( applyEaseCurve,			"applyEaseCurve");
	def_visible_generic				( addMultiplierCurve,		"addMultiplierCurve");
	def_visible_generic				( deleteMultiplierCurve,	"deleteMultiplierCurve");
	def_visible_generic_debug_ok	( numMultiplierCurves,		"numMultiplierCurves");
	def_visible_generic_debug_ok	( getMultiplierValue,		"getMultiplierValue");

	def_visible_generic_debug_ok	( getBeforeORT,				"getBeforeORT");
	def_visible_generic_debug_ok	( getAfterORT,				"getAfterORT");
	def_mapped_generic				( setBeforeORT,				"setBeforeORT");
	def_mapped_generic				( setAfterORT,				"setAfterORT");
	def_mapped_generic				( enableORTs,				"enableORTs");
#pragma warning(pop)
