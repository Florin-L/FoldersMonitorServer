// FoldersMonitorServer.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <iostream>
#include "Trace.h"
#include "CFactory.h"

//#include "..\FoldersMonitorServer\FoldersMonitorServer_h.h"

bool g_enableTraces = true;
int g_majorVer = 0;
int g_minorVer = 0;

////////////////////////////////////////////////////////////////////////////////
/// Manages the global per-process reference counter.

///
void ModuleAddRef()
{
	ULONG cRef = ::CoAddRefServerProcess();
	com::Trace(L"[%s] : global server reference counter = %d", __FUNCTIONW__, cRef);
}

///
void ModuleReleaseRef()
{
	ULONG cRef = ::CoReleaseServerProcess();
	com::Trace(L"[%s] : global server reference counter = %d", __FUNCTIONW__, cRef);
}

///
static HRESULT LoadTypeInfo(ITypeInfo **ppTypeInfo, const CLSID &libId,
									   const CLSID &iid, LCID lcid)
{
	HRESULT hr;
	LPTYPELIB ptlib = nullptr;
	LPTYPEINFO ptinfo = nullptr;

	*ppTypeInfo = nullptr;

	// Load type library.
	hr = ::LoadRegTypeLib(libId, 0, 0, lcid, &ptlib);
	if (FAILED(hr))
	{
		com::Trace(L"[%s] : LoadRegTypeLib failed ! (%d)", __FUNCTIONW__, hr);
		return hr;
	}

	// Get the information for the interface of the object.
	hr = ptlib->GetTypeInfoOfGuid(iid, &ptinfo);
	if (FAILED(hr))
	{
		com::Trace(L"[%s] : GetTypeInfoOfGuid failed ! (%d)", __FUNCTIONW__, hr);
		ptlib->Release();
		return hr;
	}

	ptlib->Release();
	*ppTypeInfo = ptinfo;

	return NOERROR;
}

////////////////////////////////////////////////////////////////////////////////
///
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HRESULT hr = ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return hr;

	//
	com::base::CFactory::s_hModule = hInstance;
	com::base::CFactory::s_dwThreadID = ::GetCurrentThreadId();

	// self registration flags ?
	if (wcsstr(lpCmdLine, L"/RegServer") != 0)
	{
		hr = com::base::CFactory::RegisterAll();
		_ASSERT(SUCCEEDED(hr));
		com::Trace(L"The server was registered.");

		//
		//ITypeInfo *typeInfo = nullptr;
		//HRESULT r = LoadTypeInfo(&typeInfo, LIBID_FoldersMonitorLib, IID_IFoldersMonitor, 0);

		::CoUninitialize();
		return hr;
	}

	if (wcsstr(lpCmdLine, L"/UnregServer") != 0)
	{
		hr = com::base::CFactory::UnregisterAll();
		_ASSERT(SUCCEEDED(hr));
		com::Trace(L"The server was unregistered.");

		::CoUninitialize();
		return hr;
	}

	//
	if (com::base::CFactory::StartFactories())
	{
		com::Trace(L"[%s] : %s\n", __FUNCTIONW__, L"The class factories were started !");

#if 0
		HRESULT res = ::CoResumeClassObjects();
		if (FAILED(res))
			com::Trace(L"[%s] : %s%s", __FUNCTIONW__, L"CoResumeClassObjects failed !", L"\\n");
#endif

		// wait for shutdown
		MSG msg;
		while (::GetMessage(&msg, nullptr, 0, 0))  
		{
			::DispatchMessage(&msg);
		}

		// unregister the class factories
		com::base::CFactory::StopFactories();
		//
		com::Trace(L"[%s] : The server is shut down.", __FUNCTIONW__);
	}
	else
	{
		com::Trace(L"[%s] : %s\n", __FUNCTIONW__, L"Registration of the class object failed !");
	}

	::CoUninitialize();
	return 0;
}
