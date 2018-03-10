/*
    fmod_studio_common.h
    Copyright (c), Firelight Technologies Pty, Ltd. 2016.

    This header defines common enumerations, structs and callbacks that are shared between the C and C++ interfaces.
*/

#ifndef FMOD_STUDIO_COMMON_H
#define FMOD_STUDIO_COMMON_H

#include "fmod.h"


/*
    FMOD Studio types.
*/

typedef struct FMOD_STUDIO_SYSTEM FMOD_STUDIO_SYSTEM;
typedef struct FMOD_STUDIO_EVENTDESCRIPTION FMOD_STUDIO_EVENTDESCRIPTION;
typedef struct FMOD_STUDIO_EVENTINSTANCE FMOD_STUDIO_EVENTINSTANCE;
typedef struct FMOD_STUDIO_CUEINSTANCE FMOD_STUDIO_CUEINSTANCE;
typedef struct FMOD_STUDIO_PARAMETERINSTANCE FMOD_STUDIO_PARAMETERINSTANCE;
typedef struct FMOD_STUDIO_BUS FMOD_STUDIO_BUS;
typedef struct FMOD_STUDIO_VCA FMOD_STUDIO_VCA;
typedef struct FMOD_STUDIO_BANK FMOD_STUDIO_BANK;
typedef struct FMOD_STUDIO_COMMANDREPLAY FMOD_STUDIO_COMMANDREPLAY;


/*
[DEFINE]
[
    [NAME]
    FMOD_STUDIO_INITFLAGS

    [DESCRIPTION]
    Studio System initialization flags.
    Use them with Studio::System::initialize in the *studioflags* parameter to change various behavior.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::initialize
]
*/
#define FMOD_STUDIO_INIT_NORMAL                     0x00000000  /* Initialize normally. */
#define FMOD_STUDIO_INIT_LIVEUPDATE                 0x00000001  /* Enable live update. */
#define FMOD_STUDIO_INIT_ALLOW_MISSING_PLUGINS      0x00000002  /* Load banks even if they reference plugins that have not been loaded. */
#define FMOD_STUDIO_INIT_SYNCHRONOUS_UPDATE         0x00000004  /* Disable asynchronous processing and perform all processing on the calling thread instead. */
#define FMOD_STUDIO_INIT_DEFERRED_CALLBACKS         0x00000008  /* Defer timeline callbacks until the main update. See Studio::EventInstance::setCallback for more information. */
/* [DEFINE_END] */

typedef unsigned int FMOD_STUDIO_INITFLAGS;


/*
[ENUM]
[
    [DESCRIPTION]
    These values describe the loading status of various objects.

    [REMARKS]
    Calling Studio::System::loadBankFile, Studio::System::loadBankMemory or Studio::System::loadBankCustom
    will trigger loading of metadata from the bank.

    Calling Studio::EventDescription::loadSampleData, Studio::EventDescription::createInstance
    or Studio::Bank::loadSampleData may trigger asynchronous loading of sample data.

    [SEE_ALSO]
    Studio::EventDescription::getSampleLoadingState
    Studio::Bank::getLoadingState
    Studio::Bank::getSampleLoadingState
]
*/
typedef enum FMOD_STUDIO_LOADING_STATE
{
    FMOD_STUDIO_LOADING_STATE_UNLOADING,        /* Currently unloading. */
    FMOD_STUDIO_LOADING_STATE_UNLOADED,         /* Not loaded. */
    FMOD_STUDIO_LOADING_STATE_LOADING,          /* Loading in progress. */
    FMOD_STUDIO_LOADING_STATE_LOADED,           /* Loaded and ready to play. */
    FMOD_STUDIO_LOADING_STATE_ERROR,            /* Failed to load and is now in error state. */

    FMOD_STUDIO_LOADING_STATE_FORCEINT = 65536  /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_LOADING_STATE;


/*
[ENUM]
[
    [DESCRIPTION]
    Specifies how to use the memory buffer passed to Studio::System::loadBankMemory.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::loadBankMemory
    Studio::Bank::unload
]
*/
typedef enum FMOD_STUDIO_LOAD_MEMORY_MODE
{
    FMOD_STUDIO_LOAD_MEMORY,                    /* When passed to Studio::System::loadBankMemory, FMOD duplicates the memory into its own buffers. Your buffer can be freed after Studio::System::loadBankMemory returns. */
    FMOD_STUDIO_LOAD_MEMORY_POINT,              /* This differs from FMOD_STUDIO_LOAD_MEMORY in that FMOD uses the memory as is, without duplicating the memory into its own buffers. Cannot not be freed after load, only after calling Studio::Bank::unload. */

    FMOD_STUDIO_LOAD_MEMORY_FORCEINT = 65536    /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_LOAD_MEMORY_MODE;


/*
[ENUM]
[
    [DESCRIPTION]
    Describes the type of a parameter.

    [REMARKS]
    There are two primary types of parameters: game controlled and automatic.
    Game controlled parameters receive their value from the API using
    Studio::ParameterInstance::setValue. Automatic parameters are updated inside
    FMOD based on the positional information of the event and listener.

    **Horizontal angle** means the angle between vectors projected onto the
    listener's XZ plane (for the EVENT_ORIENTATION and DIRECTION parameters)
    or the global XZ plane (for the LISTENER_ORIENTATION parameter).

    [SEE_ALSO]
    FMOD_STUDIO_PARAMETER_DESCRIPTION
    Studio::ParameterInstance::setValue
    Studio::EventInstance::set3DAttributes
    Studio::System::setListenerAttributes
]
*/
typedef enum FMOD_STUDIO_PARAMETER_TYPE
{
    FMOD_STUDIO_PARAMETER_GAME_CONTROLLED,                  /* Controlled via the API using Studio::ParameterInstance::setValue. */
    FMOD_STUDIO_PARAMETER_AUTOMATIC_DISTANCE,               /* Distance between the event and the listener. */
    FMOD_STUDIO_PARAMETER_AUTOMATIC_EVENT_CONE_ANGLE,       /* Angle between the event's forward vector and the vector pointing from the event to the listener (0 to 180 degrees). */
    FMOD_STUDIO_PARAMETER_AUTOMATIC_EVENT_ORIENTATION,      /* Horizontal angle between the event's forward vector and listener's forward vector (-180 to 180 degrees). */
    FMOD_STUDIO_PARAMETER_AUTOMATIC_DIRECTION,              /* Horizontal angle between the listener's forward vector and the vector pointing from the listener to the event (-180 to 180 degrees). */
    FMOD_STUDIO_PARAMETER_AUTOMATIC_ELEVATION,              /* Angle between the listener's XZ plane and the vector pointing from the listener to the event (-90 to 90 degrees). */
    FMOD_STUDIO_PARAMETER_AUTOMATIC_LISTENER_ORIENTATION,   /* Horizontal angle between the listener's forward vector and the global positive Z axis (-180 to 180 degrees). */

    FMOD_STUDIO_PARAMETER_MAX,                              /* Maximum number of parameter types supported. */
    FMOD_STUDIO_PARAMETER_FORCEINT = 65536                  /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_PARAMETER_TYPE;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Information for loading a bank with Studio::System::loadBankCustom.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::loadBankCustom
]
*/
typedef struct FMOD_STUDIO_BANK_INFO
{
    int   size;                                 /* The size of this struct (for binary compatibility) */
    void *userData;                             /* User data to be passed to the file callbacks */
    int   userDataLength;                       /* If this is non-zero, userData will be copied internally */
    FMOD_FILE_OPEN_CALLBACK  openCallback;      /* Callback for opening this file. */
    FMOD_FILE_CLOSE_CALLBACK closeCallback;     /* Callback for closing this file. */
    FMOD_FILE_READ_CALLBACK  readCallback;      /* Callback for reading from this file. */
    FMOD_FILE_SEEK_CALLBACK  seekCallback;      /* Callback for seeking within this file. */
} FMOD_STUDIO_BANK_INFO;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Structure describing an event parameter.

    [REMARKS]

    [SEE_ALSO]
    Studio::EventDescription::getParameter
    FMOD_STUDIO_PARAMETER_TYPE
]
*/
typedef struct FMOD_STUDIO_PARAMETER_DESCRIPTION
{
    const char *name;                           /* Name of the parameter. */
    float minimum;                              /* Minimum parameter value. */
    float maximum;                              /* Maximum parameter value. */
    FMOD_STUDIO_PARAMETER_TYPE type;            /* Type of the parameter */
} FMOD_STUDIO_PARAMETER_DESCRIPTION;


/*
[ENUM]
[
    [DESCRIPTION]
    These definitions describe a user property's type.

    [REMARKS]

    [SEE_ALSO]
    FMOD_STUDIO_USER_PROPERTY
]
*/
typedef enum FMOD_STUDIO_USER_PROPERTY_TYPE
{
    FMOD_STUDIO_USER_PROPERTY_TYPE_INTEGER,         /* Integer property */
    FMOD_STUDIO_USER_PROPERTY_TYPE_BOOLEAN,         /* Boolean property */
    FMOD_STUDIO_USER_PROPERTY_TYPE_FLOAT,           /* Float property */
    FMOD_STUDIO_USER_PROPERTY_TYPE_STRING,          /* String property */

    FMOD_STUDIO_USER_PROPERTY_TYPE_FORCEINT = 65536 /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_USER_PROPERTY_TYPE;


/*
[ENUM]
[
    [DESCRIPTION]
    These definitions describe built-in event properties.

    [REMARKS]
    For FMOD_STUDIO_EVENT_PROPERTY_CHANNELPRIORITY, a value of -1 uses the priority
    set in FMOD Studio, while other values override it. This property uses the same
    system as Channel::setPriority; this means lower values are higher priority
    (i.e. 0 is the highest priority while 256 is the lowest).

    [SEE_ALSO]
    Studio::EventInstance::getProperty
    Studio::EventInstance::setProperty
]
*/
typedef enum FMOD_STUDIO_EVENT_PROPERTY
{
    FMOD_STUDIO_EVENT_PROPERTY_CHANNELPRIORITY,     /* Priority to set on low-level channels created by this event instance (-1 to 256). */
    FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_DELAY,      /* Schedule delay to synchronized playback for multiple tracks in DSP clocks, or -1 for default. */
    FMOD_STUDIO_EVENT_PROPERTY_SCHEDULE_LOOKAHEAD,  /* Schedule look-ahead on the timeline in DSP clocks, or -1 for default. */
    FMOD_STUDIO_EVENT_PROPERTY_MINIMUM_DISTANCE,    /* Override the event's 3D minimum distance, or -1 for default. */
    FMOD_STUDIO_EVENT_PROPERTY_MAXIMUM_DISTANCE,    /* Override the event's 3D maximum distance, or -1 for default. */
    FMOD_STUDIO_EVENT_PROPERTY_MAX,                 /* Maximum number of event properties supported. */

    FMOD_STUDIO_EVENT_PROPERTY_FORCEINT = 65536 /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_EVENT_PROPERTY;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Structure describing a user property.

    [REMARKS]

    [SEE_ALSO]
    Studio::EventDescription::getUserProperty
]
*/
typedef struct FMOD_STUDIO_USER_PROPERTY
{
    const char *name;                           /* Name of the user property. */
    FMOD_STUDIO_USER_PROPERTY_TYPE type;        /* Type of the user property. Use this to select one of the following values. */

    union
    {
        int intValue;                           /* Value of the user property. Only valid when type is FMOD_STUDIO_USER_PROPERTY_TYPE_INTEGER. */
        FMOD_BOOL boolValue;                    /* Value of the user property. Only valid when type is FMOD_STUDIO_USER_PROPERTY_TYPE_BOOLEAN. */
        float floatValue;                       /* Value of the user property. Only valid when type is FMOD_STUDIO_USER_PROPERTY_TYPE_FLOAT. */
        const char *stringValue;                /* Value of the user property. Only valid when type is FMOD_STUDIO_USER_PROPERTY_TYPE_STRING. */
    };
} FMOD_STUDIO_USER_PROPERTY;


/*
[DEFINE]
[
    [NAME]
    FMOD_STUDIO_SYSTEM_CALLBACK_TYPE

    [DESCRIPTION]
    These callback types are used with Studio::System::setCallback.

    [REMARKS]

    [SEE_ALSO]
    FMOD_STUDIO_SYSTEM_CALLBACK
    Studio::System::setCallback
]
*/
#define FMOD_STUDIO_SYSTEM_CALLBACK_PREUPDATE       0x00000001  /* Called at the start of the main Studio update.  For async mode this will be on its own thread. */
#define FMOD_STUDIO_SYSTEM_CALLBACK_POSTUPDATE      0x00000002  /* Called at the end of the main Studio update.  For async mode this will be on its own thread. */
#define FMOD_STUDIO_SYSTEM_CALLBACK_BANK_UNLOAD     0x00000004  /* Called when bank has just been unloaded, after all resources are freed. CommandData will be the bank handle.*/
#define FMOD_STUDIO_SYSTEM_CALLBACK_ALL             0xFFFFFFFF  /* Pass this mask to Studio::System::setCallback to receive all callback types. */
/* [DEFINE_END] */

typedef unsigned int FMOD_STUDIO_SYSTEM_CALLBACK_TYPE;


/*
[DEFINE]
[
    [NAME]
    FMOD_STUDIO_EVENT_CALLBACK_TYPE

    [DESCRIPTION]
    These callback types are used with FMOD_STUDIO_EVENT_CALLBACK.

    [REMARKS]
    The data passed to the event callback function in the *parameters* argument varies based on the callback type.

    FMOD_STUDIO_EVENT_CALLBACK_STARTED is called when:

     * Studio::EventInstance::start has been called on an event which was not already playing.

    FMOD_STUDIO_EVENT_CALLBACK_RESTARTED is called when:

     * Studio::EventInstance::start has been called on an event which was already playing.

    FMOD_STUDIO_EVENT_CALLBACK_STOPPED is called when:

     * The event has stopped due to Studio::EventInstance::stop being called with FMOD_STUDIO_STOP_IMMEDIATE.
     * The event has finished fading out after Studio::EventInstance::stop was called with FMOD_STUDIO_STOP_ALLOWFADEOUT.
     * The event has stopped naturally by reaching the end of the timeline, and no further sounds can be triggered due to
       parameter changes.

    FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND is called when:

    * A programmer sound is about to play. FMOD expects the callback to provide an FMOD::Sound object for it to use.

    FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND is called when:

    * A programmer sound has stopped playing. At this point it is safe to release the FMOD::Sound object that was used.

    [SEE_ALSO]
    Studio::EventDescription::setCallback
    Studio::EventInstance::setCallback
    FMOD_STUDIO_EVENT_CALLBACK
]
*/
#define FMOD_STUDIO_EVENT_CALLBACK_STARTED                  0x00000001  /* Called when an instance starts. Parameters = unused. */
#define FMOD_STUDIO_EVENT_CALLBACK_RESTARTED                0x00000002  /* Called when an instance is restarted. Parameters = unused. */
#define FMOD_STUDIO_EVENT_CALLBACK_STOPPED                  0x00000004  /* Called when an instance stops. Parameters = unused. */
#define FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND  0x00000008  /* Called when a programmer sound needs to be created in order to play a programmer instrument. Parameters = FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES. */
#define FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND 0x00000010  /* Called when a programmer sound needs to be destroyed. Parameters = FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES. */
#define FMOD_STUDIO_EVENT_CALLBACK_PLUGIN_CREATED           0x00000020  /* Called when a DSP plugin instance has just been created. Parameters = FMOD_STUDIO_PLUGIN_INSTANCE_PROPERTIES. */
#define FMOD_STUDIO_EVENT_CALLBACK_PLUGIN_DESTROYED         0x00000040  /* Called when a DSP plugin instance is about to be destroyed. Parameters = FMOD_STUDIO_PLUGIN_INSTANCE_PROPERTIES. */
#define FMOD_STUDIO_EVENT_CALLBACK_CREATED                  0x00000080  /* Called when an instance is fully created. Parameters = unused. */
#define FMOD_STUDIO_EVENT_CALLBACK_DESTROYED                0x00000100  /* Called when an instance is just about to be destroyed. Parameters = unused. */
#define FMOD_STUDIO_EVENT_CALLBACK_START_FAILED             0x00000200  /* Called when an instance did not start, e.g. due to polyphony. Parameters = unused. */
#define FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER          0x00000400  /* Called when the timeline passes a named marker.  Parameters = FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES. */
#define FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT            0x00000800  /* Called when the timeline hits a beat in a tempo section.  Parameters = FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES. */
#define FMOD_STUDIO_EVENT_CALLBACK_ALL                      0xFFFFFFFF  /* Pass this mask to Studio::EventDescription::setCallback or Studio::EventInstance::setCallback to receive all callback types. */
/* [DEFINE_END] */

typedef unsigned int FMOD_STUDIO_EVENT_CALLBACK_TYPE;

/*
[STRUCTURE]
[
    [DESCRIPTION]
    This structure holds information about a programmer sound.

    [REMARKS]
    This data is passed to the event callback function when type is FMOD_STUDIO_EVENT_CALLBACK_CREATE_PROGRAMMER_SOUND
    or FMOD_STUDIO_EVENT_CALLBACK_DESTROY_PROGRAMMER_SOUND.

    To support non-blocking loading of FSB subsounds, you can specify the subsound you want to use by setting the
    subsoundIndex field. This will cause FMOD to wait until the provided sound is ready and then get the specified
    subsound from it.

    [SEE_ALSO]
    FMOD_STUDIO_EVENT_CALLBACK
    Studio::EventDescription::setCallback
    Studio::EventInstance::setCallback
]
*/
typedef struct FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES
{
    const char *name;                           /* The name of the programmer instrument (set in FMOD Studio). */
    FMOD_SOUND *sound;                          /* The programmer-created sound. This should be filled in by the create callback, and cleaned up by the destroy callback. This can be cast to/from FMOD::Sound* type. */
    int subsoundIndex;                          /* The index of the subsound to use, or -1 if the provided sound should be used directly. Defaults to -1. */
} FMOD_STUDIO_PROGRAMMER_SOUND_PROPERTIES;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    This structure holds information about a DSP plugin instance.

    [REMARKS]
    This data is passed to the event callback function when type is FMOD_STUDIO_EVENT_CALLBACK_PLUGIN_CREATED
    or FMOD_STUDIO_EVENT_CALLBACK_PLUGIN_DESTROYED.

    [SEE_ALSO]
    FMOD_STUDIO_EVENT_CALLBACK
    Studio::EventDescription::setCallback
    Studio::EventInstance::setCallback
]
*/
typedef struct FMOD_STUDIO_PLUGIN_INSTANCE_PROPERTIES
{
    const char *name;                           /* The name of the plugin effect or sound (set in FMOD Studio). */
    FMOD_DSP *dsp;                              /* The DSP plugin instance. This can be cast to FMOD::DSP* type. */
} FMOD_STUDIO_PLUGIN_INSTANCE_PROPERTIES;

/*
[STRUCTURE]
[
    [DESCRIPTION]
    This structure holds information about a marker on the timeline.

    [REMARKS]
    This data is passed to the event callback function when type is FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_MARKER.

    [SEE_ALSO]
    FMOD_STUDIO_EVENT_CALLBACK
    Studio::EventDescription::setCallback
    Studio::EventInstance::setCallback
]
*/
typedef struct FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES
{
    const char* name;                           /* The marker name */
    int position;                               /* The position of the marker on the timeline in milliseconds. */
} FMOD_STUDIO_TIMELINE_MARKER_PROPERTIES;

/*
[STRUCTURE]
[
    [DESCRIPTION]
    This structure holds information about a beat on the timeline.

    [REMARKS]
    This data is passed to the event callback function when type is FMOD_STUDIO_EVENT_CALLBACK_TIMELINE_BEAT.

    [SEE_ALSO]
    FMOD_STUDIO_EVENT_CALLBACK
    Studio::EventDescription::setCallback
    Studio::EventInstance::setCallback
]
*/
typedef struct FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES
{
    int bar;                                    /* The bar number (starting from 1). */
    int beat;                                   /* The beat number within the bar (starting from 1). */
    int position;                               /* The position of the beat on the timeline in milliseconds. */
    float tempo;                                /* The current tempo in beats per minute. */
    int timeSignatureUpper;                     /* The current time signature upper number (beats per bar). */
    int timeSignatureLower;                     /* The current time signature lower number (beat unit). */
} FMOD_STUDIO_TIMELINE_BEAT_PROPERTIES;

/*
[ENUM]
[
    [DESCRIPTION]
    These values describe the playback state of an event instance.

    [REMARKS]

    [SEE_ALSO]
    Studio::EventInstance::getPlaybackState
    Studio::EventInstance::start
    Studio::EventInstance::stop
    Studio::CueInstance::trigger
    Studio::ParameterInstance::setValue
]
*/
typedef enum FMOD_STUDIO_PLAYBACK_STATE
{
    FMOD_STUDIO_PLAYBACK_PLAYING,               /* Currently playing. */
    FMOD_STUDIO_PLAYBACK_SUSTAINING,            /* The timeline cursor is paused on a sustain point. */
    FMOD_STUDIO_PLAYBACK_STOPPED,               /* Not playing. */
    FMOD_STUDIO_PLAYBACK_STARTING,              /* Start has been called but the instance is not fully started yet. */
    FMOD_STUDIO_PLAYBACK_STOPPING,              /* Stop has been called but the instance is not fully stopped yet. */

    FMOD_STUDIO_PLAYBACK_FORCEINT = 65536       /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_PLAYBACK_STATE;


/*
[ENUM]
[
    [DESCRIPTION]
    Controls how to stop playback of an event instance.

    [REMARKS]

    [SEE_ALSO]
    Studio::EventInstance::stop
    Studio::Bus::stopAllEvents
]
*/
typedef enum FMOD_STUDIO_STOP_MODE
{
    FMOD_STUDIO_STOP_ALLOWFADEOUT,              /* Allows AHDSR modulators to complete their release, and DSP effect tails to play out. */
    FMOD_STUDIO_STOP_IMMEDIATE,                 /* Stops the event instance immediately. */

    FMOD_STUDIO_STOP_FORCEINT = 65536           /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_STOP_MODE;


/*
[DEFINE]
[
    [NAME]
    FMOD_STUDIO_LOAD_BANK_FLAGS

    [DESCRIPTION]
    Flags passed into Studio loadBank commands to control bank load behaviour.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::loadBankFile
    Studio::System::loadBankMemory
    Studio::System::loadBankCustom
]
*/
#define FMOD_STUDIO_LOAD_BANK_NORMAL                0x00000000         /* Standard behaviour. */
#define FMOD_STUDIO_LOAD_BANK_NONBLOCKING           0x00000001         /* Bank loading occurs asynchronously rather than occurring immediately. */
#define FMOD_STUDIO_LOAD_BANK_DECOMPRESS_SAMPLES    0x00000002         /* Force samples to decompress into memory when they are loaded, rather than staying compressed. */
/* [DEFINE_END] */

typedef unsigned int FMOD_STUDIO_LOAD_BANK_FLAGS;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Settings for advanced features like configuring memory and cpu usage.

    [REMARKS]
    Members marked with [r] mean the variable is modified by FMOD and is for reading purposes only.  Do not change this value.<br>
    Members marked with [w] mean the variable can be written to.  The user can set the value.<br>
    Members marked with [r/w] are either read or write depending on if you are using System::setAdvancedSettings (w) or System::getAdvancedSettings (r).

    [SEE_ALSO]
    Studio::System::setAdvancedSettings
    Studio::System::getAdvancedSettings
    FMOD_MODE
]
*/
typedef struct FMOD_STUDIO_ADVANCEDSETTINGS
{
    int                 cbSize;                     /* [w]   Size of this structure.  Use sizeof(FMOD_STUDIO_ADVANCEDSETTINGS)  NOTE: This must be set before calling Studio::System::getAdvancedSettings or Studio::System::setAdvancedSettings! */
    unsigned int        commandQueueSize;           /* [r/w] Optional. Specify 0 to ignore. Specify the command queue size for studio async processing.  Default 32kB. */
    unsigned int        handleInitialSize;          /* [r/w] Optional. Specify 0 to ignore. Specify the initial size to allocate for handles.  Memory for handles will grow as needed in pages. Default 8192 * sizeof(void*) */
    int                 studioUpdatePeriod;         /* [r/w] Optional. Specify 0 to ignore. Specify the update period of Studio when in async mode, in milliseconds.  Will be quantised to the nearest multiple of mixer duration.  Default is 20ms. */
} FMOD_STUDIO_ADVANCEDSETTINGS;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Performance information for FMOD Studio and low level systems.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::getCPUUsage
]
*/
typedef struct FMOD_STUDIO_CPU_USAGE
{
    float               dspUsage;                           /* Returns the % CPU time taken by DSP processing on the low level mixer thread. */
    float               streamUsage;                        /* Returns the % CPU time taken by stream processing on the low level stream thread. */
    float               geometryUsage;                      /* Returns the % CPU time taken by geometry processing on the low level geometry thread. */
    float               updateUsage;                        /* Returns the % CPU time taken by low level update, called as part of the studio update. */
    float               studioUsage;                        /* Returns the % CPU time taken by studio update, called from the studio thread. Does not include low level update time. */
} FMOD_STUDIO_CPU_USAGE;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Information for a single buffer in FMOD Studio.

    [REMARKS]

    [SEE_ALSO]
    FMOD_STUDIO_BUFFER_USAGE
]
*/
typedef struct FMOD_STUDIO_BUFFER_INFO
{
    int                 currentUsage;                       /* Current buffer usage in bytes. */
    int                 peakUsage;                          /* Peak buffer usage in bytes. */
    int                 capacity;                           /* Buffer capacity in bytes. */
    int                 stallCount;                         /* Cumulative number of stalls due to buffer overflow. */
    float               stallTime;                          /* Cumulative amount of time stalled due to buffer overflow, in seconds. */
} FMOD_STUDIO_BUFFER_INFO;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Information for FMOD Studio buffer usage.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::getBufferUsage
    Studio::System::resetBufferUsage
    FMOD_STUDIO_BUFFER_INFO
]
*/
typedef struct FMOD_STUDIO_BUFFER_USAGE
{
    FMOD_STUDIO_BUFFER_INFO studioCommandQueue;             /* Information for the Studio Async Command buffer, controlled by FMOD_STUDIO_ADVANCEDSETTINGS commandQueueSize. */
    FMOD_STUDIO_BUFFER_INFO studioHandle;                   /* Information for the Studio handle table, controlled by FMOD_STUDIO_ADVANCEDSETTINGS handleInitialSize. */
} FMOD_STUDIO_BUFFER_USAGE;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Information for loading a sound from a sound table.

    [REMARKS]
    The name_or_data member points into FMOD internal memory, which will become
    invalid if the sound table bank is unloaded.

    If mode flags such as FMOD_CREATESTREAM or FMOD_NONBLOCKING are required,
    they should be ORed together with the mode member when calling System::createSound.

    [SEE_ALSO]
    Studio::System::getSoundInfo
    System::createSound
]
*/
typedef struct FMOD_STUDIO_SOUND_INFO
{
    const char* name_or_data;           /* The filename or memory buffer that contains the sound. */
    FMOD_MODE mode;                     /* Mode flags required for loading the sound. */
    FMOD_CREATESOUNDEXINFO exinfo;      /* Extra information required for loading the sound. */
    int subsoundIndex;                  /* Subsound index for loading the sound. */
} FMOD_STUDIO_SOUND_INFO;


/*
[DEFINE]
[
    [NAME]
    FMOD_STUDIO_COMMANDCAPTURE_FLAGS

    [DESCRIPTION]
    Flags passed into Studio::System::startCommandCapture.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::startCommandCapture
]
*/
#define FMOD_STUDIO_COMMANDCAPTURE_NORMAL                      0x00000000       /* Standard behaviour. */
#define FMOD_STUDIO_COMMANDCAPTURE_FILEFLUSH                   0x00000001       /* Call file flush on every command. */
#define FMOD_STUDIO_COMMANDCAPTURE_SKIP_INITIAL_STATE          0x00000002       /* Normally the initial state of banks and instances is captured, unless this flag is set. */
/* [DEFINE_END] */

typedef unsigned int FMOD_STUDIO_COMMANDCAPTURE_FLAGS;


/*
[DEFINE]
[
    [NAME]
    FMOD_STUDIO_COMMANDREPLAY_FLAGS

    [DESCRIPTION]
    Flags passed into Studio::System::loadCommandReplay.

    [REMARKS]

    [SEE_ALSO]
    Studio::System::loadCommandReplay
]
*/
#define FMOD_STUDIO_COMMANDREPLAY_NORMAL               0x00000000       /* Standard behaviour. */
#define FMOD_STUDIO_COMMANDREPLAY_SKIP_CLEANUP         0x00000001       /* Normally the playback will release any created resources when it stops, unless this flag is set. */
/* [DEFINE_END] */

typedef unsigned int FMOD_STUDIO_COMMANDREPLAY_FLAGS;


/*
[ENUM]
[
    [DESCRIPTION]
    Used to distinguish the types used in command replays.

    [REMARKS]

    [SEE_ALSO]
]
*/
typedef enum FMOD_STUDIO_INSTANCETYPE
{
    FMOD_STUDIO_INSTANCETYPE_NONE,
    FMOD_STUDIO_INSTANCETYPE_SYSTEM,
    FMOD_STUDIO_INSTANCETYPE_EVENTDESCRIPTION,
    FMOD_STUDIO_INSTANCETYPE_EVENTINSTANCE,
    FMOD_STUDIO_INSTANCETYPE_PARAMETERINSTANCE,
    FMOD_STUDIO_INSTANCETYPE_CUEINSTANCE,
    FMOD_STUDIO_INSTANCETYPE_BUS,
    FMOD_STUDIO_INSTANCETYPE_VCA,
    FMOD_STUDIO_INSTANCETYPE_BANK,
    FMOD_STUDIO_INSTANCETYPE_COMMANDREPLAY,

    FMOD_STUDIO_INSTANCETYPE_FORCEINT = 65536    /* Makes sure this enum is signed 32bit. */
} FMOD_STUDIO_INSTANCETYPE;


/*
[STRUCTURE]
[
    [DESCRIPTION]
    Information about a single command in a command replay file.

    [REMARKS]
    This information has metadata about the command at the given index.  Note that the handle fields are
    from the recorded session, and will no longer correspond to any actual object type in the current
    system.

    [SEE_ALSO]
    Studio::CommandReplay::getCommandInfo
]
*/
typedef struct FMOD_STUDIO_COMMAND_INFO
{
    const char* commandName;                                    /* The full name of the API function for this command. */
    int parentCommandIndex;                                     /* For commands that operate on an instance, this is the command that created the instance. */
    int frameNumber;                                            /* The frame the command belongs to. */
    float frameTime;                                            /* The playback time at which this command will be executed. */
    FMOD_STUDIO_INSTANCETYPE instanceType;                      /* The type of object that this command uses as an instance. */
    FMOD_STUDIO_INSTANCETYPE outputType;                        /* The type of object that this command outputs, if any. */
    unsigned int instanceHandle;                                /* The original handle value of the instance.  This will no longer correspond to any actual object in playback. */
    unsigned int outputHandle;                                  /* The original handle value of the command output.  This will no longer correspond to any actual object in playback. */
} FMOD_STUDIO_COMMAND_INFO;


/*
    FMOD Studio callbacks.
*/
typedef FMOD_RESULT (F_CALLBACK *FMOD_STUDIO_SYSTEM_CALLBACK)                           (FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_SYSTEM_CALLBACK_TYPE type, void *commanddata, void *userdata);
typedef FMOD_RESULT (F_CALLBACK *FMOD_STUDIO_EVENT_CALLBACK)                            (FMOD_STUDIO_EVENT_CALLBACK_TYPE type, FMOD_STUDIO_EVENTINSTANCE *event, void *parameters);
typedef FMOD_RESULT (F_CALLBACK *FMOD_STUDIO_COMMANDREPLAY_FRAME_CALLBACK)              (FMOD_STUDIO_COMMANDREPLAY *replay, int commandIndex, float currentTime, void *userdata);
typedef FMOD_RESULT (F_CALLBACK *FMOD_STUDIO_COMMANDREPLAY_LOAD_BANK_CALLBACK)          (FMOD_STUDIO_COMMANDREPLAY *replay, int commandIndex, const FMOD_GUID *bankGuid, const char *bankFilename, FMOD_STUDIO_LOAD_BANK_FLAGS flags, FMOD_STUDIO_BANK **bank, void *userdata);
typedef FMOD_RESULT (F_CALLBACK *FMOD_STUDIO_COMMANDREPLAY_CREATE_INSTANCE_CALLBACK)    (FMOD_STUDIO_COMMANDREPLAY *replay, int commandIndex, FMOD_STUDIO_EVENTDESCRIPTION *eventDescription, FMOD_STUDIO_EVENTINSTANCE **instance, void *userdata);

#endif // FMOD_STUDIO_COMMON_H
