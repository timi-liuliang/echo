//-------------------------------------------------------------
// Access to Point Cache
//


#pragma once

#include "iFnPub.h"

//new class IDs

#define POINTCACHEOSM_CLASS_ID	Class_ID(0x270f1fe3, 0x3b14999)
#define POINTCACHEWSM_CLASS_ID	Class_ID(0x370f1fe3, 0x3b14999)

#define POINTCACHE_CLASS POINTCACHEOSM_CLASS_ID

//deprecated class IDS..
#define OLDPOINTCACHE_CLASS_ID	Class_ID(0x21d07ae1, 0x48d30bec)
#define OLDPOINTCACHEWSM_CLASS_ID	Class_ID(0x21d07ae1, 0x48d30bed)
#define OLDPARTICLECACHE_CLASS_ID	Class_ID(0x21d07ae1, 0x48d30bee)




class IPointCache;
class IPointCacheWSM;
class IParticleCache;

//***************************************************************
//Function Publishing System stuff   
//****************************************************************
#define POINTCACHE_INTERFACE Interface_ID(0x53b4409b, 0x18ee7cc8)

#define GetIPointCacheInterface(cd) \
			(IPointCache *)(cd)->GetInterface(POINTCACHE_INTERFACE)

#define POINTCACHEWSM_INTERFACE Interface_ID(0x53b4409b, 0x18ee7cc9)

#define GetIPointCacheWSMInterface(cd) \
			(IPointCacheWSM *)(cd)->GetInterface(POINTCACHEWSM_INTERFACE)

#define PARTICLECACHE_INTERFACE Interface_ID(0x53b4409b, 0x18ee7cd0)

#define GetIParticleCacheInterface(cd) \
			(IParticleCache *)(cd)->GetInterface(PARTICLECACHE_INTERFACE)

enum 
{
	pointcache_record,
	pointcache_setcache,
	pointcache_enablemods,
	pointcache_disablemods
};

//POINTCACHE_MODIFIERVERSION == 1
enum point_cache_mod_1_playback_params {
	playback_original,
	playback_start,
	playback_range,
	playback_graph,
};

enum point_cache_pc_params {
	pc_cache_file,
	pc_record_start,
	pc_record_end,
	pc_sample_rate,
	pc_strength,
	pc_relative,
	pc_playback_type,
	pc_playback_start,
	pc_playback_end,
	pc_playback_before,
	pc_playback_after,
	pc_playback_frame,
	pc_interp_type,
	pc_apply_to_spline,
	pc_preload_cache,
	pc_clamp_graph,
	pc_force_unc_path,
	pc_load_type,
	pc_apply_to_whole_object,
	pc_load_type_slave,
	pc_file_count
};

//****************************************************************

#pragma warning(push)
#pragma warning(disable:4100)
/*! \sa  Class FPMixinInterface
, Class IPointCacheWSM\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the PointCache Modifier. You can obtain
a pointer to the PointCache Modifier interface using;
<b>GetIPointCacheInterface(cd)</b>. This macro will return\n\n
<b>(IPointCache *)(cd)-\>GetFPInterace(POINTCACHE_INTERFACE).</b> Sample code
supporting this class can be found in
<b>/MAXSDK/SAMPLES/MODIFIERS/POINTCACHE</b>.\n\n
All methods of this class are Implemented by the System.  */

class IPointCache :  public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_0(pointcache_record, fnRecord);
			VFN_0(pointcache_setcache, fnSetCache);
			VFN_0(pointcache_enablemods, fnEnableMods);
			VFN_0(pointcache_disablemods, fnDisableMods);

		END_FUNCTION_MAP
		virtual FPInterfaceDesc* GetDesc()=0;    // <-- must implement 

		/*! \remarks This method will press the Record button in the rollup
		interface. */
		virtual void	fnRecord()=0;
		/*! \remarks This method will press the Set Cache button in the rollup
		interface. */
		virtual void	fnSetCache()=0;
		/*! \remarks This method will press the Enable Modifiers Below button
		in the rollup interface. */
		virtual void	fnEnableMods()=0;
		/*! \remarks This method will press the Disable Modifiers Below button
		in the rollup interface. */
		virtual void	fnDisableMods()=0;

	};



/*! \sa  Class FPMixinInterface
, Class IPointCache\n\n
\par Description:
This class is available in release 4.0 and later only.\n\n
This class represents the interface to the PointCache World Space Modifier. You
can obtain a pointer to the PointCache World Space Modifier interface using;
<b>GetIPointCacheWSMInterface(cd)</b>. This macro will return
<b>(IPointCacheWSM *)(cd)-\>GetFPInterace(POINTCACHEWSM_INTERFACE).</b> Sample
code supporting this class can be found in
<b>/MAXSDK/SAMPLES/MODIFIERS/POINTCACHE</b>.\n\n
All methods of this class are Implemented by the System.  */
class IPointCacheWSM :  public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_0(pointcache_record, fnRecord);
			VFN_0(pointcache_setcache, fnSetCache);
			VFN_0(pointcache_enablemods, fnEnableMods);
			VFN_0(pointcache_disablemods, fnDisableMods);

		END_FUNCTION_MAP
		virtual FPInterfaceDesc* GetDesc()=0;    // <-- must implement 

		/*! \remarks This method will press the Record button in the rollup
		interface. */
		virtual void	fnRecord()=0;
		/*! \remarks This method will press the Set Cache button in the rollup
		interface. */
		virtual void	fnSetCache()=0;
		/*! \remarks This method will press the Enable Modifiers Below button
		in the rollup interface. */
		virtual void	fnEnableMods()=0;
		/*! \remarks This method will press the Disable Modifiers Below button
		in the rollup interface. */
		virtual void	fnDisableMods()=0;

	};

class IParticleCache :  public FPMixinInterface 
	{
	public:

		//Function Publishing System
		//Function Map For Mixin Interface
		//*************************************************
		BEGIN_FUNCTION_MAP
			VFN_0(pointcache_record, fnRecord);
			VFN_0(pointcache_setcache, fnSetCache);

		END_FUNCTION_MAP
		virtual FPInterfaceDesc* GetDesc()=0;    // <-- must implement 

		virtual void	fnRecord()=0;
		virtual void	fnSetCache()=0;

	};
#pragma warning(pop)
