/*$ preserve start $*/

/*
    fmod_studio.h - FMOD Studio API
    Copyright (c), Firelight Technologies Pty, Ltd. 2016.

    This header defines the C API. If you are programming in C++ use fmod_studio.hpp.
*/

#ifndef FMOD_STUDIO_H
#define FMOD_STUDIO_H

#include "fmod_studio_common.h"

#ifdef __cplusplus
extern "C" 
{
#endif

/*
    Global
*/
FMOD_RESULT F_API FMOD_Studio_ParseID(const char *idString, FMOD_GUID *id);
FMOD_RESULT F_API FMOD_Studio_System_Create(FMOD_STUDIO_SYSTEM **system, unsigned int headerVersion);

/*$ preserve end $*/

/*
    System
*/
FMOD_BOOL   F_API FMOD_Studio_System_IsValid(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_SetAdvancedSettings(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_ADVANCEDSETTINGS *settings);
FMOD_RESULT F_API FMOD_Studio_System_GetAdvancedSettings(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_ADVANCEDSETTINGS *settings);
FMOD_RESULT F_API FMOD_Studio_System_Initialize(FMOD_STUDIO_SYSTEM *system, int maxchannels, FMOD_STUDIO_INITFLAGS studioflags, FMOD_INITFLAGS flags, void *extradriverdata);
FMOD_RESULT F_API FMOD_Studio_System_Release(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_Update(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_GetLowLevelSystem(FMOD_STUDIO_SYSTEM *system, FMOD_SYSTEM **lowLevelSystem);
FMOD_RESULT F_API FMOD_Studio_System_GetEvent(FMOD_STUDIO_SYSTEM *system, const char *pathOrID, FMOD_STUDIO_EVENTDESCRIPTION **event);
FMOD_RESULT F_API FMOD_Studio_System_GetBus(FMOD_STUDIO_SYSTEM *system, const char *pathOrID, FMOD_STUDIO_BUS **bus);
FMOD_RESULT F_API FMOD_Studio_System_GetVCA(FMOD_STUDIO_SYSTEM *system, const char *pathOrID, FMOD_STUDIO_VCA **vca);
FMOD_RESULT F_API FMOD_Studio_System_GetBank(FMOD_STUDIO_SYSTEM *system, const char *pathOrID, FMOD_STUDIO_BANK **bank);
FMOD_RESULT F_API FMOD_Studio_System_GetEventByID(FMOD_STUDIO_SYSTEM *system, const FMOD_GUID *id, FMOD_STUDIO_EVENTDESCRIPTION **event);
FMOD_RESULT F_API FMOD_Studio_System_GetBusByID(FMOD_STUDIO_SYSTEM *system, const FMOD_GUID *id, FMOD_STUDIO_BUS **bus);
FMOD_RESULT F_API FMOD_Studio_System_GetVCAByID(FMOD_STUDIO_SYSTEM *system, const FMOD_GUID *id, FMOD_STUDIO_VCA **vca);
FMOD_RESULT F_API FMOD_Studio_System_GetBankByID(FMOD_STUDIO_SYSTEM *system, const FMOD_GUID *id, FMOD_STUDIO_BANK **bank);
FMOD_RESULT F_API FMOD_Studio_System_GetSoundInfo(FMOD_STUDIO_SYSTEM *system, const char *key, FMOD_STUDIO_SOUND_INFO *info);
FMOD_RESULT F_API FMOD_Studio_System_LookupID(FMOD_STUDIO_SYSTEM *system, const char *path, FMOD_GUID *id);
FMOD_RESULT F_API FMOD_Studio_System_LookupPath(FMOD_STUDIO_SYSTEM *system, const FMOD_GUID *id, char *path, int size, int *retrieved);
FMOD_RESULT F_API FMOD_Studio_System_GetNumListeners(FMOD_STUDIO_SYSTEM *system, int *numlisteners);
FMOD_RESULT F_API FMOD_Studio_System_SetNumListeners(FMOD_STUDIO_SYSTEM *system, int numlisteners);
FMOD_RESULT F_API FMOD_Studio_System_GetListenerAttributes(FMOD_STUDIO_SYSTEM *system, int index, FMOD_3D_ATTRIBUTES *attributes);
FMOD_RESULT F_API FMOD_Studio_System_SetListenerAttributes(FMOD_STUDIO_SYSTEM *system, int index, FMOD_3D_ATTRIBUTES *attributes);
FMOD_RESULT F_API FMOD_Studio_System_LoadBankFile(FMOD_STUDIO_SYSTEM *system, const char *filename, FMOD_STUDIO_LOAD_BANK_FLAGS flags, FMOD_STUDIO_BANK **bank);
FMOD_RESULT F_API FMOD_Studio_System_LoadBankMemory(FMOD_STUDIO_SYSTEM *system, const char *buffer, int length, FMOD_STUDIO_LOAD_MEMORY_MODE mode, FMOD_STUDIO_LOAD_BANK_FLAGS flags, FMOD_STUDIO_BANK **bank);
FMOD_RESULT F_API FMOD_Studio_System_LoadBankCustom(FMOD_STUDIO_SYSTEM *system, const FMOD_STUDIO_BANK_INFO *info, FMOD_STUDIO_LOAD_BANK_FLAGS flags, FMOD_STUDIO_BANK **bank);
FMOD_RESULT F_API FMOD_Studio_System_RegisterPlugin(FMOD_STUDIO_SYSTEM *system, const FMOD_DSP_DESCRIPTION *description);
FMOD_RESULT F_API FMOD_Studio_System_UnregisterPlugin(FMOD_STUDIO_SYSTEM *system, const char *name);
FMOD_RESULT F_API FMOD_Studio_System_UnloadAll(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_FlushCommands(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_StartCommandCapture(FMOD_STUDIO_SYSTEM *system, const char *filename, FMOD_STUDIO_COMMANDCAPTURE_FLAGS flags);
FMOD_RESULT F_API FMOD_Studio_System_StopCommandCapture(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_LoadCommandReplay(FMOD_STUDIO_SYSTEM *system, const char *filename, FMOD_STUDIO_COMMANDREPLAY_FLAGS flags, FMOD_STUDIO_COMMANDREPLAY **replay);
FMOD_RESULT F_API FMOD_Studio_System_GetBankCount(FMOD_STUDIO_SYSTEM *system, int *count);
FMOD_RESULT F_API FMOD_Studio_System_GetBankList(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_BANK **array, int capacity, int *count);
FMOD_RESULT F_API FMOD_Studio_System_GetCPUUsage(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_CPU_USAGE *usage);
FMOD_RESULT F_API FMOD_Studio_System_GetBufferUsage(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_BUFFER_USAGE *usage);
FMOD_RESULT F_API FMOD_Studio_System_ResetBufferUsage(FMOD_STUDIO_SYSTEM *system);
FMOD_RESULT F_API FMOD_Studio_System_SetCallback(FMOD_STUDIO_SYSTEM *system, FMOD_STUDIO_SYSTEM_CALLBACK callback, FMOD_STUDIO_SYSTEM_CALLBACK_TYPE callbackmask);
FMOD_RESULT F_API FMOD_Studio_System_SetUserData(FMOD_STUDIO_SYSTEM *system, void *userdata);
FMOD_RESULT F_API FMOD_Studio_System_GetUserData(FMOD_STUDIO_SYSTEM *system, void **userdata);

/*
    EventDescription
*/
FMOD_BOOL   F_API FMOD_Studio_EventDescription_IsValid(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetID(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_GUID *id);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetPath(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, char *path, int size, int *retrieved);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetParameterCount(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, int *count);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetParameterByIndex(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, int index, FMOD_STUDIO_PARAMETER_DESCRIPTION *parameter);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetParameter(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, const char *name, FMOD_STUDIO_PARAMETER_DESCRIPTION *parameter);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetUserPropertyCount(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, int *count);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetUserPropertyByIndex(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, int index, FMOD_STUDIO_USER_PROPERTY *property);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetUserProperty(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, const char *name, FMOD_STUDIO_USER_PROPERTY *property);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetLength(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, int *length);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetMinimumDistance(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, float *distance);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetMaximumDistance(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, float *distance);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetSoundSize(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, float *size);
FMOD_RESULT F_API FMOD_Studio_EventDescription_IsOneshot(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_BOOL *oneshot);
FMOD_RESULT F_API FMOD_Studio_EventDescription_IsStream(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_BOOL *isStream);
FMOD_RESULT F_API FMOD_Studio_EventDescription_Is3D(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_BOOL *is3D);        
FMOD_RESULT F_API FMOD_Studio_EventDescription_CreateInstance(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_STUDIO_EVENTINSTANCE **instance);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetInstanceCount(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, int *count);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetInstanceList(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_STUDIO_EVENTINSTANCE **array, int capacity, int *count);
FMOD_RESULT F_API FMOD_Studio_EventDescription_LoadSampleData(FMOD_STUDIO_EVENTDESCRIPTION *eventDesc);
FMOD_RESULT F_API FMOD_Studio_EventDescription_UnloadSampleData(FMOD_STUDIO_EVENTDESCRIPTION *eventDesc);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetSampleLoadingState(FMOD_STUDIO_EVENTDESCRIPTION *eventDesc, FMOD_STUDIO_LOADING_STATE *state);
FMOD_RESULT F_API FMOD_Studio_EventDescription_ReleaseAllInstances(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription);
FMOD_RESULT F_API FMOD_Studio_EventDescription_SetCallback(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, FMOD_STUDIO_EVENT_CALLBACK callback, FMOD_STUDIO_EVENT_CALLBACK_TYPE callbackmask);
FMOD_RESULT F_API FMOD_Studio_EventDescription_GetUserData(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, void **userData);
FMOD_RESULT F_API FMOD_Studio_EventDescription_SetUserData(FMOD_STUDIO_EVENTDESCRIPTION *eventdescription, void *userData);   

/*
    EventInstance
*/
FMOD_BOOL   F_API FMOD_Studio_EventInstance_IsValid(FMOD_STUDIO_EVENTINSTANCE *eventinstance);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetDescription(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_STUDIO_EVENTDESCRIPTION **description);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetVolume(FMOD_STUDIO_EVENTINSTANCE *eventinstance, float *volume);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetVolume(FMOD_STUDIO_EVENTINSTANCE *eventinstance, float volume);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetPitch(FMOD_STUDIO_EVENTINSTANCE *eventinstance, float *pitch);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetPitch(FMOD_STUDIO_EVENTINSTANCE *eventinstance, float pitch);
FMOD_RESULT F_API FMOD_Studio_EventInstance_Get3DAttributes(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_3D_ATTRIBUTES *attributes);
FMOD_RESULT F_API FMOD_Studio_EventInstance_Set3DAttributes(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_3D_ATTRIBUTES *attributes);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetProperty(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_STUDIO_EVENT_PROPERTY index, float *value);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetProperty(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_STUDIO_EVENT_PROPERTY index, float value);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetPaused(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_BOOL *paused);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetPaused(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_BOOL paused);
FMOD_RESULT F_API FMOD_Studio_EventInstance_Start(FMOD_STUDIO_EVENTINSTANCE *eventinstance);
FMOD_RESULT F_API FMOD_Studio_EventInstance_Stop(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_STUDIO_STOP_MODE mode);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetTimelinePosition(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int *position);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetTimelinePosition(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int position);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetPlaybackState(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_STUDIO_PLAYBACK_STATE *state);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetChannelGroup(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_CHANNELGROUP **group);
FMOD_RESULT F_API FMOD_Studio_EventInstance_Release(FMOD_STUDIO_EVENTINSTANCE *eventinstance);
FMOD_RESULT F_API FMOD_Studio_EventInstance_IsVirtual(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_BOOL *virtualState);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetParameter(FMOD_STUDIO_EVENTINSTANCE *eventinstance, const char *name, FMOD_STUDIO_PARAMETERINSTANCE **parameter);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetParameterByIndex(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int index, FMOD_STUDIO_PARAMETERINSTANCE **parameter);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetParameterCount(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int *count);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetParameterValue(FMOD_STUDIO_EVENTINSTANCE *eventinstance, const char *name, float value);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetParameterValueByIndex(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int index, float value);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetCue(FMOD_STUDIO_EVENTINSTANCE *eventinstance, const char *name, FMOD_STUDIO_CUEINSTANCE **cue);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetCueByIndex(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int index, FMOD_STUDIO_CUEINSTANCE **cue);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetCueCount(FMOD_STUDIO_EVENTINSTANCE *eventinstance, int *count);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetCallback(FMOD_STUDIO_EVENTINSTANCE *eventinstance, FMOD_STUDIO_EVENT_CALLBACK callback, FMOD_STUDIO_EVENT_CALLBACK_TYPE callbackmask);
FMOD_RESULT F_API FMOD_Studio_EventInstance_GetUserData(FMOD_STUDIO_EVENTINSTANCE *eventinstance, void **userData);
FMOD_RESULT F_API FMOD_Studio_EventInstance_SetUserData(FMOD_STUDIO_EVENTINSTANCE *eventinstance, void *userData);

/*
    CueInstance
*/
FMOD_BOOL   F_API FMOD_Studio_CueInstance_IsValid(FMOD_STUDIO_CUEINSTANCE *cueinstance);
FMOD_RESULT F_API FMOD_Studio_CueInstance_Trigger(FMOD_STUDIO_CUEINSTANCE *cueinstance);

/*
    ParameterInstance
*/
FMOD_BOOL   F_API FMOD_Studio_ParameterInstance_IsValid(FMOD_STUDIO_PARAMETERINSTANCE *parameterinstance);
FMOD_RESULT F_API FMOD_Studio_ParameterInstance_GetDescription(FMOD_STUDIO_PARAMETERINSTANCE *parameterinstance, FMOD_STUDIO_PARAMETER_DESCRIPTION *description);
FMOD_RESULT F_API FMOD_Studio_ParameterInstance_GetValue(FMOD_STUDIO_PARAMETERINSTANCE *parameterinstance, float *value);
FMOD_RESULT F_API FMOD_Studio_ParameterInstance_SetValue(FMOD_STUDIO_PARAMETERINSTANCE *parameterinstance, float value);

/*
    Bus
*/
FMOD_BOOL   F_API FMOD_Studio_Bus_IsValid(FMOD_STUDIO_BUS *bus);
FMOD_RESULT F_API FMOD_Studio_Bus_GetID(FMOD_STUDIO_BUS *bus, FMOD_GUID *id);
FMOD_RESULT F_API FMOD_Studio_Bus_GetPath(FMOD_STUDIO_BUS *bus, char *path, int size, int *retrieved);
FMOD_RESULT F_API FMOD_Studio_Bus_GetFaderLevel(FMOD_STUDIO_BUS *bus, float *level);
FMOD_RESULT F_API FMOD_Studio_Bus_SetFaderLevel(FMOD_STUDIO_BUS *bus, float level);
FMOD_RESULT F_API FMOD_Studio_Bus_GetPaused(FMOD_STUDIO_BUS *bus, FMOD_BOOL *paused);
FMOD_RESULT F_API FMOD_Studio_Bus_SetPaused(FMOD_STUDIO_BUS *bus, FMOD_BOOL paused);
FMOD_RESULT F_API FMOD_Studio_Bus_GetMute(FMOD_STUDIO_BUS *bus, FMOD_BOOL *mute);
FMOD_RESULT F_API FMOD_Studio_Bus_SetMute(FMOD_STUDIO_BUS *bus, FMOD_BOOL mute);
FMOD_RESULT F_API FMOD_Studio_Bus_StopAllEvents(FMOD_STUDIO_BUS *bus, FMOD_STUDIO_STOP_MODE mode);
FMOD_RESULT F_API FMOD_Studio_Bus_LockChannelGroup(FMOD_STUDIO_BUS *bus);
FMOD_RESULT F_API FMOD_Studio_Bus_UnlockChannelGroup(FMOD_STUDIO_BUS *bus);
FMOD_RESULT F_API FMOD_Studio_Bus_GetChannelGroup(FMOD_STUDIO_BUS *bus, FMOD_CHANNELGROUP **group);

/*
    VCA
*/
FMOD_BOOL   F_API FMOD_Studio_VCA_IsValid(FMOD_STUDIO_VCA *vca);
FMOD_RESULT F_API FMOD_Studio_VCA_GetID(FMOD_STUDIO_VCA *vca, FMOD_GUID *id);
FMOD_RESULT F_API FMOD_Studio_VCA_GetPath(FMOD_STUDIO_VCA *vca, char *path, int size, int *retrieved);
FMOD_RESULT F_API FMOD_Studio_VCA_GetFaderLevel(FMOD_STUDIO_VCA *vca, float *level);
FMOD_RESULT F_API FMOD_Studio_VCA_SetFaderLevel(FMOD_STUDIO_VCA *vca, float level);

/*
    Bank
*/
FMOD_BOOL   F_API FMOD_Studio_Bank_IsValid(FMOD_STUDIO_BANK *bank);
FMOD_RESULT F_API FMOD_Studio_Bank_GetID(FMOD_STUDIO_BANK *bank, FMOD_GUID *id);
FMOD_RESULT F_API FMOD_Studio_Bank_GetPath(FMOD_STUDIO_BANK *bank, char *path, int size, int *retrieved);
FMOD_RESULT F_API FMOD_Studio_Bank_Unload(FMOD_STUDIO_BANK *bank);
FMOD_RESULT F_API FMOD_Studio_Bank_LoadSampleData(FMOD_STUDIO_BANK *bank);
FMOD_RESULT F_API FMOD_Studio_Bank_UnloadSampleData(FMOD_STUDIO_BANK *bank);
FMOD_RESULT F_API FMOD_Studio_Bank_GetLoadingState(FMOD_STUDIO_BANK *bank, FMOD_STUDIO_LOADING_STATE *state);
FMOD_RESULT F_API FMOD_Studio_Bank_GetSampleLoadingState(FMOD_STUDIO_BANK *bank, FMOD_STUDIO_LOADING_STATE *state);
FMOD_RESULT F_API FMOD_Studio_Bank_GetStringCount(FMOD_STUDIO_BANK *bank, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetStringInfo(FMOD_STUDIO_BANK *bank, int index, FMOD_GUID *id, char *path, int size, int *retrieved);
FMOD_RESULT F_API FMOD_Studio_Bank_GetEventCount(FMOD_STUDIO_BANK *bank, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetEventList(FMOD_STUDIO_BANK *bank, FMOD_STUDIO_EVENTDESCRIPTION **array, int capacity, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetBusCount(FMOD_STUDIO_BANK *bank, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetBusList(FMOD_STUDIO_BANK *bank, FMOD_STUDIO_BUS **array, int capacity, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetVCACount(FMOD_STUDIO_BANK *bank, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetVCAList(FMOD_STUDIO_BANK *bank, FMOD_STUDIO_VCA **array, int capacity, int *count);
FMOD_RESULT F_API FMOD_Studio_Bank_GetUserData(FMOD_STUDIO_BANK *bank, void **userData);
FMOD_RESULT F_API FMOD_Studio_Bank_SetUserData(FMOD_STUDIO_BANK *bank, void *userData);

/*
    Command playback information
*/
FMOD_BOOL   F_API FMOD_Studio_CommandReplay_IsValid(FMOD_STUDIO_COMMANDREPLAY *replay);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetSystem(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_STUDIO_SYSTEM **system);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetLength(FMOD_STUDIO_COMMANDREPLAY *replay, float *length);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetCommandCount(FMOD_STUDIO_COMMANDREPLAY *replay, int *count);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetCommandInfo(FMOD_STUDIO_COMMANDREPLAY *replay, int commandIndex, FMOD_STUDIO_COMMAND_INFO *info);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetCommandString(FMOD_STUDIO_COMMANDREPLAY *replay, int commandIndex, char *description, int capacity);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetCommandAtTime(FMOD_STUDIO_COMMANDREPLAY *replay, float time, int *commandIndex);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SetBankPath(FMOD_STUDIO_COMMANDREPLAY *replay, const char *bankPath);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_Start(FMOD_STUDIO_COMMANDREPLAY *replay);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_Stop(FMOD_STUDIO_COMMANDREPLAY *replay);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SeekToTime(FMOD_STUDIO_COMMANDREPLAY *replay, float time);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SeekToCommand(FMOD_STUDIO_COMMANDREPLAY *replay, int commandIndex);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetPaused(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_BOOL *paused);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SetPaused(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_BOOL paused);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetPlaybackState(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_STUDIO_PLAYBACK_STATE *state);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetCurrentCommand(FMOD_STUDIO_COMMANDREPLAY *replay, int *commandIndex, float *currentTime);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_Release(FMOD_STUDIO_COMMANDREPLAY *replay);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SetFrameCallback(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_STUDIO_COMMANDREPLAY_FRAME_CALLBACK callback);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SetLoadBankCallback(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_STUDIO_COMMANDREPLAY_LOAD_BANK_CALLBACK callback);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SetCreateInstanceCallback(FMOD_STUDIO_COMMANDREPLAY *replay, FMOD_STUDIO_COMMANDREPLAY_CREATE_INSTANCE_CALLBACK callback);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_GetUserData(FMOD_STUDIO_COMMANDREPLAY *replay, void **userdata);
FMOD_RESULT F_API FMOD_Studio_CommandReplay_SetUserData(FMOD_STUDIO_COMMANDREPLAY *replay, void *userdata);

/*$ preserve start $*/

#ifdef __cplusplus
}
#endif

#endif /* FMOD_STUDIO_H */

/*$ preserve end $*/
