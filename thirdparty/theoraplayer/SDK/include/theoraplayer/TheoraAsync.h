/************************************************************************************
This source file is part of the Theora Video Playback Library
For latest info, see http://libtheoraplayer.googlecode.com
*************************************************************************************
Copyright (c) 2008-2014 Kresimir Spes (kspes@cateia.com)
This program is free software; you can redistribute it and/or modify it under
the terms of the BSD license: http://opensource.org/licenses/BSD-3-Clause
*************************************************************************************/
#ifndef _TheoraAsync_h
#define _TheoraAsync_h

#include <stdlib.h>
#ifndef _WIN32
#include <pthread.h>
#endif

/// @note Based on hltypes::Thread
class TheoraMutex
{
public:
	class ScopeLock
	{
	public:
		ScopeLock(TheoraMutex* mutex = NULL, bool logUnhandledUnlocks = true);
		~ScopeLock();
		bool acquire(TheoraMutex* mutex);
		bool release();

	protected:
		TheoraMutex* mMutex;
		bool mLogUnhandledUnlocks;

	};

	TheoraMutex();
	~TheoraMutex();
	void lock();
	void unlock();
		
protected:
	void* mHandle;
		
};

/// @note Based on hltypes::Thread
class TheoraThread
{
	TheoraMutex mRunningMutex;
public:
	TheoraThread();
	virtual ~TheoraThread();
	void start();
	void stop();
	void resume();
	void pause();
	bool isRunning();
	virtual void execute() = 0;
	void join();
		
protected:
	void* mId;
	volatile bool mRunning;
		
};

#endif
