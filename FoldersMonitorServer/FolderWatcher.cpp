#include "stdafx.h"
#include "Trace.h"
#include "FolderWatcher.h"

///
struct ScopeHandle
{
	ScopeHandle(HANDLE handle = nullptr)
		: m_handle(handle)
	{
	}

	~ScopeHandle()
	{
		if (m_handle && (m_handle != INVALID_HANDLE_VALUE))
		{
			::CloseHandle(m_handle);
			m_handle = nullptr;
		}
	}

	operator HANDLE()
	{
		return m_handle;
	}

	HANDLE m_handle;
};

///
void FolderWatcher::Refresh()
{
	ScopedLock<CsLock> lock(m_notifLock);

	HANDLE hDir = ::CreateFile(m_folderName.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_DELETE | FILE_SHARE_WRITE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		nullptr);

	FILE_NOTIFY_INFORMATION buffer[1024] = {0};
	DWORD cBytesRead = 0;

	DWORD flags = 0;
	if (m_notifyFilter & 2)
		flags |= FILE_NOTIFY_CHANGE_FILE_NAME;

	if (m_notifyFilter & 4)
		flags |= FILE_NOTIFY_CHANGE_ATTRIBUTES;

	if (m_notifyFilter & 8)
		flags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

	// we are going to use the synch way

	BOOL bRes = ::ReadDirectoryChangesW(hDir, 
		&buffer, sizeof(buffer),
		FALSE,
		flags,
		&cBytesRead,
		nullptr,
		nullptr);

	while (bRes)
	{
		switch (buffer[0].Action)
		{
		case FILE_ACTION_ADDED:
		case FILE_ACTION_MODIFIED:
		case FILE_ACTION_REMOVED:
		case FILE_ACTION_RENAMED_NEW_NAME:
			{
				int i= 0;
				do {
					m_listener->OnNameChanged(buffer[i].FileName);
					++i;
				} while (buffer[i].NextEntryOffset);
			}
			break;
		}
	}

}

///
int FolderWatcher::SetFolder(const std::wstring & folderName)
{
	if (folderName.compare(m_folderName) == 0)
		return 0;

	//
	StopWatch();

	//
	m_folderName = folderName;
	::ResetEvent(m_hStopEvent);
	//
	com::Trace(L"[%s] : folder name has changed into %s\n", __FUNCTIONW__, m_folderName.c_str());

	return 0;
}

///
int FolderWatcher::StartWatch()
{
	HANDLE changeHandles[2];
	int retval = ERROR_SUCCESS;

	DWORD flags = 0;
	if (m_notifyFilter & 2)
		flags |= FILE_NOTIFY_CHANGE_FILE_NAME;

	if (m_notifyFilter & 4)
		flags |= FILE_NOTIFY_CHANGE_ATTRIBUTES;

	if (m_notifyFilter & 8)
		flags |= FILE_NOTIFY_CHANGE_LAST_WRITE;

	//
	ScopeHandle sh = ::FindFirstChangeNotification(m_folderName.c_str(),
		FALSE,
		flags);

	changeHandles[0] = sh;

	if (!changeHandles[0] || INVALID_HANDLE_VALUE == changeHandles[0])
	{
		int error = ::GetLastError();
		com::Trace(L"[%s] : %s (error %d)\n", __FUNCTIONW__, L"FindFirstChangeNotification failed", error); 
		return error;
	}

	changeHandles[1] = m_hStopEvent;
	if (!changeHandles[1] || INVALID_HANDLE_VALUE == changeHandles[1])
		return ERROR_INVALID_HANDLE;

	//
	for  (;;)
	{
		DWORD status= ::WaitForMultipleObjects(2, changeHandles, FALSE, INFINITE);

		switch (status)
		{
		case WAIT_OBJECT_0:
			{
				// a change event ocurred
				com::Trace(L"[%s] : %s\n", __FUNCTIONW__, L"A change event was fired.");
				//
				Refresh();

				if (FALSE == ::FindNextChangeNotification(changeHandles[0]))
				{
					int error = ::GetLastError();
					com::Trace(L"[%s] : %s: %d\n", L"FindNextChangeNotification failed", error); 
					return error;
				}
			}
			break;

		case WAIT_OBJECT_0 + 1:
			// the "watcher" was stopped
			com::Trace(L"[%s] : %s\n", __FUNCTIONW__, L"The watcher was stopped.");
			return ERROR_SUCCESS;

		case WAIT_TIMEOUT:
			com::Trace(L"[%s] : %s\n", __FUNCTIONW__, L"Timeout");
			return ERROR_TIMEOUT;
			break;

		default:
			com::Trace(L"[%s] : %s (%d).\n", __FUNCTIONW__, L"Unhandled wait status", status);
			return ::GetLastError();
		}
	}

	return ERROR_SUCCESS;
}

///
void FolderWatcher::StopWatch()
{
	_ASSERT(m_hStopEvent);

	if (m_hStopEvent)
		::SetEvent(m_hStopEvent);
}