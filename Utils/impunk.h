#pragma once

///////////////////////////////////////////////////////////////////////////////
#define NO_MODULE_LOCK(ClassName)\
	inline static void STDMETHODCALLTYPE ModuleAddRef() {}\
	inline static void STDMETHODCALLTYPE ModuleRelease() {}\

///////////////////////////////////////////////////////////////////////////////
extern void STDMETHODCALLTYPE ModuleAddRef();
extern void STDMETHODCALLTYPE ModuleRelease();

///////////////////////////////////////////////////////////////////////////////
struct AUTO_LONG
{
	AUTO_LONG() : value(0) {}

    inline LONG *operator & (void) { return &value; } 
    inline LONG operator ++(void) { return ++value; } 
    inline LONG operator --(void) { return --value; } 
    inline operator LONG (void) { return value; } 

	LONG value;
};

///////////////////////////////////////////////////////////////////////////////

#define IMPLEMENT_UNKNOWN(ClassName)\
	AUTO_LONG m_cRef;\
	\
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv) {\
		return InterfaceTableQueryInterface(this, GetInterfaceTable(), riid, ppv);\
	}\
	\
	ULONG __stdcall AddRef() {\
		if (0 == m_cRef)\
			ModuleAddRef();\
		return InterlockedIncrement(&m_cRef);\
	}\
	\
	ULONG __stdcall Release() {\
		ULONG res = InterlockedDecrement(&m_cRef);\
		if (0 == res)\
		{\
			delete this;\
			ModuleRelease();\
		}\
		return res;\
	}\

///////////////////////////////////////////////////////////////////////////////
#define IMPLEMENT_AGGREGATABLE_UNKNOWN(ClassName)\
	public:\
		AUTO_LONG m_cRef;\
	struct NonDelegatingUnknown : public IUnknown{\
		ClassName * This() {return (ClassName*)((BYTE*)this - FIELD_OFFSET(ClassName, m_innerUnknown)); }\
		STDMETHODIMP QueryInterface(REFIID riid, void **ppv)\
		{ return This()->InternalQueryInterface(riid, ppv); }\
		STDMETHODIMP_(ULONG) AddRef(void)\
		{  return This()->InternalAddRef(); }\
		STDMETHODIMP_(ULONG) Release(void)\
		{ return This()->InternalRelease(); }\
		NonDelegatingUnknown(void)\
		{ This()->m_pUnkOuter = this; }\
	};\
	NonDelegatingUnknown m_innerUnknown;\
    IUnknown *m_pUnkOuter;\
    STDMETHODIMP InternalQueryInterface(REFIID riid, void **ppv)\
    {\
        if (riid == IID_IUnknown)\
            return ((IUnknown*)(*ppv = static_cast<IUnknown*>(&m_innerUnknown)))->AddRef(), S_OK;\
        return InterfaceTableQueryInterface(this, GetInterfaceTable(), riid, ppv);\
    }\
    STDMETHODIMP_(ULONG) InternalAddRef(void)\
    {\
        extern void STDAPICALLTYPE ModuleAddRef(void);\
        if (m_cRef == 0)\
            ModuleAddRef();\
        return InterlockedIncrement(&m_cRef);\
    }\
    STDMETHODIMP_(ULONG) InternalRelease(void)\
    {\
        extern void STDAPICALLTYPE ModuleRelease(void);\
        ULONG res = InterlockedDecrement(&m_cRef);\
        if (res == 0) {\
            delete this;\
            ModuleRelease();\
        }\
        return res;\
    }\
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv)\
    { return m_pUnkOuter->QueryInterface(riid, ppv); }\
    STDMETHODIMP_(ULONG) AddRef(void)\
    {  return m_pUnkOuter->AddRef(); }\
    STDMETHODIMP_(ULONG) Release(void)\
    { return m_pUnkOuter->Release(); }\
    STDMETHODIMP ExternalQueryInterface(REFIID riid, void **ppv)\
    { return m_pUnkOuter->QueryInterface(riid, ppv); }\
    STDMETHODIMP_(ULONG) ExternalAddRef(void)\
    { return m_pUnkOuter->AddRef(); }\
    STDMETHODIMP_(ULONG) ExternalRelease(void)\
    { return m_pUnkOuter->Release(); }\
    IUnknown *GetControllingUnknown() { return m_pUnkOuter; }\
    IUnknown *GetNonDelegatingUnknown() { return &m_innerUnknown; }\
    HRESULT SetControllingUnknown(IUnknown * pUnkOuter) {\
        m_pUnkOuter = pUnkOuter ? pUnkOuter : &m_innerUnknown;\
        ++m_cRef;\
        HRESULT hr = CreateAggregates();\
        --m_cRef;\
        return hr;\
    }\

///////////////////////////////////////////////////////////////////////////////
#define IMPLEMENT_COMPOSITE_UNKNOWN(OuterClassName, InnerClassName, DataMemberName)\
	OuterClassName * This() {\
		return (OuterClassName *)((BYTE *)this - FIELD_OFFSET(OuterClassName, DataMemberName));\
	}\
	\
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppv) {\
		return This()->QueryInterface(riid, ppv);\
	}\
	ULONG __stdcall AddRef() {\
		return This()->AddRef();\
	}\
	ULONG __stdcall Release() {\
		return This()->Release();\
	}