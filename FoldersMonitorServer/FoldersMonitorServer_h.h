

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Thu Nov 14 22:25:47 2013
 */
/* Compiler settings for FoldersMonitorServer.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0595 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __FoldersMonitorServer_h_h__
#define __FoldersMonitorServer_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IFoldersMonitor_FWD_DEFINED__
#define __IFoldersMonitor_FWD_DEFINED__
typedef interface IFoldersMonitor IFoldersMonitor;

#endif 	/* __IFoldersMonitor_FWD_DEFINED__ */


#ifndef __IFoldersMonitorEvents_FWD_DEFINED__
#define __IFoldersMonitorEvents_FWD_DEFINED__
typedef interface IFoldersMonitorEvents IFoldersMonitorEvents;

#endif 	/* __IFoldersMonitorEvents_FWD_DEFINED__ */


#ifndef __CoFoldersMonitor_FWD_DEFINED__
#define __CoFoldersMonitor_FWD_DEFINED__

#ifdef __cplusplus
typedef class CoFoldersMonitor CoFoldersMonitor;
#else
typedef struct CoFoldersMonitor CoFoldersMonitor;
#endif /* __cplusplus */

#endif 	/* __CoFoldersMonitor_FWD_DEFINED__ */


/* header files for imported files */
#include "wtypes.h"
#include "Unknwn.h"
#include "objidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_FoldersMonitorServer_0000_0000 */
/* [local] */ 

typedef /* [version][uuid] */  DECLSPEC_UUID("CEDC3FC3-3451-43CF-8A0A-8A18FBF82028") 
enum NotificationFlags
    {
        NONE	= 0,
        CHANGE_NAME	= 2,
        CHANGE_ATTRIBUTES	= 4,
        CHANGE_LAST_WRITE	= 8
    } 	NotificationFlags;



extern RPC_IF_HANDLE __MIDL_itf_FoldersMonitorServer_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_FoldersMonitorServer_0000_0000_v0_0_s_ifspec;

#ifndef __IFoldersMonitor_INTERFACE_DEFINED__
#define __IFoldersMonitor_INTERFACE_DEFINED__

/* interface IFoldersMonitor */
/* [oleautomation][unique][uuid][object] */ 


EXTERN_C const IID IID_IFoldersMonitor;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("EAD6832B-72F1-417B-8606-6139245791F7")
    IFoldersMonitor : public IUnknown
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Start( 
            int nMaxTasksCount) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE Stop( void) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE CreateTask( 
            /* [in] */ BSTR folderName,
            /* [in] */ DWORD flags,
            /* [retval][out] */ BSTR *taskId) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE RemoveTask( 
            /* [in] */ BSTR taskId,
            /* [out] */ int *error) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE StartTask( 
            /* [in] */ BSTR taskId,
            /* [out] */ int *error) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE StopTask( 
            /* [in] */ BSTR taskId,
            /* [out] */ int *error) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFoldersMonitorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFoldersMonitor * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFoldersMonitor * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFoldersMonitor * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Start )( 
            IFoldersMonitor * This,
            int nMaxTasksCount);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *Stop )( 
            IFoldersMonitor * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *CreateTask )( 
            IFoldersMonitor * This,
            /* [in] */ BSTR folderName,
            /* [in] */ DWORD flags,
            /* [retval][out] */ BSTR *taskId);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *RemoveTask )( 
            IFoldersMonitor * This,
            /* [in] */ BSTR taskId,
            /* [out] */ int *error);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *StartTask )( 
            IFoldersMonitor * This,
            /* [in] */ BSTR taskId,
            /* [out] */ int *error);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *StopTask )( 
            IFoldersMonitor * This,
            /* [in] */ BSTR taskId,
            /* [out] */ int *error);
        
        END_INTERFACE
    } IFoldersMonitorVtbl;

    interface IFoldersMonitor
    {
        CONST_VTBL struct IFoldersMonitorVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFoldersMonitor_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFoldersMonitor_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFoldersMonitor_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFoldersMonitor_Start(This,nMaxTasksCount)	\
    ( (This)->lpVtbl -> Start(This,nMaxTasksCount) ) 

#define IFoldersMonitor_Stop(This)	\
    ( (This)->lpVtbl -> Stop(This) ) 

#define IFoldersMonitor_CreateTask(This,folderName,flags,taskId)	\
    ( (This)->lpVtbl -> CreateTask(This,folderName,flags,taskId) ) 

#define IFoldersMonitor_RemoveTask(This,taskId,error)	\
    ( (This)->lpVtbl -> RemoveTask(This,taskId,error) ) 

#define IFoldersMonitor_StartTask(This,taskId,error)	\
    ( (This)->lpVtbl -> StartTask(This,taskId,error) ) 

#define IFoldersMonitor_StopTask(This,taskId,error)	\
    ( (This)->lpVtbl -> StopTask(This,taskId,error) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFoldersMonitor_INTERFACE_DEFINED__ */


#ifndef __IFoldersMonitorEvents_INTERFACE_DEFINED__
#define __IFoldersMonitorEvents_INTERFACE_DEFINED__

/* interface IFoldersMonitorEvents */
/* [object][oleautomation][unique][uuid] */ 


EXTERN_C const IID IID_IFoldersMonitorEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("231A27E0-55B9-44A2-9025-F82B8ED5F40F")
    IFoldersMonitorEvents : public IUnknown
    {
    public:
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnNameChanged( 
            /* [in] */ int action,
            /* [in] */ BSTR fileName) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnAttributesChanged( 
            /* [in] */ int action,
            /* [in] */ BSTR fileName) = 0;
        
        virtual /* [id] */ HRESULT STDMETHODCALLTYPE OnLastWriteChanged( 
            /* [in] */ int action,
            /* [in] */ BSTR fileName) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IFoldersMonitorEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFoldersMonitorEvents * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFoldersMonitorEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFoldersMonitorEvents * This);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnNameChanged )( 
            IFoldersMonitorEvents * This,
            /* [in] */ int action,
            /* [in] */ BSTR fileName);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnAttributesChanged )( 
            IFoldersMonitorEvents * This,
            /* [in] */ int action,
            /* [in] */ BSTR fileName);
        
        /* [id] */ HRESULT ( STDMETHODCALLTYPE *OnLastWriteChanged )( 
            IFoldersMonitorEvents * This,
            /* [in] */ int action,
            /* [in] */ BSTR fileName);
        
        END_INTERFACE
    } IFoldersMonitorEventsVtbl;

    interface IFoldersMonitorEvents
    {
        CONST_VTBL struct IFoldersMonitorEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFoldersMonitorEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFoldersMonitorEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFoldersMonitorEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFoldersMonitorEvents_OnNameChanged(This,action,fileName)	\
    ( (This)->lpVtbl -> OnNameChanged(This,action,fileName) ) 

#define IFoldersMonitorEvents_OnAttributesChanged(This,action,fileName)	\
    ( (This)->lpVtbl -> OnAttributesChanged(This,action,fileName) ) 

#define IFoldersMonitorEvents_OnLastWriteChanged(This,action,fileName)	\
    ( (This)->lpVtbl -> OnLastWriteChanged(This,action,fileName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IFoldersMonitorEvents_INTERFACE_DEFINED__ */



#ifndef __FoldersMonitorLib_LIBRARY_DEFINED__
#define __FoldersMonitorLib_LIBRARY_DEFINED__

/* library FoldersMonitorLib */
/* [uuid] */ 


EXTERN_C const IID LIBID_FoldersMonitorLib;

EXTERN_C const CLSID CLSID_CoFoldersMonitor;

#ifdef __cplusplus

class DECLSPEC_UUID("8864BFA3-4D7D-4147-A54E-2C23C22D76D4")
CoFoldersMonitor;
#endif
#endif /* __FoldersMonitorLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


