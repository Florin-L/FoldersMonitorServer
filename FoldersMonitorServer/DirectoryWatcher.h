#pragma once

#include <vector>
#include <memory>
#include <utility>
#include "ThreadSafeQueue.h"

namespace watch {

	//
	typedef std::pair<DWORD, std::wstring> DirectoryChangeNotification;

	// forward declarations
	class DirectoryChanges;
	class Worker;

	////////////////////////////////////////////////////////////////////////////////
	/// ReadChangesRequest
	/// One instance of this object is created for each call to AddDirectory().

	class Request
	{
	public:
		Request(Worker *server, const std::wstring &dirName,
			bool watchSubtree, DWORD filter, DWORD size);

		~Request();

		///
		bool OpenDirectory();
		///
		void BeginRead();
		///
		void BackupBuffer(DWORD size);
		///
		void ProcessNotification();
		///
		void RequestTermination();

		///
		Worker * GetWorker()
		{
			return m_worker;
		}

		///
		const std::wstring & GetDirectoryName() const
		{
			return m_dirName;
		}

	protected:
		static void CALLBACK NotificationCompletion(DWORD errorCode,	// completion code
			DWORD cBytesTransfered,			// number of bytes transferred
			LPOVERLAPPED overlapped);		// I/O information buffer

	protected:	
		///
		Worker				*m_worker;

		///
		HANDLE				m_hDirectory;
		/// parameters for ReadDirectoryChangesW
		std::wstring		m_dirName;
		bool				m_watchSubtree;
		DWORD				m_flags;
		OVERLAPPED			m_overlapped;

		// Data buffer for the request.
		// Since the memory is allocated by malloc, it will always
		// be aligned as required by ReadDirectoryChangesW().
		std::vector<BYTE>	m_buffer;

		// Double buffer strategy so that we can issue a new read
		// request before we process the current buffer.
		std::vector<BYTE>	m_backupBuffer;
	};

	////////////////////////////////////////////////////////////////////////////////
	/// Worker
	///
	/// All functions in DirectoryChanges run in the context of the worker thread.
	/// One instance of this object is allocated for each instance of DirectoryChanges.
	/// This class is responsible for thread startup, orderly thread shutdown, and shimming
	/// the various C++ member functions with C-style Win32 functions.

	class Worker
	{
	public:
		///
		Worker(DirectoryChanges *parent)
			: m_parent(parent)
			, m_terminate(false)
			, m_nRequests(0)
		{
		}

		///
		static unsigned int WINAPI ThreadStartProc(void *args)
		{
			Worker *worker = (Worker *) args;
			worker->Run();
			return 0;
		}

		/// Called by QueueUserAPC to start orderly shutdown.
		static void CALLBACK TerminateProc(__in ULONG_PTR arg)
		{
			Worker *worker = (Worker *) arg;
			worker->RequestTermination();
		}

		/// Called by QueueUserAPC to add another directory.
		static void CALLBACK AddDirectoryProc(__in  ULONG_PTR arg)
		{
			Request *request = (Request *) arg;
			request->GetWorker()->AddDirectory(request);
		}

		/// Called by QueueUserAPC to remove a directory.
		static void CALLBACK RemoveDirectoryProc(__in ULONG_PTR arg)
		{
			Request *request = (Request *) arg;
			request->GetWorker()->RemoveDirectory(request);
		}

		///
		inline DirectoryChanges * GetParent()
		{
			return m_parent;
		}

	public:
		volatile DWORD	m_nRequests;

	protected:
		///
		void Run()
		{
			while (m_nRequests || !m_terminate)
			{
				::SleepEx(INFINITE, TRUE);
			}
		}

		///
		void AddDirectory(Request *request)
		{
			if (request->OpenDirectory())
			{
				::InterlockedIncrement(&request->GetWorker()->m_nRequests);
				m_requests.push_back(request);
				request->BeginRead();
			}
			else
			{
				delete request;
			}
		}

		///
		void RemoveDirectory(Request *remRequest)
		{
			// the iterator that points to the request to be removed
			std::vector<Request*>::iterator itReq = m_requests.end();

			for (std::vector<Request*>::iterator it = m_requests.begin(); it != m_requests.end(); ++it)
			{
				if (0 == _wcsicmp((*it)->GetDirectoryName().c_str(), remRequest->GetDirectoryName().c_str()))
				{
					itReq = it;
					break;
				}
			}

			//
			if (itReq != m_requests.end())
			{
				::InterlockedDecrement(&((*itReq)->GetWorker()->m_nRequests));
				(*itReq)->RequestTermination();
			}

			//
			delete remRequest;
			m_requests.erase(itReq);
		}

		///
		void RequestTermination()
		{
			m_terminate = true;

			for (DWORD i = 0; i < m_requests.size(); ++i)
			{
				// Each request will delete itself.
				m_requests[i]->RequestTermination();
			}

			m_requests.clear();
		}

	protected:
		DirectoryChanges		*m_parent;
		bool					m_terminate;
		std::vector<Request *>	m_requests;
	};

	////////////////////////////////////////////////////////////////////////////////
	/// DirectoryChanges

	class DirectoryChanges
	{
	public:
		DirectoryChanges(int maxChanges = 1000);
		~DirectoryChanges();

		///
		void Init();
		///
		void Terminate();

		///
		void AddDirectory(const std::wstring &, bool watchSubtree, 
			DWORD notifFilter, DWORD buffSize = 16384);

		///
		void RemoveDirectory(const std::wstring &);

		///
		HANDLE GetWaitHandle()
		{
			return m_notifications.GetWaitHandle();
		}

		///
		bool Pop(DWORD &action, std::wstring &fileName);
		///
		void Push(DWORD action, const std::wstring &fileName);

		///
		/// Check if the queue is overflowed. If so, clear it and return true.
		bool CheckOverflow()
		{
			bool b = m_notifications.IsOverflow();
			if (b)
				m_notifications.clear();

			return b;
		}

		///
		unsigned int GetThreadId() 
		{ 
			return m_threadId; 
		}

		//
		struct INotifications
		{
			virtual void OnEvent(int action, const std::wstring & fileName) = 0;
		};

		void SetListener(INotifications *listener)
		{
			if (!listener)
				return;

			if (m_listener)
				return;

			m_listener = listener;
		}

		void UnSetListener()
		{
			m_listener = nullptr;
		}

	private:
		//
		HANDLE											m_hThread;
		unsigned int									m_threadId;
		//
		Worker											*m_worker;
		//
		ThreadSafeQueue<DirectoryChangeNotification>	m_notifications;

		INotifications									*m_listener;
	};

} // namespace watch