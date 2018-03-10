/*
 * Copyright (c) 2008-2015, NVIDIA CORPORATION.  All rights reserved.
 *
 * NVIDIA CORPORATION and its licensors retain all intellectual property
 * and proprietary rights in and to this software, related documentation
 * and any modifications thereto.  Any use, reproduction, disclosure or
 * distribution of this software and related documentation without an express
 * license agreement from NVIDIA CORPORATION is strictly prohibited.
 */


#include "InternalRenderDebug.h"
#include "PxRenderDebug.h"
#include "PxProcessRenderDebug.h"
#include "PxMemoryBuffer.h"

#pragma warning(disable:4996)
#pragma warning(disable:4100)
#pragma warning(disable:4189)


#include "PsUserAllocated.h"
#include "PsString.h"
#include "PsMathUtils.h"
#include "PsPool.h"
#include "PxIOStream.h"


#ifdef WITHOUT_DEBUG_VISUALIZE

namespace physx
{
	namespace general_renderdebug4
	{

RenderDebug * createInternalRenderDebug(void)
{
	return NULL;
}

void releaseInternalRenderDebug(RenderDebug * /*n*/)
{
}


}
};


#else

namespace physx
{
	namespace general_renderdebug4
	{

#define RENDER_STATE_STACK_SIZE 16 // maximum size of the render state stack
const PxF32 FM_PI = 3.1415926535897932384626433832795028841971693993751f;
const PxF32 FM_DEG_TO_RAD = ((2.0f * FM_PI) / 360.0f);
//const PxF32 FM_RAD_TO_DEG = (360.0f / (2.0f * FM_PI));

#define MAX_SEND_BUFFER 512 // send up to 512 at a time.

class PostRenderDebug
{
public:

	PostRenderDebug(void) 
	{
		mSendBufferCount = 0;
		// since all of these commands are processed internally, in memory buffers, we don't need to be endian specific.
		mWorldSpace.setEndianMode(PxFileBuf::ENDIAN_NONE);
		mScreenSpace.setEndianMode(PxFileBuf::ENDIAN_NONE);
		mNoZ.setEndianMode(PxFileBuf::ENDIAN_NONE);
	}

	~PostRenderDebug(void)
	{
	}

	PX_INLINE void postRenderDebug(DebugPrimitive *p,const RenderState &rs)
	{
//		if ( p->mCommand == DebugCommand::DEBUG_GRAPH )
//			return;

		PxU32 plen = DebugCommand::getPrimtiveSize(*p);
		PxF32 dtime = rs.getDisplayTime();

		if ( p->mCommand == DebugCommand::DEBUG_GRAPH )
		{
			DebugGraphStream *dgs = static_cast< DebugGraphStream *>(p);
			plen = dgs->mSize;
			p = (DebugPrimitive *)dgs->mBuffer;
			dgs = static_cast< DebugGraphStream *>(p);
			dgs->mSize = plen;
		}

		if ( rs.isScreen() || p->mCommand == DebugCommand::DEBUG_GRAPH )
		{
			processState(rs,mScreenSpaceRenderState,mScreenSpace,dtime);
			mScreenSpace.write(&dtime,sizeof(dtime));
			mScreenSpace.write(p,plen);
		}
		else if ( rs.isUseZ() )
		{
			processState(rs,mWorldSpaceRenderState,mWorldSpace,dtime);
			mWorldSpace.write(&dtime,sizeof(dtime));
			mWorldSpace.write(p,plen);
		}
		else
		{
			processState(rs,mNoZRenderState,mNoZ,dtime);
			mNoZ.write(&dtime,sizeof(dtime));
			mNoZ.write(p,plen);
		}
	}

	void release(void)
	{
		delete this;
	}


	bool processRenderDebug(PxProcessRenderDebug *rd,PxF32 dtime,RenderDebugInterface *iface)
	{

		PX_ASSERT(dtime>=0);
		process(mWorldSpace,rd,dtime,iface,PxProcessRenderDebug::WORLD_SPACE);
		process(mNoZ,rd,dtime,iface,PxProcessRenderDebug::WORLD_SPACE_NOZ);
		process(mScreenSpace,rd,dtime,iface,PxProcessRenderDebug::SCREEN_SPACE);
		rd->flushFrame(iface);

		new ( &mWorldSpaceRenderState ) RenderState;
		new ( &mScreenSpaceRenderState ) RenderState;
		new ( &mNoZRenderState ) RenderState;

		return true;
	}

	PX_INLINE bool intersects(const PxU8 *a1,const PxU8 *a2,const PxU8 *b1,const PxU8 *b2)
	{
		bool ret = true;
		if ( a1 >= b2 || b1 >= a2 ) ret = false;
		return ret;
	}

	// This method processes the current stream of draw commands.
	// Each draw command is preceeded by its 'lifetime'.
	// If the object is still alive after it is processed, it is retained in the buffer.
	void process(PxMemoryBuffer &stream,PxProcessRenderDebug *rd,PxF32 dtime,RenderDebugInterface *iface,PxProcessRenderDebug::DisplayType type)
	{
		if ( stream.tellWrite() == 0 ) return;

		PxU8 *dest = (PxU8 *)stream.getWriteBuffer();

		const PxU8 *sendStart = NULL;
		const PxU8 *sendEnd   = NULL;

		PxF32 displayTime;
		PxU32 r = stream.read(&displayTime,sizeof(displayTime));

		BlockInfo *info = NULL;

		while ( r == sizeof(displayTime) )
		{
			displayTime-=dtime;
			bool alive = displayTime > 0.0f;
			PxU32 cmd;
			r = stream.peek(&cmd,sizeof(cmd));

			PX_ASSERT( r == sizeof(cmd) );
			PX_ASSERT( cmd < DebugCommand::LAST );

			const PxU8 *readLoc = stream.getReadLoc();
			DebugPrimitive *dp = (DebugPrimitive *)readLoc;
			PxU32 plen = DebugCommand::getPrimtiveSize(*dp);
			stream.advanceReadLoc(plen);

			if ( sendStart == NULL )
			{
				sendStart = readLoc;
			}
			sendEnd = readLoc+plen;

			if ( dp->mCommand == DebugCommand::DEBUG_BLOCK_INFO )
			{
				DebugBlockInfo *db = static_cast< DebugBlockInfo *>(dp);
				info = db->mInfo;

				if ( info && info->mChanged )
				{
					db->mCurrentTransform = info->mPose;
					info->mChanged = false;
				}
				if ( info && info->mVisibleState )
				{
					DebugPrimitive *setCurrentTransform = static_cast< DebugPrimitive *>(&db->mCurrentTransform);
					mSendBuffer[mSendBufferCount] = setCurrentTransform;
					mSendBufferCount++;
				}
			}
			else if ( info )
			{
				if ( info->mVisibleState )
				{
					mSendBuffer[mSendBufferCount] = dp;
					mSendBufferCount++;
				}
				else if (isRenderState(dp))
				{
					mSendBuffer[mSendBufferCount] = dp;
					mSendBufferCount++; 
				}
			}
			else
			{
				mSendBuffer[mSendBufferCount] = dp;
				mSendBufferCount++;
			}

   			if ( mSendBufferCount == MAX_SEND_BUFFER )
   			{
   				rd->processRenderDebug(mSendBuffer,mSendBufferCount,iface,type);
   				mSendBufferCount = 0;
   				sendStart = sendEnd = NULL;
   			}

   			if ( alive )
   			{
				if ( sendStart && intersects(sendStart,sendEnd,dest,dest+plen+sizeof(PxF32)) )
				{
					if ( mSendBufferCount )
						rd->processRenderDebug(mSendBuffer,mSendBufferCount,iface,type);
					mSendBufferCount = 0;
					sendStart = sendEnd = NULL;
				}

				// Store the new display time!
				PxF32 *fdest = (PxF32 *)dest;
				*fdest = displayTime;
				dest+=sizeof(PxF32);

   				if ( dest != (PxU8 *)dp ) // if the source and dest are the same, we don't need to move memory
   				{
 					memcpy(dest,dp,plen);
				}
				dest+=plen;
			}

			r = stream.read(&displayTime,sizeof(displayTime));
		}

		if ( mSendBufferCount )
		{
			rd->processRenderDebug(mSendBuffer,mSendBufferCount,iface,type);
			mSendBufferCount = 0;
		}

		if ( info )
			info->mChanged = false;

		stream.setWriteLoc(dest);
		rd->flush(iface,type);
		stream.seekRead(0);
	}

	void reset(BlockInfo *info)
	{
		reset(mWorldSpace,info);
		reset(mNoZ,info);
		reset(mScreenSpace,info);
	}

	// if resetInfo is NULL, then totally erase the persistent data stream.
	// if resetInfo is non-NULL, then erease all parts of the stream which are associated with this data block.
	void reset(PxMemoryBuffer &stream,BlockInfo *resetInfo)
	{
   		if ( stream.tellWrite() == 0 ) return;
   		// remove all data
		if ( resetInfo )
		{
    		PxU8 *dest = (PxU8 *)stream.getWriteBuffer();
    		PxF32 displayTime;
    		PxU32 r = stream.read(&displayTime,sizeof(displayTime));
    		BlockInfo *info = NULL;
    		while ( r == sizeof(displayTime) )
    		{
				char scratch[sizeof(DebugGraphStream)];
    			DebugGraphStream *dgs = (DebugGraphStream *)scratch;
    			r = stream.peek(dgs,sizeof(DebugGraphStream));
    			PX_ASSERT( r >= 8);
    			PX_ASSERT( dgs->mCommand < DebugCommand::LAST );
    			PxU32 plen = DebugCommand::getPrimtiveSize(*dgs);
    			const PxU8 *readLoc = stream.getReadLoc();
    			DebugPrimitive *dp = (DebugPrimitive *)readLoc;
    			stream.advanceReadLoc(plen);
    			if ( dp->mCommand == DebugCommand::DEBUG_BLOCK_INFO )
    			{
    				DebugBlockInfo *db = static_cast< DebugBlockInfo *>(dp);
    				info = db->mInfo;
    			}
       			if ( info != resetInfo )
       			{
    				// Store the new display time!
    				PxF32 *fdest = (PxF32 *)dest;
    				*fdest = displayTime;
    				dest+=sizeof(PxF32);
       				if ( dest != (PxU8 *)dp ) // if the source and dest are the same, we don't need to move memory
       				{
     					memcpy(dest,dp,plen);
    				}
    				dest+=plen;

    			}
    			r = stream.read(&displayTime,sizeof(displayTime));
    		}
    		stream.setWriteLoc(dest);
    		stream.seekRead(0);
		}
		else
		{
			// kill all saved data!
			PxU8 *dest = (PxU8 *)stream.getWriteBuffer();
			stream.setWriteLoc(dest);
			stream.seekRead(0);
		}
	}

	void updateBufferChangeCount(const RenderState &rs)
	{
		if ( rs.isScreen() /*|| p->mCommand == DebugCommand::DEBUG_GRAPH*/ )
			mScreenSpaceRenderState.mChangeCount = rs.getChangeCount();
		else if ( rs.isUseZ() )
			mWorldSpaceRenderState.mChangeCount = rs.getChangeCount();
		else
			mNoZRenderState.mChangeCount = rs.getChangeCount();
	}

	void updatePostDrawGroupPose (const RenderState &rs)
	{
		PxF32 dtime = rs.getDisplayTime();

		if ( rs.isScreen() /*|| p->mCommand == DebugCommand::DEBUG_GRAPH*/ )
			setCurrentTransform(rs, mScreenSpaceRenderState, mScreenSpace, dtime);
		else if ( rs.isUseZ() )
			setCurrentTransform(rs, mWorldSpaceRenderState, mWorldSpace, dtime);
		else
			setCurrentTransform(rs, mNoZRenderState, mNoZ, dtime);
	}

private:

	PX_INLINE void processState(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		if ( dest.getChangeCount() == 0 )
		{
			setColor(source,dest,stream,lifeTime);
			setArrowColor(source,dest,stream,lifeTime);
			setArrowSize(source,dest,stream,lifeTime);
			setRenderScale(source,dest,stream,lifeTime);
			setUserId(source,dest,stream,lifeTime);
			setUserPointer(source,dest,stream,lifeTime);
			setStates(source,dest,stream,lifeTime);
			setTextScale(source,dest,stream,lifeTime);
			setCurrentTransform(source,dest,stream,lifeTime);
			setBlockInfo(source,dest,stream,lifeTime);
			dest.incrementChangeCount();
		}
		else if ( source.getChangeCount() != dest.getChangeCount() )
		{
			applyStateChanges(source,dest,stream,lifeTime);
		}
	}

	PX_INLINE void setColor(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveU32 d(DebugCommand::SET_CURRENT_COLOR,source.mColor);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mColor = source.mColor;
	}

	PX_INLINE void setRenderScale(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveF32 d(DebugCommand::SET_CURRENT_RENDER_SCALE,source.mRenderScale);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mRenderScale = source.mRenderScale;
	}

	PX_INLINE void setArrowSize(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveF32 d(DebugCommand::SET_CURRENT_ARROW_SIZE,source.mArrowSize);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mArrowSize = source.mArrowSize;
	}

	PX_INLINE void setUserId(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveU32 d(DebugCommand::SET_CURRENT_USER_ID, (physx::PxU32)source.mUserId);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mUserId = source.mUserId;
	}

	PX_INLINE void setUserPointer(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveU64 d(DebugCommand::SET_CURRENT_USER_POINTER,(PxU64)(size_t)source.mUserPtr);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mUserPtr = source.mUserPtr;
	}


	PX_INLINE void setArrowColor(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveU32 d(DebugCommand::SET_CURRENT_ARROW_COLOR,source.mArrowColor);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mArrowColor = source.mArrowColor;
	}



	PX_INLINE void setStates(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveU32 d(DebugCommand::SET_CURRENT_RENDER_STATE,source.mStates);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mStates = source.mStates;
	}

	PX_INLINE void setTextScale(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugPrimitiveF32 d(DebugCommand::SET_CURRENT_TEXT_SCALE,source.mTextScale);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mTextScale = source.mTextScale;
	}

	PX_INLINE void setCurrentTransform(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugSetCurrentTransform d(source.mPose ? *source.mPose : PxMat44::createIdentity() );
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		if ( source.mPose )
		{
			dest.mCurrentPose = *source.mPose;
			dest.mPose = &dest.mCurrentPose;
		}
		else
		{
			dest.mPose = NULL;
		}
	}

	PX_INLINE void setBlockInfo(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		DebugBlockInfo d(source.mBlockInfo);
		stream.write(&lifeTime,sizeof(lifeTime));
		stream.write(&d,sizeof(d));
		dest.mBlockInfo = source.mBlockInfo;
	}


	void applyStateChanges(const RenderState &source,RenderState &dest,PxMemoryBuffer &stream,PxF32 lifeTime)
	{
		if ( source.mColor != dest.mColor )
		{
			setColor(source,dest,stream,lifeTime);
		}
		if ( source.mArrowColor != dest.mArrowColor )
		{
			setArrowColor(source,dest,stream,lifeTime);
		}
		if ( source.mRenderScale != dest.mRenderScale )
		{
			setRenderScale(source,dest,stream,lifeTime);
		}
		if ( source.mArrowSize != dest.mArrowSize )
		{
			setArrowSize(source,dest,stream,lifeTime);
		}
		if ( source.mUserId != dest.mUserId )
		{
			setUserId(source,dest,stream,lifeTime);
		}
		if ( source.mUserPtr != dest.mUserPtr )
		{
			setUserPointer(source,dest,stream,lifeTime);
		}
		if ( source.mStates != dest.mStates )
		{
			setStates(source,dest,stream,lifeTime);
		}
		if ( source.mTextScale != dest.mTextScale )
		{
			setTextScale(source,dest,stream,lifeTime);
		}
		if ( source.mPose != dest.mPose )
		{
			if ( !sameMat44(source.mPose,dest.mPose) )
			{
				setCurrentTransform(source,dest,stream,lifeTime);
    		}
		}
		if ( source.mBlockInfo != dest.mBlockInfo )
		{
			setBlockInfo(source,dest,stream,lifeTime);
		}
		dest.mChangeCount = source.mChangeCount;
	}

	bool sameMat44(const PxMat44 *a,const PxMat44 *b) const
	{
		if ( a == b ) return true;
		if ( a == NULL || b == NULL ) return false;
		const PxF32 *f1 = (const PxF32 *)a->front();
		const PxF32 *f2 = (const PxF32 *)b->front();
		for (PxU32 i=0; i<16; i++)
		{
			PxF32 diff = PxAbs(f1[i]-f2[i]);
			if ( diff > 0.000001f ) return false;
		}
		return true;
	}

	bool isRenderState(const DebugPrimitive * const dp) const
	{
		return( dp->mCommand == DebugCommand::SET_CURRENT_COLOR || 
				dp->mCommand == DebugCommand::SET_CURRENT_ARROW_COLOR ||
				dp->mCommand == DebugCommand::SET_CURRENT_ARROW_SIZE ||
				dp->mCommand == DebugCommand::SET_CURRENT_RENDER_SCALE ||
				dp->mCommand == DebugCommand::SET_CURRENT_USER_ID ||
				dp->mCommand == DebugCommand::SET_CURRENT_USER_POINTER ||
				dp->mCommand == DebugCommand::SET_CURRENT_RENDER_STATE ||
				dp->mCommand == DebugCommand::SET_CURRENT_TEXT_SCALE ||
				dp->mCommand == DebugCommand::SET_CURRENT_TRANSFORM );
	}

	PxMemoryBuffer	mWorldSpace;
	PxMemoryBuffer  mScreenSpace;
	PxMemoryBuffer	mNoZ;

	RenderState		mWorldSpaceRenderState;
	RenderState		mScreenSpaceRenderState;
	RenderState		mNoZRenderState;

	PxU32			mSendBufferCount;
	const DebugPrimitive	*mSendBuffer[MAX_SEND_BUFFER];
};




class InternalRenderDebug : public RenderDebug, public UserAllocated, PostRenderDebug
{
public:

	InternalRenderDebug(PxProcessRenderDebug *process)
	{
		mFrameTime = 1.0f / 60.0f;
		mBlockIndex = 0;
		mStackIndex = 0;
		mUpdateCount = 0;
		mOwnProcess = false;
		if ( process )
		{
			mProcessRenderDebug = process;
		}
		else
		{
			mProcessRenderDebug = createProcessRenderDebug();
			mOwnProcess = true;
		}
		initColors();
	}

	~InternalRenderDebug(void)
	{
		if ( mOwnProcess )
		{
			mProcessRenderDebug->release();
		}
	}

	virtual PxU32 getUpdateCount(void) const
	{
		return mUpdateCount;
	}

    virtual bool render(PxF32 dtime,RenderDebugInterface *iface)
    {
    	bool ret = false;

		ret = PostRenderDebug::processRenderDebug(mProcessRenderDebug,dtime,iface);
		mUpdateCount++;

    	return ret;
    }

	virtual void  reset(PxI32 blockIndex=-1)  // -1 reset *everything*, 0 = reset everything except stuff inside blocks, > 0 reset a specific block of data.
	{
		if ( blockIndex == -1 )
		{
			PostRenderDebug::reset(NULL);
		}
		else
		{
			const HashMap<PxU32, BlockInfo *>::Entry *e = mBlocksHash.find((PxU32)blockIndex);
			if ( e )
			{
				BlockInfo *b = e->second;
				PostRenderDebug::reset(b);
			}
		}
	}


	virtual void  drawGrid(bool zup=false,PxU32 gridSize=40)  // draw a grid.
	{
		DrawGrid d(zup,gridSize);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  pushRenderState(void)
	{
		PX_ASSERT( mStackIndex < RENDER_STATE_STACK_SIZE );
		if ( mStackIndex < RENDER_STATE_STACK_SIZE )
		{
			mRenderStateStack[mStackIndex] = mCurrentState;
			if ( mCurrentState.mPose )
			{
				mRenderStateStack[mStackIndex].mPose = &mRenderStateStack[mStackIndex].mCurrentPose;
			}
			mStackIndex++;
		}
	}

    virtual void  popRenderState(void)
    {
		PX_ASSERT(mStackIndex);
		if ( mStackIndex > 0 )
		{
			mStackIndex--;
			mCurrentState = mRenderStateStack[mStackIndex];
			if ( mRenderStateStack[mStackIndex].mPose )
			{
				mCurrentState.mPose = &mCurrentState.mCurrentPose;
			}
			PostRenderDebug::updateBufferChangeCount(mCurrentState);
			mCurrentState.incrementChangeCount();
		}
    }

	virtual void  setCurrentColor(PxU32 color=0xFFFFFF,PxU32 arrowColor=0xFF0000)
	{
		mCurrentState.mColor = color;
		mCurrentState.mArrowColor = arrowColor;
		mCurrentState.incrementChangeCount();
	}

	virtual PxU32 getCurrentColor(void) const
	{
		return mCurrentState.mColor;
	}

	virtual PxU32 getCurrentArrowColor(void) const
	{
		return mCurrentState.mArrowColor;
	}


	virtual void  setCurrentUserPointer(void *userPtr)
	{
		mCurrentState.mUserPtr = userPtr;
		mCurrentState.incrementChangeCount();
	}

	virtual void  setCurrentUserId(PxI32 userId)
	{
		mCurrentState.mUserId = userId;
		mCurrentState.incrementChangeCount();
	}


	virtual void * getCurrentUserPointer(void) const
	{
		return mCurrentState.mUserPtr;
	}

	virtual PxI32 getCurrentUserId(void)
	{
		return mCurrentState.mUserId;
	}

	virtual void  setCurrentDisplayTime(PxF32 displayTime=0.0001f)
	{
		mCurrentState.mDisplayTime = displayTime;
		mCurrentState.incrementChangeCount();
	}

	virtual PxF32 getRenderScale(void)
	{
		return mCurrentState.mRenderScale;
	}

	virtual void  setRenderScale(PxF32 scale)
	{
		mCurrentState.mRenderScale = scale;
		mCurrentState.incrementChangeCount();
	}

	virtual void  setCurrentState(PxU32 states=0)
	{
		mCurrentState.mStates = states;
		mCurrentState.incrementChangeCount();
	}

	virtual void  addToCurrentState(DebugRenderState::Enum state)  // OR this state flag into the current state.
	{
		mCurrentState.mStates|=state;
		mCurrentState.incrementChangeCount();
	}

	virtual void  removeFromCurrentState(DebugRenderState::Enum state)  // Remove this bit flat from the current state
	{
		mCurrentState.mStates&=~state;
		mCurrentState.incrementChangeCount();
	}

	virtual void  setCurrentTextScale(PxF32 textScale)
	{
		mCurrentState.mTextScale = textScale;
		mCurrentState.incrementChangeCount();
	}

	virtual void  setCurrentArrowSize(PxF32 arrowSize)
	{
		mCurrentState.mArrowSize = arrowSize;
		mCurrentState.incrementChangeCount();
	}

	virtual PxU32 getCurrentState(void) const
	{
		return mCurrentState.mStates;
	}

	virtual void  setRenderState(PxU32 states=0,  // combination of render state flags
	                             PxU32 color=0xFFFFFF, // base color
                                 PxF32 displayTime=0.0001f, // duration of display items.
	                             PxU32 arrowColor=0xFF0000, // secondary color, usually used for arrow head
                                 PxF32 arrowSize=0.1f,
								 PxF32 renderScale=1.0f,
								 PxF32 textScale=1.0f)      // seconary size, usually used for arrow head size.
	{
		physx::PxU32 saveChangeCount = mCurrentState.mChangeCount;
		new ( &mCurrentState ) RenderState(states,color,displayTime,arrowColor,arrowSize,renderScale,textScale);
		mCurrentState.mChangeCount = saveChangeCount;
		mCurrentState.incrementChangeCount();
	}


	virtual PxU32 getRenderState(PxU32 &color,PxF32 &displayTime,PxU32 &arrowColor,PxF32 &arrowSize,PxF32 &renderScale,PxF32 & /*textScale*/) const
	{
		color = mCurrentState.mColor;
		displayTime = mCurrentState.mDisplayTime;
		arrowColor = mCurrentState.mArrowColor;
		arrowSize = mCurrentState.mArrowSize;
		renderScale = mCurrentState.mRenderScale;
		return mCurrentState.mStates;
	}


	virtual void  endDrawGroup(void)
	{
		popRenderState();
		PostRenderDebug::updatePostDrawGroupPose(mCurrentState);
	}

	virtual void  setDrawGroupVisible(PxI32 blockId,bool state)
	{
		const HashMap<PxU32, BlockInfo *>::Entry *e = mBlocksHash.find((PxU32)blockId);
		if ( e )
		{
			BlockInfo *b = e->second;
			if ( b->mVisibleState != state )
			{
				b->mVisibleState = state;
			}
		}
	}

	virtual void  debugPolygon(PxU32 pcount,const physx::PxVec3 *points)
	{
		if ( mCurrentState.isSolid() )
		{
			PX_ASSERT( pcount >= 3 );
			PX_ASSERT( pcount <= 256 );
			bool wasOverlay = mCurrentState.hasRenderState(DebugRenderState::SolidWireShaded);
			if( wasOverlay )
			{
				mCurrentState.clearRenderState(DebugRenderState::SolidWireShaded);
				mCurrentState.incrementChangeCount();
			}
			const PxVec3 *v1 = &points[0];
			const PxVec3 *v2 = &points[1];
			const PxVec3 *v3 = &points[2];
			debugTri(*v1,*v2,*v3);
			for (PxU32 i=3; i<pcount; i++)
			{
				v2 = v3;
				v3 = &points[i];
				debugTri(*v1,*v2,*v3);
			}
			if ( wasOverlay )
			{
        		for (PxU32 i=0; i<(pcount-1); i++)
        		{
        			debugLine( points[i], points[i+1] );
        		}
        		debugLine(points[pcount-1],points[0]);
				mCurrentState.setRenderState(DebugRenderState::SolidWireShaded);
				mCurrentState.incrementChangeCount();
			}
		}
		else
		{
    		for (PxU32 i=0; i<(pcount-1); i++)
    		{
    			debugLine( points[i], points[i+1] );
    		}
    		debugLine(points[pcount-1],points[0]);
    	}
	}

	virtual void  debugLine(const physx::PxVec3 &p1,const physx::PxVec3 &p2)
	{
		DebugLine d(p1,p2);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugOrientedLine(const physx::PxVec3 &p1,const physx::PxVec3 &p2,const PxMat44 &transform)
	{
		PxVec3 _p1 = transform.transform(p1);
		PxVec3 _p2 = transform.transform(p2);
		debugLine(_p1,_p2);
	}

	virtual void  debugBound(const physx::PxBounds3 &_b)
	{
		DebugBound b(_b.minimum,_b.maximum );
		PostRenderDebug::postRenderDebug(&b,mCurrentState);
	}

	virtual void  debugOrientedBound(const physx::PxVec3 &sides,const PxMat44 &transform)
	{
		PxVec3 bmin = -sides*0.5f;
		PxVec3 bmax = sides*0.5f;
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}
		debugBound(bmin,bmax);
		popRenderState();
	}

	virtual void  debugBound(const physx::PxVec3 &bmin,const physx::PxVec3 &bmax)
	{
		DebugBound b(bmin,bmax);
		PostRenderDebug::postRenderDebug(&b,mCurrentState);
	}

	virtual void  debugPoint(const physx::PxVec3 &pos,PxF32 radius)
	{
		DebugPoint d(pos,radius);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugQuad(const physx::PxVec3 &pos,const physx::PxVec2 &scale,PxF32 rotation)
	{
		DebugQuad d(pos,scale,rotation);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}


	virtual void  debugPoint(const physx::PxVec3 &pos,const PxVec3 &radius)
	{
		DebugPointScale d(pos,radius);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}


	virtual void  debugOrientedBound(const physx::PxVec3 &bmin,const physx::PxVec3 &bmax,const physx::PxVec3 &pos,const physx::PxQuat &quat) // the rotation as a quaternion
	{
		PxMat44 transform(quat);
		transform.setPosition(pos);
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}
		debugBound(bmin,bmax);
		popRenderState();
	}

	virtual void  debugOrientedBound(const physx::PxVec3 &bmin,const physx::PxVec3 &bmax,const PxMat44 &transform)  // the rotation as a quaternion
	{
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}
		debugBound(bmin,bmax);
		popRenderState();
	}


	virtual void debugGraph(PxU32 /*numPoints*/, PxF32 * /*points*/, PxF32 /*graphMax*/, PxF32 /*graphXPos*/, PxF32 /*graphYPos*/, PxF32 /*graphWidth*/, PxF32 /*graphHeight*/, PxU32 /*colorSwitchIndex*/ = 0xFFFFFFFF)
	{
		PX_ALWAYS_ASSERT();
	}


	virtual void debugRect2d(PxF32 x1,PxF32 y1,PxF32 x2,PxF32 y2)
	{
		DebugRect2d d(x1,y1,x2,y2);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugGradientLine(const physx::PxVec3 &p1,const physx::PxVec3 &p2,const PxU32 &c1,const PxU32 &c2)
	{
		DebugGradientLine d(p1,p2,c1,c2);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugRay(const physx::PxVec3 &p1,const physx::PxVec3 &p2)
	{
		DebugRay d(p1,p2);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugCylinder(const physx::PxVec3 &p1,const physx::PxVec3 &p2,PxF32 radius)
	{
		DebugPointCylinder d(p1,p2,radius);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugThickRay(const physx::PxVec3 &p1,const physx::PxVec3 &p2,PxF32 raySize=0.02f)
	{
		DebugThickRay d(p1,p2,raySize);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugPlane(const physx::PxVec3 &normal,PxF32 dCoff,PxF32 radius1,PxF32 radius2)
	{
		DebugPlane d(normal,dCoff,radius1,radius2);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugTri(const physx::PxVec3 &p1,const physx::PxVec3 &p2,const physx::PxVec3 &p3)
	{
		DebugTri d(p1,p2,p3);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugTriNormals(const physx::PxVec3 &p1,const physx::PxVec3 &p2,const physx::PxVec3 &p3,const physx::PxVec3 &n1,const physx::PxVec3 &n2,const physx::PxVec3 &n3)
	{
		DebugTriNormals d(p1,p2,p3,n1,n2,n3);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugGradientTri(const physx::PxVec3 &p1,const physx::PxVec3 &p2,const physx::PxVec3 &p3,const PxU32 &c1,const PxU32 &c2,const PxU32 &c3)
	{
		DebugGradientTri d(p1,p2,p3,c1,c2,c3);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugGradientTriNormals(const physx::PxVec3 &p1,const physx::PxVec3 &p2,const physx::PxVec3 &p3,const physx::PxVec3 &n1,const physx::PxVec3 &n2,const physx::PxVec3 &n3,const PxU32 &c1,const PxU32 &c2,const PxU32 &c3)
	{
		DebugGradientTriNormals d(p1,p2,p3,n1,n2,n3,c1,c2,c3);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	virtual void  debugSphere(const physx::PxVec3 &pos, PxF32 radius)
	{
		PxMat44 pose = PxMat44::createIdentity();
		pose.setPosition(pos);

		debugOrientedSphere(radius, 2, pose);
	}

	virtual void  debugOrientedSphere(PxF32 radius, PxU32 subdivision,const PxMat44 &transform)
	{
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}

		DebugSphere d(radius, subdivision);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);

		popRenderState();
	}

	// a squashed sphere
	virtual void  debugOrientedSphere(const PxVec3 &radius, PxU32 subdivision,const PxMat44 &transform)
	{
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}

		DebugSquashedSphere d(radius, subdivision);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);

		popRenderState();
	}


	virtual void  debugOrientedCapsule(PxF32 radius,PxF32 height,PxU32 subdivision,const PxMat44 &transform)
	{
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}
		DebugCapsule d(radius,height,subdivision);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
		popRenderState();
	}

	virtual void  debugOrientedCapsuleTapered(PxF32 radius1, PxF32 radius2, PxF32 height, PxU32 subdivision, const PxMat44& transform)
	{
		pushRenderState();

		if (mCurrentState.mPose)
		{
			PxMat44 xform = *mCurrentState.mPose * transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}

		DebugTaperedCapsule d(radius1, radius2, height, subdivision);
		PostRenderDebug::postRenderDebug(&d, mCurrentState);

		popRenderState();
	}

	virtual void  debugOrientedCylinder(PxF32 radius,PxF32 height,PxU32 subdivision,bool closeSides,const PxMat44 &transform)
	{
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}
		DebugCylinder d(radius,height,subdivision,closeSides);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
		popRenderState();
	}

	virtual void  debugOrientedCircle(PxF32 radius,PxU32 subdivision,const PxMat44 &transform)
	{
		pushRenderState();
		if ( mCurrentState.mPose )
		{
			PxMat44 xform = *mCurrentState.mPose*transform;
			setPose(xform);
		}
		else
		{
			setPose(transform);
		}
		DebugCircle d(radius,subdivision);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
		popRenderState();
	}

	virtual void  debugAxes(const PxMat44 &transform,PxF32 distance=0.1f,PxF32 brightness=1.0f)
	{
		DebugAxes d(transform,distance,brightness);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}


    virtual void debugArc(const physx::PxVec3 &center,const physx::PxVec3 &p1,const physx::PxVec3 &p2,PxF32 arrowSize=0.1f,bool showRoot=false)
    {
		DebugArc d(center,p1,p2,arrowSize,showRoot);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
    }

    virtual void debugThickArc(const physx::PxVec3 &center,const physx::PxVec3 &p1,const physx::PxVec3 &p2,PxF32 thickness=0.02f,bool showRoot=false)
    {
		DebugThickArc d(center,p1,p2,thickness,showRoot);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
    }

	virtual void  debugDetailedSphere(const physx::PxVec3 &pos,PxF32 radius,PxU32 stepCount)
	{
		DebugDetailedSphere d(pos,radius,stepCount);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}


    virtual void debugText(const physx::PxVec3 &pos,const char *fmt,...)
    {
		char wbuff[256];
		wbuff[255] = 0;
		va_list arg;
		va_start( arg, fmt );
		physx::string::_vsnprintf(wbuff,sizeof(wbuff)-1, fmt, arg);
		va_end(arg);
		DebugText dt(pos,PxMat44::createIdentity(),wbuff);
		PostRenderDebug::postRenderDebug(&dt,mCurrentState);
    }

    virtual void debugOrientedText(const physx::PxVec3 &pos,const physx::PxQuat &rot,const char *fmt,...)
    {
		char wbuff[256];
		wbuff[255] = 0;
		va_list arg;
		va_start( arg, fmt );
		physx::string::_vsnprintf(wbuff,sizeof(wbuff)-1, fmt, arg);
		va_end(arg);
		PxMat44 pose(rot);
		pose.setPosition(pos);
		PxVec3 origin(0,0,0);
		DebugText dt(origin,pose,wbuff);
		PostRenderDebug::postRenderDebug(&dt,mCurrentState);
    }

    virtual void debugOrientedText(const PxMat44 &pose,const char *fmt,...)
    {
		char wbuff[256];
		wbuff[255] = 0;
		va_list arg;
		va_start( arg, fmt );
		physx::string::_vsnprintf(wbuff,sizeof(wbuff)-1, fmt, arg);
		va_end(arg);
		PxVec3 origin(0,0,0);
		DebugText dt(origin,pose,wbuff);
		PostRenderDebug::postRenderDebug(&dt,mCurrentState);
		//PX_ALWAYS_ASSERT();
    }

	virtual void setViewMatrix(const PxF32 *view)
	{
		memcpy( mViewMatrix44, view, 16*sizeof( PxF32 ) );
		updateVP();
		if ( mProcessRenderDebug )
		{
			const PxMat44 *view44 = (const PxMat44 *)mViewMatrix44;
			mProcessRenderDebug->setViewMatrix(*view44);
		}
	}

	virtual void setProjectionMatrix(const PxF32 *projection)
	{
		memcpy( mProjectionMatrix44, projection, 16*sizeof( PxF32 ) );
		updateVP();
	}

	void updateVP(void)
	{
		PxF32* e = mViewProjectionMatrix44;
		for( int c = 0; c < 4; ++c )
		{
			for( int r = 0; r < 4; ++r, ++e )
			{
				PxF32 sum = 0;
				for( int k = 0; k < 4; ++k )
				{
					sum += mProjectionMatrix44[r+(k<<2)]*mViewMatrix44[k+(c<<2)];
				}
				*e = sum;
			}
		}
		// grab the world-space eye position.
 		const PxMat44 *view44 = (const PxMat44 *)mViewMatrix44;
		PxMat44 viewInverse = view44->inverseRT();
		mEyePos = viewInverse.transform( PxVec3( 0.0f, 0.0f, 0.0f ));
	}

	virtual const PxF32* getViewProjectionMatrix(void) const
	{
		return mViewProjectionMatrix44;
	}

	virtual const PxF32 *getViewMatrix(void) const
	{
		return mViewMatrix44;			// returns column major array
	}

	virtual const PxF32 *getProjectionMatrix(void) const
	{
		return mProjectionMatrix44;		// returns column major array
	}


	virtual void  eulerToQuat(const physx::PxVec3 &angles, physx::PxQuat &q)  // angles are in degrees.
	{
		PxF32 roll  = angles.x*0.5f*FM_DEG_TO_RAD;
		PxF32 pitch = angles.y*0.5f*FM_DEG_TO_RAD;
		PxF32 yaw   = angles.z*0.5f*FM_DEG_TO_RAD;

		PxF32 cr = cosf(roll);
		PxF32 cp = cosf(pitch);
		PxF32 cy = cosf(yaw);

		PxF32 sr = sinf(roll);
		PxF32 sp = sinf(pitch);
		PxF32 sy = sinf(yaw);

		PxF32 cpcy = cp * cy;
		PxF32 spsy = sp * sy;
		PxF32 spcy = sp * cy;
		PxF32 cpsy = cp * sy;

		PxF32 x   = ( sr * cpcy - cr * spsy);
		PxF32 y   = ( cr * spcy + sr * cpsy);
		PxF32 z   = ( cr * cpsy - sr * spcy);
		PxF32 w   = cr * cpcy + sr * spsy;
		q = PxQuat(x,y,z,w);
	}

	virtual PxI32 beginDrawGroup(const PxMat44 &pose)
	{
		pushRenderState();

		setRenderState(DebugRenderState::InfiniteLifeSpan,0xFFFFFF,0.0001f,0xFF0000,0.1f,mCurrentState.mRenderScale,mCurrentState.mTextScale);

		mBlockIndex++;
		mCurrentState.mBlockInfo = mBlocks.construct();
		mCurrentState.mBlockInfo->mHashValue = mBlockIndex;
		mCurrentState.mBlockInfo->mPose = pose;
		mBlocksHash[mBlockIndex] =  mCurrentState.mBlockInfo;

		return (physx::PxI32)mBlockIndex;
	}

	virtual void  setDrawGroupPose(PxI32 blockId,const PxMat44 &pose)
	{
		const HashMap<PxU32, BlockInfo *>::Entry *e = mBlocksHash.find((PxU32)blockId);
		if ( e )
		{
			BlockInfo *b = e->second;
			if ( memcmp(&pose,&b->mPose,sizeof(PxMat44)) != 0 ) // if the pose has changed...
			{
				b->mPose = pose;
				b->mChanged = true;
			}
		}
	}

	virtual void  setPose(const PxMat44 &pose)
	{
		PxMat44 id = PxMat44::createIdentity();
		if ( id.column0 != pose.column0 ||
			 id.column1 != pose.column1 ||
			 id.column2 != pose.column2 ||
			 id.column3 != pose.column3 )
		{
			mCurrentState.mCurrentPose = pose;
			mCurrentState.mPose 	   = &mCurrentState.mCurrentPose;
		}
		else
		{
			mCurrentState.mPose = NULL;
			mCurrentState.mCurrentPose = PxMat44::createIdentity();
		}
		mCurrentState.incrementChangeCount();
	}

	virtual const PxMat44 & getPose(void) const
	{
		return mCurrentState.mCurrentPose;
	}

	/* \brief Create an createDebugGraphDesc.  This is the manual way of setting up a graph.  Every parameter can
	and must be customized when using this constructor.
	*/
	virtual DebugGraphDesc* createDebugGraphDesc(void)
	{
		DebugGraphDesc* dGDPtr = (DebugGraphDesc *)PX_ALLOC(sizeof(DebugGraphDesc), PX_DEBUG_EXP("DebugGraphDesc"));
		new  (dGDPtr ) DebugGraphDesc;

		dGDPtr->mNumPoints = 0;
		dGDPtr->mPoints = NULL;
		dGDPtr->mGraphXLabel = NULL;
		dGDPtr->mGraphYLabel = NULL;
		dGDPtr->mGraphColor = 0x00FF0000;
		dGDPtr->mArrowColor = 0x0000FF00;
		dGDPtr->mColorSwitchIndex = (PxU32) -1;
		dGDPtr->mGraphMax		= 0.0f;
		dGDPtr->mGraphXPos		= 0.0f;
		dGDPtr->mGraphYPos		= 0.0f;
		dGDPtr->mGraphWidth		= 0.0f;
		dGDPtr->mGraphHeight	= 0.0f;
		dGDPtr->mCutOffLevel	= 0.0f;
		return(dGDPtr);
	}

	virtual void releaseDebugGraphDesc(DebugGraphDesc *desc) 
	{
		PX_FREE(desc);
	}

	/**
	\brief Create an createDebugGraphDesc using the minimal amount of work.  This constructor provides for six custom
	graphs to be simultaneously drawn on the display at one time numbered 0 to 5.  The position, color, and size
	of the graphs are automatically set based on the graphNum argument.
	*/
	#define LEFT_X			(-0.9f)
	#define RIGHT_X			(+0.1f)
	#define TOP_Y			(+0.4f)
	#define MID_Y			(-0.2f)
	#define BOTTOM_Y		(-0.8f)
	virtual DebugGraphDesc* createDebugGraphDesc(PxU32 graphNum,PxU32 dataCount,const PxF32 *dataArray, PxF32 maxY, char* xLabel, char* yLabel)
	{
		static struct
		{
			PxF32 xPos, yPos;
		} graphData[MAX_GRAPHS] =
		{
			{LEFT_X,  TOP_Y},
			{LEFT_X,  MID_Y},
			{LEFT_X,  BOTTOM_Y},
			{RIGHT_X, TOP_Y},
			{RIGHT_X, MID_Y},
			{RIGHT_X, BOTTOM_Y}
		};
		PX_ASSERT(graphNum < MAX_GRAPHS);
		DebugGraphDesc* dGDPtr	= createDebugGraphDesc();

		dGDPtr->mGraphColor		= 0x00FF0000;
		dGDPtr->mArrowColor		= 0x0000FF00;
		dGDPtr->mColorSwitchIndex = (PxU32) -1;

		// no cut off line by default
		dGDPtr->mCutOffLevel	= 0.0f;
		dGDPtr->mNumPoints		= dataCount;
		dGDPtr->mPoints			= dataArray;
		dGDPtr->mGraphMax		= maxY;
		dGDPtr->mGraphXLabel	= xLabel;
		dGDPtr->mGraphYLabel	= yLabel;

		dGDPtr->mGraphXPos		= graphData[graphNum].xPos;
		dGDPtr->mGraphYPos		= graphData[graphNum].yPos;;
		dGDPtr->mGraphWidth		= GRAPH_WIDTH_DEFAULT;
		dGDPtr->mGraphHeight	= GRAPH_HEIGHT_DEFAULT;

		return(dGDPtr);
	}

	virtual void debugGraph(DebugGraphDesc& graphDesc)
	{
		DebugGraphStream d(graphDesc);
		PostRenderDebug::postRenderDebug(&d,mCurrentState);
	}

	/**
	\brief Set a debug color value by name.
	*/
	virtual void setDebugColor(DebugColors::Enum colorEnum, PxU32 value)
	{
		if( colorEnum < DebugColors::NUM_COLORS )
		{
			colors[ colorEnum ] = value;
		}
	}

	/**
	\brief Return a debug color value by name.
	*/
	virtual PxU32 getDebugColor(DebugColors::Enum colorEnum) const
	{
		if( colorEnum < DebugColors::NUM_COLORS )
		{
			return colors[ colorEnum ];
		}
		return 0;
	}

	/**
	\brief Return a debug color value by RGB inputs
	*/
	virtual PxU32 getDebugColor(float red, float green, float blue) const
	{
		union
		{
			PxU8 colorChars[4];
			PxU32 color;
		};

		colorChars[3] = 0xff; // alpha
		colorChars[2] = (PxU8)(red * 255);
		colorChars[1] = (PxU8)(green * 255);
		colorChars[0] = (PxU8)(blue * 255);

		return color;
	}

	virtual void getDebugBounds(physx::PxBounds3 & /*b*/) const
	{
		PX_ALWAYS_ASSERT();
	}

	virtual void renderIndexedTriangleMesh(PxU32 /*triangleMeshId*/,const char * /*material*/,
		PxU32 /*transformCount*/,
		const PxF32 * /*localToWorld44*/,
		const PxF32 * /*positions*/,
		const PxF32 * /*normals*/,
		const PxU32 * /*colors*/,
		const PxF32 * /*tangents*/,
		const PxF32 * /*binormals*/,
		const PxF32 * /*texcoords0*/,
		const PxF32 * /*texcoords1*/,
		const PxF32 * /*texcoords2*/,
		const PxF32 * /*texcoords3*/,
		const PxU32  * /*indices*/,
		PxU32 /*triCount*/,
		PxU32 /*vertCount*/)
	{
		PX_ALWAYS_ASSERT();
	}

	virtual	PxU32 getTriangleMeshId(void)
	{
		PxU32 ret = 0;
		PX_ALWAYS_ASSERT();
		return ret;
	}

private:

	void initColors()
	{
#define INIT_COLOR( name, defVal, minVal, maxVal )	\
	colorsDefValue[ name ] = PxU32(defVal);		\
	colorsMinValue[ name ] = PxU32(minVal);		\
	colorsMaxValue[ name ] = PxU32(maxVal);

		INIT_COLOR(DebugColors::Default,		0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::PoseArrows,		0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::MeshStatic,		0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::MeshDynamic,	0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Shape,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Text0,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Text1,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::ForceArrowsLow,	0xffffff00,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::ForceArrowsNorm,0xff00ff00,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::ForceArrowsHigh,0xffff0000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Color0,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Color1,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Color2,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Color3,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Color4,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Color5,			0x00000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Red,			0xffff0000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Green,			0xff00ff00,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Blue,			0xff0000ff,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::DarkRed,		0xff800000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::DarkGreen,		0xff008000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::DarkBlue,		0xff000080,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::LightRed,		0xffff8080,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::LightGreen,		0xff80ff00,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::LightBlue,		0xff00ffff,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Purple,			0xffff00ff,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::DarkPurple,		0xff800080,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Yellow,			0xffffff00,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Orange,			0xffff8000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Gold,			0xff808000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Emerald,		0xff008080,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::White,			0xffffffff,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Black,			0xff000000,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::Gray,			0xff808080,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::LightGray,		0xffC0C0C0,		PX_MIN_U32,		PX_MAX_U32);
		INIT_COLOR(DebugColors::DarkGray,		0xff404040,		PX_MIN_U32,		PX_MAX_U32);
#undef INIT_COLOR

		memcpy( colors, colorsDefValue, sizeof(PxU32) * DebugColors::NUM_COLORS );
	}

	void releaseRenderDebug(void)
	{
		delete this;
	}

	virtual void setFrameTime(PxF32 ftime)
	{
		mFrameTime = ftime;
	}

	virtual PxF32 getFrameTime(void) const
	{
		return mFrameTime;
	}

	PxU32					 		  mUpdateCount;
	PxProcessRenderDebug			 *mProcessRenderDebug;
	RenderState                       mCurrentState;
	PxU32                             mStackIndex;
	RenderState                       mRenderStateStack[RENDER_STATE_STACK_SIZE];
	mutable PxF32                     mViewMatrix44[16];
	mutable PxF32                     mProjectionMatrix44[16];
	mutable PxF32                     mViewProjectionMatrix44[16];
	PxVec3                            mEyePos;
	//Colors for debug rendering purposes
	PxU32							colors[DebugColors::NUM_COLORS];
	PxU32							colorsMinValue[DebugColors::NUM_COLORS];
	PxU32							colorsMaxValue[DebugColors::NUM_COLORS];
	PxU32							colorsDefValue[DebugColors::NUM_COLORS];

	PxU32							mBlockIndex;
	HashMap<PxU32, BlockInfo *>		mBlocksHash;
	Pool< BlockInfo >				mBlocks;
	PxF32							mFrameTime;
	bool							mOwnProcess;

};

DebugGraphStream::DebugGraphStream(const DebugGraphDesc &d) : DebugPrimitive(DebugCommand::DEBUG_GRAPH)
{
	physx::PxMemoryBuffer mb;
	mb.setEndianMode(PxFileBuf::ENDIAN_NONE);
	physx::PxIOStream stream(mb,0);

	mSize = 0;
	stream << mCommand;
	stream << mSize;
	stream << d.mNumPoints;
	stream.storeString(d.mGraphXLabel ? d.mGraphXLabel : "",true);
	stream.storeString(d.mGraphYLabel ? d.mGraphYLabel : "",true);

	mb.alignWrite(4);

	for (physx::PxU32 i=0; i<d.mNumPoints; i++)
	{
		stream << d.mPoints[i];
	}
	stream << d.mCutOffLevel;
	stream << d.mGraphMax;
	stream << d.mGraphXPos;
	stream << d.mGraphYPos;
	stream << d.mGraphWidth;
	stream << d.mGraphHeight;
	stream << d.mGraphColor;
	stream << d.mArrowColor;
	stream << d.mColorSwitchIndex;

	mBuffer = mb.getWriteBufferOwnership(mSize);
	mAllocated = true;
}

DebugGraphStream::~DebugGraphStream(void)
{
	if ( mAllocated )
	{
		PX_FREE(mBuffer);
	}
}


//

RenderDebug * createInternalRenderDebug(PxProcessRenderDebug *process)
{
	InternalRenderDebug *m = PX_NEW(InternalRenderDebug)(process);
	return static_cast< RenderDebug *>(m);
}



};
};
#endif
