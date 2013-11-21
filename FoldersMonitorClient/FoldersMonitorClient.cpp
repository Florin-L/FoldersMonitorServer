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
#include <memory>
#include <iostream>
#include "Sink.h"

#include "..\FoldersMonitorServer\FoldersMonitorServer_h.h"
#include "..\FoldersMonitorServer\FoldersMonitorServer_i.c"

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		std::wcout << L"Usage:\n" << argv[0] << L" " << argv[1] << L" " << argv[2] << std::endl;
		exit(1);
	}

	std::wstring directoryName1 = argv[1];
	std::wstring directoryName2 = argv[2];

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
	IConnectionPointContainerPtr pcpc;
	hr = pUnk->QueryInterface<IConnectionPointContainer>(&pcpc);
	_ASSERT(SUCCEEDED(hr));

	Sink *sink = Sink::Create();
	DWORD dwCookie = 0;

	IConnectionPointPtr pcp;
	hr = pcpc->FindConnectionPoint(IID_IFoldersMonitorEvents, &pcp);
	_ASSERT(SUCCEEDED(hr));

	pcp->Advise((IUnknown*) sink, &dwCookie);

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

	_bstr_t folder(directoryName1.c_str());

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
	_bstr_t folder1(directoryName2.c_str());

	hr = pFoldersMonitor->CreateTask(folder1, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE, &taskId1);
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
	pcp->Unadvise(dwCookie);
	pcp = nullptr;

	sink->Release();

	//
	//hr = pGlobalItfTable->RevokeInterfaceFromGlobal(cookie);
	//_ASSERT(SUCCEEDED(hr));

	::CoUninitialize();

	return 0;
}

