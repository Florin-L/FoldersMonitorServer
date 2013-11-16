

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.00.0595 */
/* at Fri Nov 15 20:27:54 2013
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


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IFoldersMonitor,0xEAD6832B,0x72F1,0x417B,0x86,0x06,0x61,0x39,0x24,0x57,0x91,0xF7);


MIDL_DEFINE_GUID(IID, IID_IFoldersMonitorEvents,0x231A27E0,0x55B9,0x44A2,0x90,0x25,0xF8,0x2B,0x8E,0xD5,0xF4,0x0F);


MIDL_DEFINE_GUID(IID, LIBID_FoldersMonitorLib,0x17F8FB77,0x23E3,0x4747,0x9C,0xBA,0xBF,0xA6,0x1B,0x3C,0x6F,0x78);


MIDL_DEFINE_GUID(CLSID, CLSID_CoFoldersMonitor,0x8864BFA3,0x4D7D,0x4147,0xA5,0x4E,0x2C,0x23,0xC2,0x2D,0x76,0xD4);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



