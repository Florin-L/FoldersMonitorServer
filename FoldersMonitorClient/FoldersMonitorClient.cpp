// FoldersMonitorClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <crtdbg.h>
#include <Unknwn.h>
#include <objbase.h>
#include <OCIdl.h>
#include <ObjIdl.h>
#include <comdef.h>
#include <comip.h>
#include <iostream>
#include <thread>

#include "..\FoldersMonitorServer\FoldersMonitorServer_h.h"
#include "..\FoldersMonitorServer\FoldersMonitorServer_i.c"

int _tmain(int argc, _TCHAR* argv[])
{
	HRESULT hr = ::CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return hr;

	_COM_SMARTPTR_TYPEDEF(IUnknown, __uuidof(IUnknown));
	_COM_SMARTPTR_TYPEDEF(IClassFactory, __uuidof(IClassFactory));
	_COM_SMARTPTR_TYPEDEF(IFoldersMonitor, __uuidof(IFoldersMonitor));

	//
	//IGlobalInterfaceTablePtr pGlobalItfTable;

	//hr = ::CoCreateInstance(CLSID_StdGlobalInterfaceTable, nullptr,
	//	CLSCTX_INPROC_SERVER,
	//	IID_IGlobalInterfaceTable,
	//	(void **) &pGlobalItfTable);

	//_ASSERT(SUCCEEDED(hr));
	//pGlobalItfTable->AddRef();

	//
	IUnknownPtr pUnk;
	hr = ::CoCreateInstance(CLSID_CoFoldersMonitor, nullptr, 
		CLSCTX_LOCAL_SERVER, 
		IID_IUnknown, (void**) &pUnk);

	_ASSERT(SUCCEEDED(hr));
	pUnk->AddRef();

	//
	IFoldersMonitorPtr pFoldersMonitor;

	hr = pUnk->QueryInterface<IFoldersMonitor>(&pFoldersMonitor);
	_ASSERT(SUCCEEDED(hr));

	//DWORD cookie = 0;
	//hr = pGlobalItfTable->RegisterInterfaceInGlobal((IUnknown*) pFoldersMonitor,
	//	IID_IFoldersMonitor, &cookie);

	//_ASSERT(SUCCEEDED(hr));

	hr = pFoldersMonitor->Start(1000);
	_ASSERT(SUCCEEDED(hr));

	_bstr_t folder(L"d:\\temp");

	BSTR taskId = nullptr;
	hr = pFoldersMonitor->CreateTask(folder, FILE_NOTIFY_CHANGE_FILE_NAME, &taskId);
	_ASSERT(SUCCEEDED(hr));

	std::wcout << _bstr_t(taskId) << std::endl;

	//
	//std::thread t([/*cookie,*/ taskId, pFoldersMonitor] {
	//	HRESULT hr = S_OK;

	//hr = ::CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	//_ASSERT(SUCCEEDED(hr));

	//IGlobalInterfaceTablePtr pGlobalItfTable;	// local to this thread

	//hr = ::CoCreateInstance(CLSID_StdGlobalInterfaceTable, nullptr,
	//	CLSCTX_INPROC_SERVER,
	//	IID_IGlobalInterfaceTable,
	//	(void **) &pGlobalItfTable);

	//_ASSERT(SUCCEEDED(hr));
	//pGlobalItfTable->AddRef();

	//IFoldersMonitorPtr pFoldersMonitor;
	//hr = pGlobalItfTable->GetInterfaceFromGlobal(cookie, IID_IFoldersMonitor,
	//	(void **) &pFoldersMonitor);

	//_ASSERT(SUCCEEDED(hr));

	//::CoUninitialize();
	//});

	//
	int error = 0;
	hr = pFoldersMonitor->StartTask(taskId, &error);
	_ASSERT(SUCCEEDED(hr) && !error);	

	//
	BSTR taskId1;
	_bstr_t folder1(L"d:\\tmp");

	hr = pFoldersMonitor->CreateTask(folder1, FILE_NOTIFY_CHANGE_FILE_NAME, &taskId1);
	_ASSERT(SUCCEEDED(hr));

	std::wcout << _bstr_t(taskId1) << std::endl;

	hr = pFoldersMonitor->StartTask(taskId1, &error);
	_ASSERT(SUCCEEDED(hr) && !error);	

	//
	//t.join();
	//::SysFreeString(taskId);

	std::wcout << L"waiting ..." << std::endl;
	getc(stdin);

	error = 0;
	hr = pFoldersMonitor->StopTask(taskId, &error);
	_ASSERT(SUCCEEDED(hr) && !error);

	//
	error = 0;
	hr = pFoldersMonitor->RemoveTask(taskId, &error);
	_ASSERT(SUCCEEDED(hr) && !error);

	hr = pFoldersMonitor->RemoveTask(taskId1, &error);
	_ASSERT(SUCCEEDED(hr) && !error);

	//
	hr = pFoldersMonitor->Stop();
	_ASSERT(SUCCEEDED(hr));

	//
	::SysFreeString(taskId);

	//
	//hr = pGlobalItfTable->RevokeInterfaceFromGlobal(cookie);
	//_ASSERT(SUCCEEDED(hr));

	::CoUninitialize();

	return 0;
}

