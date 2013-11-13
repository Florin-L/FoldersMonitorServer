
#include "stdafx.h"
#include "CFactory.h"
#include "FoldersMonitorServerImpl.h"
#include "FoldersMonitorServer_h.h"

//
// The following array contains the data used by CFactory
// to create components. Each element in the array contains
// the CLSID, the pointer to the creation function, and the name
// of the component to place in the Registry.
//

namespace com { namespace base {

	CFactoryData g_FactoryDataArray[] =
	{
		{&CLSID_CoFoldersMonitor, 
		CoFoldersMonitor::CreateInstance,	// factory 
		L"CoFoldersMonitor",				// Friendly Name
		L"CoFoldersMonitor",				// ProgID
		L"CoFoldersMonitor.1",				// Version-independent ProgID
		&LIBID_FoldersMonitorLib,							// Type Library ID
		nullptr, 
		0}
	};

	int g_cFactoryDataEntries = sizeof(g_FactoryDataArray) / sizeof(CFactoryData);

} // com 
} // base
