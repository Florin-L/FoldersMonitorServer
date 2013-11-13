
#pragma once

#include "lock.h"

////////////////////////////////////////////////////////////////////////////////
///
class FolderWatcher
{
public:
	//
	FolderWatcher(const std::wstring &folderName = L"", DWORD notifyFilter = 0);
	virtual ~FolderWatcher();

	int SetFolder(const std::wstring &);
	const std::wstring & GetFolder() const;

	void SetNotifyFilter(DWORD notifyFilter);

	void Refresh();
	int StartWatch();
	void StopWatch();
	void Continue();

	//
	struct INotifyChanges
	{
		virtual void OnNameChanged(const std::wstring &) = 0;
		virtual void OnAttributesChanged(const std::wstring &) = 0;
		virtual void OnLastWriteChanged(const std::wstring &) = 0;
	};

	void SetEventsListener(INotifyChanges *listener)
	{
		if (m_listener)
			return;

		m_listener = listener;
	}

	INotifyChanges * UnSetEventsListener()
	{
		INotifyChanges *tmp = m_listener;
		m_listener = nullptr;

		return tmp;
	}

private:
	FolderWatcher(const FolderWatcher &);
	FolderWatcher & operator =(const FolderWatcher &);

private:
	std::wstring	m_folderName;
	DWORD			m_notifyFilter;
	HANDLE			m_hStopEvent;

	INotifyChanges	*m_listener;
	CsLock			m_notifLock;
};

///
inline FolderWatcher::FolderWatcher(const std::wstring & folderName,
									DWORD notifyFilter)
	: m_folderName(folderName)
	, m_notifyFilter(notifyFilter)
	, m_listener(nullptr)
{
	m_hStopEvent = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
}

///
inline FolderWatcher::~FolderWatcher()
{
	if (m_hStopEvent)
	{
		::CloseHandle(m_hStopEvent);
		m_hStopEvent = nullptr;
	}

	UnSetEventsListener();
}

///
inline const std::wstring & FolderWatcher::GetFolder() const
{
	return m_folderName;
}

///
inline void FolderWatcher::SetNotifyFilter(DWORD notifyFilter)
{
	m_notifyFilter = notifyFilter;
}