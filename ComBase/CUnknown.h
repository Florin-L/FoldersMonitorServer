
#pragma once

#include <objbase.h>

namespace com { namespace base
{
	////////////////////////////////////////////////////////////////////////////////
	//
	interface INDUnknown
	{
		virtual HRESULT __stdcall NDQueryInterface(const IID &, void **) = 0;
		virtual ULONG __stdcall NDAddRef() = 0;
		virtual ULONG __stdcall NDRelease() = 0;
	};

	typedef INDUnknown * PNDUNKNOWN;

	////////////////////////////////////////////////////////////////////////////////
	//
	class CUnknown
		: public INDUnknown
	{
	public:
		// Implementation of INondelegatingUnknown interface

		virtual HRESULT __stdcall NDQueryInterface(const IID &, void **);
		virtual ULONG __stdcall NDAddRef();
		virtual ULONG __stdcall NDRelease();

		// Constructor
		CUnknown(IUnknown *pUnkOuter);

		// Destructor
		virtual ~CUnknown();

		// Initialization (especially for aggregates)
		virtual HRESULT Init()
		{
			return S_OK;
		}

		// Notification to derived classes that we are releasing
		virtual void FinalRelease();

		//
		static long ActiveComponents()
		{
			return s_cActiveComponents;
		}

		// Helper function
		HRESULT FinishQI(IUnknown *, void **);

	protected:
		// Support for delegation : return the owner's IUnkown.
		IUnknown * GetOuterUnknown()
		{
			return m_pUnknownOuter;
		}

	private:
		// Reference count for this object.
		long m_cRef;

		// Pointer to external outer IUnknown
		IUnknown *m_pUnknownOuter;

		// Count of all active instances.
		static long s_cActiveComponents;

	};

	// Delegating IUnknown

#define DECLARE_IUNKNOWN \
	virtual HRESULT __stdcall QueryInterface(const IID &iid, void **ppv) \
	{ \
	return GetOuterUnknown()->QueryInterface(iid, ppv); \
	}\
	virtual ULONG __stdcall AddRef() \
	{ \
	return GetOuterUnknown()->AddRef(); \
	}\
	virtual ULONG __stdcall Release() \
	{ \
	return GetOuterUnknown()->Release(); \
	}

}	// namespace com
}	// namespace base