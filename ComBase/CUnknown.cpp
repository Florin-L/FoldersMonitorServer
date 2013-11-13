
#include "CUnknown.h"
#include "CFactory.h"

namespace com { namespace base
{
	//
	long CUnknown::s_cActiveComponents = 0;

	//
	CUnknown::CUnknown(IUnknown *pUnkOuter)
		: m_cRef(1)
	{
		if (!pUnkOuter)
		{
			m_pUnknownOuter = 
				reinterpret_cast<IUnknown*>(static_cast<INDUnknown*>(this));
		}
		else
		{
			m_pUnknownOuter = pUnkOuter;
		}

		::InterlockedIncrement(&s_cActiveComponents);
	}

	//
	CUnknown::~CUnknown()
	{
		::InterlockedDecrement(&s_cActiveComponents);

		// If this is an EXE server, shut it down.
		CFactory::CloseExe();
	}

	//
	void CUnknown::FinalRelease()
	{
		m_cRef = 1;
	}

	//
	// Nondelegating IUnknown
	//   - Override to handle custom interfaces.
	//
	HRESULT __stdcall CUnknown::NDQueryInterface(const IID &iid, void **ppv)
	{
		// CUnknown supports only IUnknown.
		if (iid == IID_IUnknown)
		{
			return FinishQI(reinterpret_cast<IUnknown*>(static_cast<INDUnknown*>(this)), ppv);
		}	
		else
		{
			*ppv = NULL;
			return E_NOINTERFACE;
		}
	}

	//
	// AddRef
	//
	ULONG __stdcall CUnknown::NDAddRef()
	{
		return InterlockedIncrement(&m_cRef);
	}

	//
	// Release
	//
	ULONG __stdcall CUnknown::NDRelease()
	{
		InterlockedDecrement(&m_cRef);
		if (m_cRef == 0)
		{
			FinalRelease() ;
			delete this;
			return 0;
		}

		return m_cRef;
	}

	//
	// FinishQI
	//   - Helper function to simplify overriding
	//     NondelegatingQueryInterface
	//
	HRESULT CUnknown::FinishQI(IUnknown *pI, void **ppv) 
	{
		*ppv = pI;
		pI->AddRef();
		return S_OK;
	}

} // namespace com
} // namespace base