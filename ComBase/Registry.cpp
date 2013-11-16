
#include <objbase.h>
#include <crtdbg.h>
#include <string>
#include <Shlwapi.h>
#include "Registry.h"

#pragma comment(lib, "shlwapi.lib")

////////////////////////////////////////////////////////
//
// Constants
//

// Size of a GUID as a string
static const int GUID_STRING_SIZE = 39 ;
//
static const std::wstring kEmptyString = L"";

////////////////////////////////////////////////////////
//
// Internal helper functions prototypes
//

// Set the given key and its value.
static BOOL SetKeyAndValue(const std::wstring & key,
						   const std::wstring & subkey,
						   const std::wstring & value)
{
	HKEY hKey;

	std::wstring keyBuf = key;

	// Add subkey name to buffer.
	if (!subkey.empty())
	{
		keyBuf.append(L"\\");
		keyBuf.append(subkey);
	}

	// Create and open key and subkey.
	long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,
		keyBuf.c_str(), 
		0, NULL, REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, 
		&hKey, NULL);

	if (lResult != ERROR_SUCCESS)
	{
		return FALSE;
	}

	// Set the Value.
	if (!value.empty())
	{
		RegSetValueEx(hKey, NULL, 0, REG_SZ, 
			reinterpret_cast<BYTE *>(const_cast<wchar_t*>(value.c_str())), 
			(DWORD)(value.length() * sizeof(wchar_t)));
	}

	RegCloseKey(hKey);

	return TRUE ;
}

// Convert a GUID into a char string.
static void GUIDtoStr(const GUID & guid, 
					  std::wstring & sGUID)
{
	// Get wide string version.
	LPOLESTR wszGUID = nullptr;
	HRESULT hr = StringFromCLSID(guid, &wszGUID);
	_ASSERT(SUCCEEDED(hr));

	sGUID = wszGUID;

	// Free memory.
	CoTaskMemFree(wszGUID);
}

// Determine if a particular subkey exists.
static BOOL SubkeyExists(const std::wstring &path,
						 const std::wstring &subkey)
{
	HKEY hKey;

	std::wstring keyBuf = path;

	// Add subkey name to buffer.
	if (!subkey.empty())
	{
		keyBuf.append(L"\\");
		keyBuf.append(subkey);
	}

	// Determine if key exists by trying to open it.
	LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, 
		keyBuf.c_str(),
		0,
		KEY_ALL_ACCESS,
		&hKey);

	if (lResult == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return TRUE;
	}

	return FALSE;
}

// Delete szKeyChild and all of its descendents.
static LONG RecursiveDeleteKey(HKEY hKeyParent, const wchar_t * lpszKeyChild)
{
	// Open the child.
	HKEY hKeyChild;
	LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
		KEY_ALL_ACCESS, &hKeyChild);

	if (lRes != ERROR_SUCCESS)
	{
		return lRes;
	}

	// Enumerate all of the decendents of this child.
	FILETIME time;
	wchar_t szBuffer[256];
	DWORD dwSize = 256;

	while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
		NULL, NULL, &time) == S_OK)
	{
		// Delete the decendents of this child.
		lRes = RecursiveDeleteKey(hKeyChild, szBuffer);
		if (lRes != ERROR_SUCCESS)
		{
			// Cleanup before exiting.
			RegCloseKey(hKeyChild);
			return lRes;
		}

		dwSize = 256;
	}

	// Close the child.
	RegCloseKey(hKeyChild);

	// Delete this child.
	return RegDeleteKey(hKeyParent, lpszKeyChild);
}

/////////////////////////////////////////////////////////
static LPCWSTR GetTypeLibPath(LPCWSTR modulePath)
{
	LPWSTR str = _wcsdup(modulePath);
	::PathRemoveExtensionW(str);

	_ASSERT(::PathAddExtensionW(str, L".tlb"));

	return str;
}

//
//   FUNCTION: RegisterTypeLib
//
//   PURPOSE: Register the type library.
//
//   PARAMETERS:
//   * pszTypeLib - The type library file.
//
//   NOTE: The function creates the HKCR\TypeLib\{<LIBID>} key and the 
//   HKCR\Interface\{<IID>} key in the registry.
// 
//   HKCR
//   {
//      NoRemove TypeLib
//      {
//          ForceRemove {<LIBID>}
//          {
//              1.0
//              {
//                  0
//                  {
//                      win32 = s '%MODULE%'
//                  }
//                  FLAGS = 0
//                  HELPDIR = s '%MODULE DIR%'
//              }
//          }
//      }
//      NoRemove Interface
//      {
//          ForceRemove {<IID>} = s '<Interface Name>'
//          {
//              ProxyStubClsid = s '<ProgID>'
//              ProxyStubClsid32 = s '<VersionIndependentProgID>'
//              TypeLib = s '{<LIBID>}'
//              {
//                  val Version = s '<TypeLib Version>'
//              }
//          }
//      }
//   }
//

static HRESULT RegisterTypeLib(PCWSTR pszTypeLib)
{
	HRESULT hr;
	ITypeLib *pTLB = NULL;

	hr = LoadTypeLibEx(pszTypeLib, REGKIND_REGISTER, &pTLB);
	if (SUCCEEDED(hr))
	{
		pTLB->Release();
	}

	return hr;
}

//
//   FUNCTION: UnregisterTypeLib
//
//   PURPOSE: Unregister the type library.
//
//   PARAMETERS:
//   * pszTypeLib - The type library file.
//
//   NOTE: The function deletes the HKCR\TypeLib\{<LIBID>} key and the 
//   HKCR\Interface\{<IID>} key in the registry.
//
static HRESULT UnregisterTypeLib(PCWSTR pszTypeLib)
{
	HRESULT hr;

	ITypeLib *pTLB = NULL;
	hr = LoadTypeLibEx(pszTypeLib, REGKIND_NONE, &pTLB);
	if (SUCCEEDED(hr))
	{
		TLIBATTR *pAttr = NULL;
		hr = pTLB->GetLibAttr(&pAttr);
		if (SUCCEEDED(hr))
		{
			hr = UnRegisterTypeLib(pAttr->guid, pAttr->wMajorVerNum, 
				pAttr->wMinorVerNum, pAttr->lcid, pAttr->syskind);

			pTLB->ReleaseTLibAttr(pAttr);
		}

		pTLB->Release();
	}

	return hr;
}

namespace com { namespace base {

	/////////////////////////////////////////////////////////
	//
	// Public function implementation
	//

	//
	// Register the component in the registry.
	//
	HRESULT RegisterServer(HMODULE hModule,						// DLL module handle
		const CLSID &clsid,					// Class ID
		const std::wstring &friendlyName,	// Friendly Name
		const std::wstring &verIndProgID,	// Version independent ID
		const std::wstring &progID,			// Prog ID
		const std::wstring &threadingModel,	// The threading model
		const GUID &libid)					// Library ID
	{
		// Get server location.
		std::wstring sModulePath;
		sModulePath.resize(MAX_PATH + 1);

		DWORD cSize = ::GetModuleFileName(hModule, 
			(LPWSTR) sModulePath.data(), MAX_PATH + 1);

		_ASSERT(cSize != 0);

#if 0
		wchar_t szModule[MAX_PATH + 1];

		DWORD dwResult = ::GetModuleFileName(hModule, 
			szModule, _countof(szModule));

		_ASSERT(dwResult != 0);
#endif

		// Convert the CLSID into a char.
		std::wstring sCLSID;
		sCLSID.reserve(GUID_STRING_SIZE);
		GUIDtoStr(clsid, sCLSID);

		// Build the key CLSID\\{...}
		std::wstring sKey;
		sKey = L"CLSID\\";
		sKey.append(sCLSID);

		// Add the CLSID to the registry.
		SetKeyAndValue(sKey, kEmptyString, friendlyName);

		// Add the server filename subkey under the CLSID key.
#ifdef _OUTPROC_SERVER_
		SetKeyAndValue(sKey, L"LocalServer32", sModulePath);
#else
		SetKeyAndValue(sKey, L"InprocServer32", sModulePath);
#endif

		// Add the ProgID subkey under the CLSID key.
		SetKeyAndValue(sKey, L"ProgID", progID);

		// Add the version-independent ProgID subkey under CLSID key.
		SetKeyAndValue(sKey, L"VersionIndependentProgID", verIndProgID);

		// Add the threading model subkey.
#ifdef _OUTPROC_SERVER_
		SetKeyAndValue(sKey, L"LocalServer32\\ThreadingModel", threadingModel);
#else
		SetKeyAndValue(sKey, L"InprocServer32\\ThreadingModel", threadingModel);
#endif

		// Add the Type Library ID subkey under the CLSID key.
		std::wstring sLIBID;
		sLIBID.reserve(GUID_STRING_SIZE);

		GUIDtoStr(libid, sLIBID);
		SetKeyAndValue(sKey, L"TypeLib", sLIBID);

		LPCWSTR tlbPath = GetTypeLibPath(sModulePath.c_str());
		_ASSERT(tlbPath && *tlbPath);

		RegisterTypeLib(tlbPath);
		free((void*) tlbPath);
		tlbPath = nullptr;

		// Add the version-independent ProgID subkey under HKEY_CLASSES_ROOT.
		SetKeyAndValue(verIndProgID, kEmptyString, friendlyName); 
		SetKeyAndValue(verIndProgID, L"CLSID", sCLSID);
		SetKeyAndValue(verIndProgID, L"CurVer", progID);

		// Add the versioned ProgID subkey under HKEY_CLASSES_ROOT.
		SetKeyAndValue(progID, kEmptyString, friendlyName); 
		SetKeyAndValue(progID, L"CLSID", sCLSID);

		return S_OK ;
	}

	//
	// Remove the component from the registry.
	//
	LONG UnregisterServer(HMODULE hModule,
		const CLSID & clsid,				// Class ID
		const std::wstring & verIndProgID,	// Programmatic
		const std::wstring & progID)			//   IDs
	{
		// Convert the CLSID into a char.
		std::wstring sCLSID;
		sCLSID.reserve(GUID_STRING_SIZE);

		GUIDtoStr(clsid, sCLSID);

		// Build the key CLSID\\{...}
		std::wstring sKey(L"CLSID\\");
		sKey.append(sCLSID);

		// Check for a another server for this component.
#ifdef _OUTPROC_SERVER_
		if (SubkeyExists(sKey.c_str(), L"InprocServer32"))
#else
		if (SubkeyExists(sKey.c_str(), L"LocalServer32"))
#endif
		{
			// Delete only the path for this server.
#ifdef _OUTPROC_SERVER_
			sKey.append(L"\\LocalServer32");
#else
			sKey.append(L"\\InprocServer32");
#endif
			LONG lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, sKey.c_str());
			_ASSERT(lResult == ERROR_SUCCESS);
		}
		else
		{
			// Delete all related keys.
			// Delete the CLSID Key - CLSID\{...}
			LONG lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, sKey.c_str());
			_ASSERT((lResult == ERROR_SUCCESS) ||
				(lResult == ERROR_FILE_NOT_FOUND)); // Subkey may not exist.

			// Delete the version-independent ProgID Key.
			lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, verIndProgID.c_str());
			_ASSERT((lResult == ERROR_SUCCESS) ||
				(lResult == ERROR_FILE_NOT_FOUND)); // Subkey may not exist.

			// Delete the ProgID key.
			lResult = RecursiveDeleteKey(HKEY_CLASSES_ROOT, progID.c_str());
			_ASSERT((lResult == ERROR_SUCCESS) ||
				(lResult == ERROR_FILE_NOT_FOUND)); // Subkey may not exist.
		}

		// unregister the type lib
		// Get server location.
		wchar_t szModule[MAX_PATH + 1];

		DWORD dwResult = ::GetModuleFileName(hModule, 
			szModule, _countof(szModule));

		_ASSERT(dwResult != 0);

		LPCWSTR tlbPath = GetTypeLibPath(szModule);
		_ASSERT(tlbPath && *tlbPath);

		RegisterTypeLib(tlbPath);
		free((void*) tlbPath);
		tlbPath = nullptr;
		//

		return S_OK ;
	}

#if 0
	///////////////////////////////////////////////////////////
	//
	// Internal helper functions
	//

	// Convert a GUID to a char string.
	void GUIDtoStr(const GUID & guid, 
		std::wstring & sGUID)
	{
		// Get wide string version.
		LPOLESTR wszGUID = nullptr;
		HRESULT hr = StringFromCLSID(guid, &wszGUID);
		_ASSERT(SUCCEEDED(hr));

		sGUID = wszGUID;

		// Free memory.
		CoTaskMemFree(wszGUID);
	}

	//
	// Delete a key and all of its descendents.
	//
	LONG RecursiveDeleteKey(HKEY hKeyParent,				// Parent of key to delete
		const wchar_t * lpszKeyChild)	// Key to delete
	{
		// Open the child.
		HKEY hKeyChild;
		LONG lRes = RegOpenKeyEx(hKeyParent, lpszKeyChild, 0,
			KEY_ALL_ACCESS, &hKeyChild);

		if (lRes != ERROR_SUCCESS)
		{
			return lRes;
		}

		// Enumerate all of the decendents of this child.
		FILETIME time;
		wchar_t szBuffer[256];
		DWORD dwSize = 256;

		while (RegEnumKeyEx(hKeyChild, 0, szBuffer, &dwSize, NULL,
			NULL, NULL, &time) == S_OK)
		{
			// Delete the decendents of this child.
			lRes = RecursiveDeleteKey(hKeyChild, szBuffer);
			if (lRes != ERROR_SUCCESS)
			{
				// Cleanup before exiting.
				RegCloseKey(hKeyChild);
				return lRes;
			}

			dwSize = 256;
		}

		// Close the child.
		RegCloseKey(hKeyChild);

		// Delete this child.
		return RegDeleteKey(hKeyParent, lpszKeyChild);
	}

	//
	// Determine if a particular subkey exists.
	//
	BOOL SubkeyExists(const std::wstring &path,    // Path of key to check
		const std::wstring &subkey)   // Key to check
	{
		HKEY hKey;

		std::wstring keyBuf = path;

		// Add subkey name to buffer.
		if (!subkey.empty())
		{
			keyBuf.append(L"\\");
			keyBuf.append(subkey);
		}

		// Determine if key exists by trying to open it.
		LONG lResult = ::RegOpenKeyEx(HKEY_CLASSES_ROOT, 
			keyBuf.c_str(),
			0,
			KEY_ALL_ACCESS,
			&hKey);

		if (lResult == ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return TRUE;
		}

		return FALSE;
	}

	//
	// Create a key and set its value.
	//   - This helper function was borrowed and modifed from
	//     Kraig Brockschmidt's book Inside OLE.
	//
	BOOL SetKeyAndValue(const std::wstring & key,
		const std::wstring & subkey,
		const std::wstring & value)
	{
		HKEY hKey;

		std::wstring keyBuf = key;

		// Add subkey name to buffer.
		if (!subkey.empty())
		{
			keyBuf.append(L"\\");
			keyBuf.append(subkey);
		}

		// Create and open key and subkey.
		long lResult = RegCreateKeyEx(HKEY_CLASSES_ROOT,
			keyBuf.c_str(), 
			0, NULL, REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS, NULL, 
			&hKey, NULL);

		if (lResult != ERROR_SUCCESS)
		{
			return FALSE;
		}

		// Set the Value.
		if (!value.empty())
		{
			RegSetValueEx(hKey, NULL, 0, REG_SZ, 
				reinterpret_cast<BYTE *>(const_cast<wchar_t*>(value.c_str())), 
				(DWORD)(value.length() * sizeof(wchar_t)));
		}

		RegCloseKey(hKey);

		return TRUE ;
	}
#endif
} // namespace com
} // namespace base