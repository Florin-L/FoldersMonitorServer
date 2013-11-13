#pragma once

// the extensibility function
typedef HRESULT (*INTERFACE_FINDER)(void *pThis, DWORD dwData, REFIID riid, void **ppv);

// pseudo-function  to indicate this entry is just offset
#define ENTRY_IS_OFFSET INTERFACE_FINDER(-1)

// table layout
typedef struct _INTERFACE_ENTRY
{
	const IID			*pIID;		// the IID to match
	INTERFACE_FINDER	pfnFinder;	// the extensibility function
	DWORD				dwData;		// offset data
} INTERFACE_ENTRY;

//
#define BASE_OFFSET(ClassName, BaseName) \
	((DWORD)(static_cast<BaseName*>(reinterpret_cast<ClassName*>(0x1))) - 0x1) \

#define COMPOSITE_OFFSET(ClassName, BaseName,\
	MemberType, MemberName) \
	(DWORD(static_cast<BaseName*>(\
		reinterpret_cast<MemberType*>(Ox1OOOOOOO + \
			offsetof(ClassName, MemberName)))) - OxlOOOOOOO) \

//
#define BEGIN_INTERFACE_TABLE(ClassName) \
	typedef ClassName _ITCls; \
	const INTERFACE_ENTRY * GetInterfaceTable() { \
		static const INTERFACE_ENTRY table[] = { \

#define IMPLEMENTS_INTERFACE(Itf) \
	{&IID_##Itf, ENTRY_IS_OFFSET, BASE_OFFSET(_ITCls, Itf)}, \

#define IMPLEMENTS_DISPINTERFACE(Itf) \
	{&DIID_##Itf, ENTRY_IS_OFFSET, BASE_OFFSET(_ITCls, Itf)}, \

#define IMPLEMENTS_INTERFACE_AS(ReqItf, Itf) \
	{&IID_##ReqItf, ENTRY_IS_OFFSET, BASE_OFFSET(_ITCls, Itf)}, \

#define IMPLEMENTS_INTERFACE_WITH_COMPOSITE(ReqItf, MemberType, MemberName) \
	{&IID_##ReqItf,ENTRY_IS_OFFSET, COMPOSITE_OFFSET(_ITCls, ReqItf, MemberType, MemberName)}, \

#define END_INTERFACE_TABLE() \
	{0, 0, 0} }; return table; } \

//
HRESULT InterfaceTableQueryInterface(void * pThis, const INTERFACE_ENTRY * pTable, REFIID iid, void **ppv);