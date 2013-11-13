#pragma once

#include <list>
#include "lock.h"

namespace watch {

	////////////////////////////////////////////////////////////////////////////////
	/// ThreadSafeQueue
	template <typename T>
	class ThreadSafeQueue
		: protected std::list<T>
	{
	public:
		///
		ThreadSafeQueue(int maxCount)
			: m_hSemaphore(nullptr)
			, m_overflow(false)
		{
			m_hSemaphore = ::CreateSemaphore(nullptr,
				0,
				maxCount,
				nullptr);

			_ASSERT(m_hSemaphore);
		}

		///
		~ThreadSafeQueue()
		{
			::CloseHandle(m_hSemaphore);
			m_hSemaphore = nullptr;
		}

		///
		void Push(T & elem)
		{
			{
				ScopedLock<CsLock> lock(m_lock);
				push_back(elem);
			}

			if (!::ReleaseSemaphore(m_hSemaphore, 1, nullptr))
			{
				// If the semaphore is full, then take back the entry.
				pop_back();

				if (::GetLastError() == ERROR_TOO_MANY_POSTS)
					m_overflow = true;
			}
		}

		///
		bool Pop(T & elem)
		{
			ScopedLock<CsLock> lock(m_lock);

			// If the user calls pop() more than once after the
			// semaphore is signaled, then the semaphore count will
			// get out of sync.  We fix that when the queue empties.
			if (empty())
			{
				while (::WaitForSingleObject(m_hSemaphore, 0) != WAIT_TIMEOUT)
					1;

				return false;
			}

			elem = front();
			pop_front();

			return true;
		}

		/// If overflow, use this to clear the queue.
		void clear()
		{
			ScopedLock<CsLock> lock(m_lock);

			for (DWORD i = 0; i < size(); ++i)
				::WaitForSingleObject(m_hSemaphore, 0);

			__super::clear();
			m_overflow = false;
		}

		///
		HANDLE GetWaitHandle()
		{
			return m_hSemaphore;
		}

		///
		bool IsOverflow()
		{
			return m_overflow;
		}

	protected:
		///
		HANDLE	m_hSemaphore;
		bool	m_overflow;

		///
		CsLock	m_lock;
	};

}	//	namespace watch