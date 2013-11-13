
#pragma once

#include <OCIdl.h>
#include "CUnknown.h"
#include "lock.h"
#include "DirectoryWatcher.h"

#include "FoldersMonitorServer_h.h"

///////////////////////////////////////////////////////////////////////////////
#define IMPLEMENT_COMPOSITE_UNKNOWN(OuterClassName, InnerClassName, DataMemberName)\
	OuterClassName * This() {\
		return (OuterClassName *)((BYTE *)this - FIELD_OFFSET(OuterClassName, DataMemberName));\
	}\
	\
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv) {\
		return This()->QueryInterface(riid, ppv);\
	}\
	ULONG __stdcall AddRef() {\
		return This()->AddRef();\
	}\
	ULONG __stdcall Release() {\
		return This()->Release();\
	}\

////////////////////////////////////////////////////////////////////////////////
// CoFoldersMonitor : the implementation of the service

class CoFoldersMonitor
	: public com::base::CUnknown
	, public IFoldersMonitor
	, public IExternalConnection
	, public IConnectionPointContainer
{
public:
	/// the delegating IUnknown
	DECLARE_IUNKNOWN;

	///
	CoFoldersMonitor(IUnknown *pUnkOuter);

	/// the non-delegating QI
	STDMETHODIMP NDQueryInterface(const IID &riid, void **ppv);

	// The factory method.
	static HRESULT CreateInstance(IUnknown *pUnknownOuter, CUnknown **ppNewComponent);

#if 0
	/// IDispatch
	///
	STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);

	STDMETHODIMP GetTypeInfo(UINT itinfo, 
		LCID lcid, ITypeInfo** pptinfo);

	STDMETHODIMP GetIDsOfNames(REFIID riid, 
		LPOLESTR* rgszNames, UINT cNames,
		LCID lcid, DISPID* rgdispid);

	STDMETHODIMP Invoke(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, 
		DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr);
#endif

	/// IExternalConnection
	///
	STDMETHODIMP_(DWORD) AddConnection(DWORD extconn, DWORD);
	STDMETHODIMP_(DWORD) ReleaseConnection(DWORD extconn, DWORD, 
		BOOL bLastReleaseKillsStub);

	/// IConnectionPointContainer
	///
	STDMETHODIMP EnumConnectionPoints(/* [out] */ IEnumConnectionPoints **ppEnum);

	STDMETHODIMP FindConnectionPoint(/* [in] */ const IID &riid,
		/* [out] */ IConnectionPoint **ppCP);

	/// IFoldersMonitor
	///
	STDMETHODIMP Start(__in int maxTasksCount);
	STDMETHODIMP Stop();
	STDMETHODIMP CreateTask(__in BSTR folderName, __in DWORD flags, __out BSTR *taskId);
	STDMETHODIMP RemoveTask(__in BSTR taskId, __out int *error);
	STDMETHODIMP StartTask(__in BSTR taskId, __out int *error);
	STDMETHODIMP StopTask(__in BSTR taskId, __out int *error);
	//STDMETHODIMP UpdateNotificationFlags(BSTR taskId, NotificationFlags setFlags, NotificationFlags resetFlags);
	//STDMETHODIMP GetNotificationFlags(BSTR taskId, NotificationFlags *flags);

protected:
	virtual ~CoFoldersMonitor();

private:
	STDMETHODIMP Fire_OnNameChanged(__in int action, __in BSTR fileName);
	STDMETHODIMP Fire_OnAttributesChanged(__in int action, __in BSTR fileName);
	STDMETHODIMP Fire_OnLastWriteChanged(__in int action, __in BSTR fileName);

	///
	static unsigned int WINAPI WorkerThreadProc(LPVOID args);

private:
	////////////////////////////////////////////////////////////////////////////
	// Events listener.

	typedef std::list<IFoldersMonitorEvents *> EventsListeners;
	EventsListeners m_events;

	////////////////////////////////////////////////////////////////////////////
	// CPFolderMonitorEvents

	class CPFolderMonitorEvents
		: public IConnectionPoint
	{
	public:
		CPFolderMonitorEvents() {}
		~CPFolderMonitorEvents() {}

		////////////////////////////////////////////////////////////////////////
		// IUnknown
		IMPLEMENT_COMPOSITE_UNKNOWN(CoFoldersMonitor, CPFolderMonitorEvents, m_cpEvents);

		/////////////////////
		// IConnectionPoint

		virtual HRESULT STDMETHODCALLTYPE GetConnectionInterface(/* [out] */ IID *pIID);

		virtual HRESULT STDMETHODCALLTYPE GetConnectionPointContainer(/* [out] */ IConnectionPointContainer **ppCPC);

		virtual HRESULT STDMETHODCALLTYPE Advise(/* [in] */ IUnknown *pUnkSink,
			/* [out] */ DWORD *pdwCookie);

		virtual HRESULT STDMETHODCALLTYPE Unadvise(/* [in] */ DWORD dwCookie);

		virtual HRESULT STDMETHODCALLTYPE EnumConnections(/* [out] */ IEnumConnections ** /*ppEnum*/)
		{
			return E_NOTIMPL;
		}
	};

	CPFolderMonitorEvents	m_cpEvents;
	CsLock					m_adviseLock;

private:
	// the outstanding references to this service
	long										m_cStrongLocks;
	// the directory monitoring service
	std::unique_ptr<watch::DirectoryChanges>	m_watcher;

	//
	HANDLE			m_hWorkerThread;
	unsigned int	m_workerThreadId;
	HANDLE			m_hStopWorker;

	//
	struct Task
	{
		std::wstring	folderName;
		DWORD			flags;
		bool			running;

		Task()
			: flags(0)
			, running(false)
		{
		}
	};

	// key: the ID of the task which is in charge of monitoring the folder
	// value: the properties of the task
	typedef std::wstring			TaskKey;
	typedef Task *					TaskPtr;

	typedef std::map<TaskKey, TaskPtr>	TasksMap;
	typedef TasksMap::const_iterator	TasksMapConstIt;
	typedef TasksMap::iterator			TasksMapIt;

	// the map of the monitoring tasks
	TasksMap m_tasksMap;
};