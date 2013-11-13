
#include "stdafx.h"
#include <process.h>
#include "DirectoryWatcher.h"

namespace watch {

	////////////////////////////////////////////////////////////////////////////////
	/// Request

	///
	Request::Request(Worker *worker, const std::wstring &dirName,
		bool watchSubtree, DWORD filter, DWORD size)
		: m_hDirectory(nullptr)
		, m_worker(worker)
		, m_dirName(dirName)
		, m_watchSubtree(watchSubtree)
		, m_flags(filter)
	{
		::ZeroMemory(&m_overlapped, sizeof(OVERLAPPED));

		// The hEvent member is not used when there is a completion
		// function, so it's ok to use it to point to the object.
		m_overlapped.hEvent = this;

		m_buffer.resize(size);
		m_backupBuffer.resize(size);
	}

	///
	Request::~Request()
	{
		// RequestTermination() must have been called successfully.
		_ASSERTE(m_hDirectory == nullptr);
	}

	///
	bool Request::OpenDirectory()
	{
		// Allow this routine to be called redundantly.
		if (m_hDirectory)
			return true;

		m_hDirectory = ::CreateFile(m_dirName.c_str(),					// pointer to the file name
			FILE_LIST_DIRECTORY,										// access (read/write) mode
			FILE_SHARE_READ	| FILE_SHARE_WRITE 	| FILE_SHARE_DELETE,	// share mode
			nullptr,													// security descriptor
			OPEN_EXISTING,												// how to create
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,			// file attributes
			nullptr);													// file with attributes to copy

		if (m_hDirectory == INVALID_HANDLE_VALUE)
			return false;

		return true;
	}

	///
	void Request::BackupBuffer(DWORD size)
	{
		memcpy(&m_backupBuffer[0], &m_buffer[0], size);
	}

	///
	void Request::BeginRead()
	{
		DWORD cBytes = 0;

		// This call needs to be reissued after every APC.
		BOOL success = ::ReadDirectoryChangesW(
			m_hDirectory,						// handle to directory
			&m_buffer[0],						// read results buffer
			(DWORD) m_buffer.size(),			// length of buffer
			m_watchSubtree,						// monitoring option
			FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION | FILE_NOTIFY_CHANGE_FILE_NAME,        // filter conditions
			&cBytes,							// bytes returned
			&m_overlapped,						// overlapped buffer
			&NotificationCompletion);			// completion routine
	}

	///
	void Request::NotificationCompletion(DWORD errorCode,
		DWORD cBytesTransfered,
		LPOVERLAPPED overlapped)
	{
		Request *request = (Request *) overlapped->hEvent;

		if (ERROR_OPERATION_ABORTED == errorCode)
		{
			::InterlockedDecrement(&request->GetWorker()->m_nRequests);
			delete request;
			return;
		}

		if (!cBytesTransfered)
			return;

		request->BackupBuffer(cBytesTransfered);

		// Get the new read issued as fast as possible. The documentation
		// says that the original OVERLAPPED structure will not be used
		// again once the completion routine is called.
		request->BeginRead();
		//
		request->ProcessNotification();
	}

	///
	void Request::ProcessNotification()
	{
		char *base = (char *) &m_backupBuffer[0];

		for (;;)
		{
			FILE_NOTIFY_INFORMATION &fni = (FILE_NOTIFY_INFORMATION &) *base;

			size_t len = fni.FileNameLength / sizeof(wchar_t);
			wchar_t *buff = fni.FileName;
			*(buff + len) = 0;

			std::wstring fileName = buff;

			m_worker->GetParent()->Push(fni.Action, fileName);

			if (!fni.NextEntryOffset)
				break;

			base += fni.NextEntryOffset;
		}
	}

	///
	void Request::RequestTermination()
	{
		::CancelIo(m_hDirectory);
		::CloseHandle(m_hDirectory);
		m_hDirectory = nullptr;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// DirectoryChanges

	///
	DirectoryChanges::DirectoryChanges(int maxChanges)
		: m_hThread(nullptr)
		, m_threadId(0)
		, m_worker(nullptr)
		, m_notifications(maxChanges)
	{
		m_worker = new Worker(this);
	}

	///
	DirectoryChanges::~DirectoryChanges()
	{
		Terminate();
		delete m_worker;
	}

	///
	void DirectoryChanges::Init()
	{
		// kick off the working thread
		m_hThread = (HANDLE) _beginthreadex(nullptr, 0, 
			Worker::ThreadStartProc,
			m_worker,
			0,
			&m_threadId);
	}

	///
	void DirectoryChanges::Terminate()
	{
		if (m_hThread)
		{
			::QueueUserAPC(Worker::TerminateProc, m_hThread, (ULONG_PTR) m_worker);
			::WaitForSingleObjectEx(m_hThread, 1000, TRUE);

			::CloseHandle(m_hThread);
			m_hThread = nullptr;
			m_threadId = 0;
		}
	}

	///
	void DirectoryChanges::AddDirectory(const std::wstring & dirName, bool watchSubtree, 
		DWORD notifFilter, DWORD buffSize)
	{
		if (!m_hThread)
			Init();

		Request *request = new Request(m_worker,
			dirName, watchSubtree, notifFilter, buffSize);

		::QueueUserAPC(Worker::AddDirectoryProc, m_hThread, (ULONG_PTR) request);
	}

	///
	bool DirectoryChanges::Pop(DWORD & action, std::wstring & fileName)
	{
		DirectoryChangeNotification pair;
		if (!m_notifications.Pop(pair))
			return false;

		action = pair.first;
		fileName = pair.second;

		return true;
	}

	///
	void DirectoryChanges::Push(DWORD action, const std::wstring & fileName)
	{
		m_notifications.Push(DirectoryChangeNotification(action, fileName));
	}

}	//	namespace watch