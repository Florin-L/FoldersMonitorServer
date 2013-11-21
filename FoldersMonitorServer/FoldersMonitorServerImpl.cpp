#include "stdafx.h"

#include <process.h>
#include "Trace.h"
#include "FoldersMonitorServerImpl.h"

#pragma comment(lib, "Rpcrt4.lib")

#if defined(_DEBUG)
#pragma comment(lib, "comsuppwd.lib")
#else
#pragma comment(lib, "comsuppw.lib")
#endif

////
#define set_error(perror, value) \
	if (perror) \
	*perror = value; \

////////////////////////////////////////////////////////////////////////////////
/// Manages the global per-process reference counter.
extern void ModuleAddRef();
extern void ModuleReleaseRef();

extern int g_majorVer;
extern int g_minorVer;

////////////////////////////////////////////////////////////////////////////////
///
CoFoldersMonitor::CoFoldersMonitor(IUnknown *pUnkOuter)
	: com::base::CUnknown(pUnkOuter)
	, m_typeInfo(nullptr)
	, m_cStrongLocks(0)
	, m_watcher()
	, m_hWorkerThread(nullptr)
	, m_workerThreadId(0)
	, m_hStopWorker(nullptr)
{
	//
	m_hStopWorker = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);
	//
	ModuleAddRef();
}

///
CoFoldersMonitor::~CoFoldersMonitor()
{
	if (m_hStopWorker)
	{
		::SetEvent(m_hStopWorker);

		DWORD rc = ::WaitForSingleObject(m_hWorkerThread, 1000);
		if (rc != WAIT_OBJECT_0)
			com::Trace(L"[%s] : failed to stop the worker !\n", __FUNCTIONW__);

		::CloseHandle(m_hWorkerThread);
		m_hWorkerThread = nullptr;
		m_workerThreadId = 0;

		::CloseHandle(m_hStopWorker);
		m_hStopWorker = nullptr;
	}

	m_tasksMap.clear();

	//
	if (m_typeInfo)
		m_typeInfo->Release();

	//
	ModuleReleaseRef();
}

///
HRESULT CoFoldersMonitor::Init()
{
	// load the type library
	//DebugBreak();
	HRESULT hr = LoadTypeInfo(&m_typeInfo, LIBID_FoldersMonitorLib, IID_IFoldersMonitor, 0);

	com::Trace(L"[%s] : LoadTypeInfo returned %d", hr);
	_ASSERT(SUCCEEDED(hr));

	return S_OK;
}

///
HRESULT CoFoldersMonitor::LoadTypeInfo(ITypeInfo **ppTypeInfo, const CLSID &libId,
									   const CLSID &iid, LCID lcid)
{
	HRESULT hr;
	LPTYPELIB ptlib = nullptr;
	LPTYPEINFO ptinfo = nullptr;

	*ppTypeInfo = nullptr;

	// Load type library.
	hr = ::LoadRegTypeLib(libId, g_majorVer, g_minorVer, lcid, &ptlib);
	if (FAILED(hr))
	{
		com::Trace(L"[%s] : LoadRegTypeLib failed ! (%d)", hr);
		return hr;
	}

	// Get the information for the interface of the object.
	hr = ptlib->GetTypeInfoOfGuid(iid, &ptinfo);
	if (FAILED(hr))
	{
		com::Trace(L"[%s] : GetTypeInfoOfGuid failed ! (%d)", hr);
		ptlib->Release();
		return hr;
	}

	ptlib->Release();
	*ppTypeInfo = ptinfo;

	return NOERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// IUnknown

///
STDMETHODIMP CoFoldersMonitor::NDQueryInterface(REFIID riid, void **ppv)
{
	if (IsEqualIID(IID_IDispatch, riid))
	{
		return FinishQI(static_cast<IDispatch*>(this), ppv);
	}
	else if (IsEqualIID(IID_IFoldersMonitor, riid))
	{
		com::Trace(L"[%s] : %s", __FUNCTIONW__, L"Query for: IID_IFoldersMonitor\n");

		return FinishQI(static_cast<IFoldersMonitor*>(this), ppv);
	}
	else if (IsEqualIID(IID_IConnectionPointContainer, riid))
	{
		return FinishQI(static_cast<IConnectionPointContainer*>(this), ppv);
	}
	else
	{
		return com::base::CUnknown::NDQueryInterface(riid, ppv);
	}
}

////////////////////////////////////////////////////////////////////////////////
///
HRESULT CoFoldersMonitor::CreateInstance(IUnknown *pUnknownOuter, CUnknown **ppNewComponent)
{
	if (pUnknownOuter != nullptr)
		return CLASS_E_NOAGGREGATION;	// no aggregation

	*ppNewComponent = new CoFoldersMonitor(pUnknownOuter);
	com::Trace(L"[%s] : %s", __FUNCTIONW__, L"The instance was created.\n");

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
/// IDispatch

///
STDMETHODIMP CoFoldersMonitor::GetTypeInfoCount(UINT* pctinfo)
{
	*pctinfo = 1;
	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::GetTypeInfo(UINT itinfo, 
										   LCID lcid, ITypeInfo** pptinfo)
{
	*pptinfo = nullptr;

	if (itinfo != 0)
		return ResultFromScode(DISP_E_BADINDEX);

	//
	m_typeInfo->AddRef();
	*pptinfo = m_typeInfo;

	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::GetIDsOfNames(REFIID riid, 
											 LPOLESTR* rgszNames, UINT cNames,
											 LCID lcid, DISPID* rgdispid)
{
	return DispGetIDsOfNames(m_typeInfo, rgszNames, cNames, rgdispid);
}

///
STDMETHODIMP CoFoldersMonitor::Invoke(DISPID dispidMember, REFIID riid,
									  LCID lcid, WORD wFlags, 
									  DISPPARAMS* pdispparams, VARIANT* pvarResult,
									  EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
	HRESULT hr = DispInvoke(static_cast<IDispatch*>(this), 
		m_typeInfo,
		dispidMember,
		wFlags,
		pdispparams,
		pvarResult,
		pexcepinfo,
		puArgErr);

	_ASSERT(SUCCEEDED(hr));

	return hr;
}

////////////////////////////////////////////////////////////////////////////////
/// IExternalConnection

///
STDMETHODIMP_(DWORD) CoFoldersMonitor::AddConnection(DWORD extconn, DWORD)
{
	com::Trace(L"[%s]\n", __FUNCTIONW__);

	if (extconn & EXTCONN_STRONG)
	{
		com::Trace(L"[%s] : new connection\n", __FUNCTIONW__);
		return ::InterlockedIncrement(&m_cStrongLocks);
	}

	return 0;
}

///
STDMETHODIMP_(DWORD) CoFoldersMonitor::ReleaseConnection(DWORD extconn, DWORD,
														 BOOL bLastReleaseKillsStub)
{
	com::Trace(L"[%s]\n", __FUNCTIONW__);

	if (extconn & EXTCONN_STRONG)
	{
		LONG res = ::InterlockedDecrement(&m_cStrongLocks);

		if ((0 == res) && bLastReleaseKillsStub)
		{
			com::Trace(L"[%s] : no more locks; disconnect the server\n", __FUNCTIONW__);
			::CoDisconnectObject(static_cast<IExternalConnection*>(this), 0);
		}

		return res;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// IConnectionPointContainer

///
STDMETHODIMP CoFoldersMonitor::EnumConnectionPoints(IEnumConnectionPoints **)
{
	return E_NOTIMPL;
}

///
STDMETHODIMP CoFoldersMonitor::FindConnectionPoint(const IID &riid, IConnectionPoint **ppCP)
{
	_ASSERT(ppCP);

	if (!ppCP)
		return E_INVALIDARG;

	if (IsEqualIID(IID_IFoldersMonitorEvents, riid))
	{
		*ppCP = &m_cpEvents;
	}
	else
	{
		*ppCP = nullptr;
		return CONNECT_E_NOCONNECTION;
	}

	if (*ppCP)
		reinterpret_cast<IUnknown*>(*ppCP)->AddRef();

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int CoFoldersMonitor::WorkerThreadProc(LPVOID args)
{
	CoFoldersMonitor *pThis = (CoFoldersMonitor *) args;

	HANDLE handles[] = {pThis->m_hStopWorker, pThis->m_watcher->GetWaitHandle()};

	for (;;)
	{
		DWORD rc = ::WaitForMultipleObjects(_countof(handles), handles, FALSE, INFINITE);
		switch (rc)
		{
		case WAIT_OBJECT_0:
			com::Trace(L"[%s] : stop event was signaled; get out the worker thread\n", __FUNCTIONW__);
			return 0;

		case WAIT_OBJECT_0 + 1:
			{
				// we have received a notification in the queue
				DWORD action;
				std::wstring fileName;

				if (pThis->m_watcher->CheckOverflow())
				{
					com::Trace(L"[%s] : Queue overflowed !\n", __FUNCTIONW__);
				}
				else
				{
					pThis->m_watcher->Pop(action, fileName);
					com::Trace(L"[%s] : action : %d, file name: %s\n", __FUNCTIONW__, action, fileName.c_str());
				}
			}
			break;

		case WAIT_IO_COMPLETION:
			// nothing to do
			break;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// IFoldersMonitor

///
STDMETHODIMP CoFoldersMonitor::Start(int maxTasksCount)
{
	if (maxTasksCount <= 0)
		return E_INVALIDARG;

	m_watcher.reset(new watch::DirectoryChanges(maxTasksCount));
	m_watcher->SetListener(static_cast<watch::DirectoryChanges::INotifications*>(this));

	// kick off the working thread
	m_hWorkerThread = (HANDLE) _beginthreadex(nullptr, 0, 
		CoFoldersMonitor::WorkerThreadProc,
		this,
		0,
		&m_workerThreadId);

	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::Stop()
{
	_ASSERT(m_hStopWorker);

	m_watcher->UnSetListener();

	if (m_hStopWorker)
		::SetEvent(m_hStopWorker);

	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::CreateTask(__in BSTR folderName, __in DWORD flags, __out BSTR *taskGuid)
{
	std::wstring id;

	//
	GUID guid;
	HRESULT hr = ::CoCreateGuid(&guid);
	if (SUCCEEDED(hr))
	{
		wchar_t *sGuid = nullptr;

		if (::UuidToString(&guid, (RPC_WSTR *)&sGuid) == RPC_S_OK)
		{
			id = sGuid;
			::RpcStringFree((RPC_WSTR *)&sGuid);
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}

	//
	TaskPtr pTask = new Task();

	pTask->folderName = _bstr_t(folderName, false);
	pTask->flags = flags;
	pTask->running = false;

	m_tasksMap.insert(std::pair<TaskKey, TaskPtr>(id, pTask));

	//
	*taskGuid = ::SysAllocString(id.c_str());

	com::Trace(L"[%s] : task %s was created\n", __FUNCTIONW__, id.c_str());

	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::RemoveTask(__in BSTR taskId, __out int *error)
{
	_bstr_t bstrTaskId(taskId, false);

	TasksMapIt it = m_tasksMap.find((const wchar_t *) bstrTaskId);
	if (it == m_tasksMap.end())
	{
		com::Trace(L"[%s] : task %s was not found\n", __FUNCTIONW__, (const wchar_t *) bstrTaskId);
		set_error(error, ERROR_NOT_FOUND);
	}
	else
	{
		m_tasksMap.erase(it);

		com::Trace(L"[%s] : task %s was removed\n", __FUNCTIONW__, (const wchar_t *) bstrTaskId);
		set_error(error, 0);
	}

	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::StartTask(__in BSTR taskId, __out int *error)
{
	_bstr_t bstrTaskId(taskId, false);

	TasksMapIt it = m_tasksMap.find((const wchar_t *) bstrTaskId);
	if (it == m_tasksMap.end())
	{
		com::Trace(L"[%s] : task %s can not be started (there is no task with the given ID)\n",
			__FUNCTIONW__, (const wchar_t *) bstrTaskId);

		set_error(error, ERROR_NOT_FOUND);
		return S_FALSE;
	}

	Task *task = it->second;
	_ASSERT(task != nullptr);

	if (task->running)
	{
		com::Trace(L"[%s] : task %s is already running\n", __FUNCTIONW__, (const wchar_t *) bstrTaskId);

		set_error(error, ERROR_THREAD_ALREADY_IN_TASK);
		return S_FALSE;
	}

	m_watcher->AddDirectory(task->folderName, false, task->flags);

	com::Trace(L"[%s] : task %s has started\n", __FUNCTIONW__, (const wchar_t *) bstrTaskId);
	task->running = true;

	set_error(error, 0);

	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::StopTask(__in BSTR taskId, __out int *error)
{
	_bstr_t bstrTaskId(taskId, false);

	TasksMapIt it = m_tasksMap.find((const wchar_t *) bstrTaskId);
	if (it == m_tasksMap.end())
	{
		com::Trace(L"[%s] : task %s can not be halted (there is no task with the given ID)\n",
			__FUNCTIONW__, (const wchar_t *) bstrTaskId);


		return S_FALSE;
	}

	Task *task = it->second;
	_ASSERT(task != nullptr);

	task->running = false;
	set_error(error, 0);

	com::Trace(L"[%s] : task %s has stopped\n", __FUNCTIONW__, (const wchar_t *) bstrTaskId);

	return S_OK;
}

/////
//STDMETHODIMP CoFoldersMonitor::UpdateNotificationFlags(BSTR taskId, NotificationFlags setFlags,
//													   NotificationFlags resetFlags)
//{
//	return E_NOTIMPL;
//}
//
/////
//STDMETHODIMP CoFoldersMonitor::GetNotificationFlags(BSTR taskId, NotificationFlags *flags)
//{
//	return E_NOTIMPL;
//}

////////////////////////////////////////////////////////////////////////////////
/// INotification::OnEvent

///
void CoFoldersMonitor::OnEvent(int action, const std::wstring & fileName)
{
	std::for_each(std::begin(m_events), std::end(m_events), [action, fileName](IFoldersMonitorEvents * handler) {
		handler->OnChanged(action, _bstr_t(fileName.c_str()).copy());
	});
}

////////////////////////////////////////////////////////////////////////////////
/// CoFoldersMonitor::CPFolderMonitorEvents

///
STDMETHODIMP CoFoldersMonitor::CPFolderMonitorEvents::GetConnectionInterface(IID *piid)
{
	_ASSERT(piid);
	if (!piid)
		return E_INVALIDARG;

	// returns the IID of the interface
	*piid = IID_IFoldersMonitorEvents;
	return S_OK;
}

///
STDMETHODIMP CoFoldersMonitor::CPFolderMonitorEvents::GetConnectionPointContainer(IConnectionPointContainer **ppcpc)
{
	_ASSERT(ppcpc);
	if (!ppcpc)
		return E_INVALIDARG;

	// returns the containing object
	(*ppcpc = This())->AddRef();
	return S_OK;
}

//
STDMETHODIMP CoFoldersMonitor::CPFolderMonitorEvents::Advise(IUnknown *pUnkSink, DWORD *pdwCookie)
{
	_ASSERT(pUnkSink && pdwCookie);

	if (!pUnkSink || !pdwCookie)
		return E_INVALIDARG;

	*pdwCookie = 0;
	HRESULT hr = S_OK;

	do
	{
		ScopedLock<CsLock> cs(This()->m_adviseLock);

		IFoldersMonitorEvents *pEvents = nullptr;

		HRESULT hr = pUnkSink->QueryInterface(IID_IFoldersMonitorEvents, (void**) &pEvents);
		if (E_NOINTERFACE == hr)
			hr = CONNECT_E_NOCONNECTION;

		if (SUCCEEDED(hr))
		{
			This()->m_events.push_back(pEvents);
			*pdwCookie = DWORD(pEvents);
		}
	} while (0);

	return hr;
}

///
STDMETHODIMP CoFoldersMonitor::CPFolderMonitorEvents::Unadvise(DWORD dwCookie)
{
	HRESULT hr = S_OK;

	do 
	{
		IFoldersMonitorEvents *pEvents = (IFoldersMonitorEvents*)dwCookie;
		_ASSERT(pEvents);

		ScopedLock<CsLock> cs(This()->m_adviseLock);

		EventsListeners::iterator it = 
			std::find(This()->m_events.begin(), This()->m_events.end(), pEvents);

		if (it == This()->m_events.end())
		{
			// the cookie does not correspond to a valid connection
			hr = CONNECT_E_NOCONNECTION;
			break;
		}

		// release the connection
		pEvents->Release();
		pEvents = nullptr;

		This()->m_events.erase(it);
	} while (0);

	return S_OK;
}
