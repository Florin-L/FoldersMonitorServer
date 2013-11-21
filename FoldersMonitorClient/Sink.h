#pragma once

#include <iostream>
#include <comutil.h>

#include "..\FoldersMonitorServer\FoldersMonitorServer_h.h"

using namespace std;

///
class Sink
	: public IFoldersMonitorEvents
{
public:
	//
	static Sink * Create()
	{
		Sink * p = new(std::nothrow) Sink();
		if (!p)
			return nullptr;

		p->AddRef();
		return p;
	}

	//
	~Sink()
	{
		_ASSERT(0 == m_cRef);
	}

	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_IFoldersMonitorEvents, riid))
		{
			*ppv = static_cast<IFoldersMonitorEvents*>(this);
		}
		else
		{
			*ppv = nullptr;
			return E_NOINTERFACE;
		}

		reinterpret_cast<IUnknown*>(*ppv)->AddRef();
		return S_OK;
	}

	STDMETHODIMP_(ULONG) AddRef()
	{
		::InterlockedIncrement(&m_cRef);
		return m_cRef;
	}
	
	STDMETHODIMP_(ULONG) Release()
	{
		::InterlockedDecrement(&m_cRef);
		if (m_cRef == 0)
		{
			delete this;
			return 0;
		}

		return m_cRef;
	}

	///////////////////////////////////////////////////////////////////////////////
	// IFoldersMonitorEvents
    public:
		virtual /* [id] */ STDMETHODIMP OnChanged(/* [in] */ int action,
            /* [in] */ BSTR fileName)
		{
			wcout << L"OnChanged: " << action << L" / " << _bstr_t(fileName, false) << endl;
			return S_OK;
		}

private:
	Sink() : m_cRef(0) {}

	Sink(const Sink &);
	Sink & operator =(const Sink &);

private:
	long m_cRef;
};