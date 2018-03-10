#pragma once

#include "engine/core/Memory/MemManager.h"

namespace Echo
{
	extern void FlushRenderTasks();

	class RenderTask
	{
	public:
		static ui32 SMALL_TASK_SIZE;

		RenderTask();
		virtual ~RenderTask();
		virtual void Execute() = 0;

		static void  InitTaskAllocator();
		static void  DestroyTaskAllocator();
		static void* AllocTaskMem();
		static void  FreeTaskMem(void* mem);
		static void  ResetTaskAllocator();

	private:
		RenderTask(const RenderTask&);
		RenderTask& operator = (const RenderTask&);
	};

	template <typename TTask>
	class TRenderTask : public RenderTask
	{
	public:
		virtual void Execute()
		{
			TTask& task = *(TTask*)m_taskStorage;
			task.Execute();
			task.~TTask();

			if (sizeof(TRenderTask) <= RenderTask::SMALL_TASK_SIZE)
			{
				RenderTask::FreeTaskMem(this);
			}
			else
			{
				delete this;
			}
		}

		static void CreateTask()
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask;
			g_render_thread->pushTask(task);
		}

		template <typename T1>
		static void CreateTask(const T1& arg1)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2>
		static void CreateTask(const T1& arg1, const T2& arg2)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3, typename T4>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3, arg4);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3, arg4, arg5);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3, arg4, arg5, arg6);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
			g_render_thread->pushTask(task);
		}

		template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
		static void CreateTask(const T1& arg1, const T2& arg2, const T3& arg3, const T4& arg4, const T5& arg5, const T6& arg6, const T7& arg7, const T8& arg8, const T9& arg9)
		{
			TRenderTask* task = allocTTask();
			new (task->m_taskStorage) TTask(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
			g_render_thread->pushTask(task);
		}

	private:
		static TRenderTask* allocTTask()
		{
			if (sizeof(TRenderTask) <= RenderTask::SMALL_TASK_SIZE)
			{
				void* mem = RenderTask::AllocTaskMem();
				return new (mem)TRenderTask;
			}
			else
			{
				return new TRenderTask;
			}
		}

		Byte m_taskStorage[sizeof(TTask)];
	};
}