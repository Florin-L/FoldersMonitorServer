#pragma once

#include <string>

namespace com { namespace base
{

	//
	// Registry.h
	//   - Helper functions registering and unregistering a component.
	//

	// This function will register a component in the Registry.
	// The component calls this function from its DllRegisterServer function.
	HRESULT RegisterServer(HMODULE hModule, 
		const CLSID &clsid, 
		const std::wstring &friendlyName,
		const std::wstring &verIndProgID,
		const std::wstring &progID,
		const std::wstring &threadingModel,
		const GUID &libid);

	// This function will unregister a component.  Components
	// call this function from their DllUnregisterServer function.
	HRESULT UnregisterServer(HMODULE hModule,
		const CLSID& clsid,
		const std::wstring &verIndProgID,
		const std::wstring &progID);

}	//	namespace com
}	//	namespace base