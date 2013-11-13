
#pragma once

///////////////////////////////////////////////////////////
// Lock

class CsLock
{
public:
	CsLock()
	{
		::InitializeCriticalSection(&m_cs);
	}

	~CsLock()
	{
		::DeleteCriticalSection(&m_cs);
	}

	void Lock()
	{
		::EnterCriticalSection(&m_cs);
	}

	void Unlock()
	{
		::LeaveCriticalSection(&m_cs);
	}

protected:
	CRITICAL_SECTION m_cs;
};


///////////////////////////////////////////////////////////
// ScopedLock

template <typename LOCK>
class ScopedLock
{
public:
	ScopedLock(LOCK &lock)
		: m_pLock(&lock)
	{
		m_pLock->Lock();
	}

	~ScopedLock()
	{
		m_pLock->Unlock();
	}

protected:
	LOCK *m_pLock;
};