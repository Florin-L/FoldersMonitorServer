
#include <objbase.h>
#include "inttable.h"

//
HRESULT InterfaceTableQueryInterface(void * pThis, const INTERFACE_ENTRY * pTable, REFIID riid, void **ppv)
{
	if (::InlineIsEqualGUID(riid, IID_IUnknown))
	{
		// first entry must be an offset
		*ppv = (char*)pThis + pTable->dwData;
		reinterpret_cast<IUnknown *>(*ppv)->AddRef();

		return S_OK;
	}

	HRESULT hr = E_NOINTERFACE;
	while (pTable->pfnFinder)
	{
		if (!pTable->pIID || InlineIsEqualGUID(*pTable->pIID, riid))
		{
			if (pTable->pfnFinder == ENTRY_IS_OFFSET)
			{
				*ppv = (char*)pThis + pTable->dwData;
				reinterpret_cast<IUnknown *>(*ppv)->AddRef();

				return S_OK;
			}
			else
			{
				hr = pTable->pfnFinder(pThis, pTable->dwData, riid,ppv);
				if (hr == S_OK)
					break;
			}
		}
		pTable++;
	}

	if (hr != S_OK)
		*ppv = nullptr;

	return hr;
}