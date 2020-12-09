
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the J2534_SIM_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// J2534_SIM_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

//have to use exports file since vc mangles __stdcall

#include "J2534_v0404.h"

#ifdef J2534_SIM_EXPORTS
//#define J2534_SIM_API
#define J2534_SIM_API  __stdcall
//__declspec(dllexport)
#else
#define J2534_SIM_API  __stdcall
//__declspec(dllimport) 
#endif

#define JTYPE long J2534_SIM_API
#define APP_EXPORT __declspec(dllexport)
#define FUNC_EXPORT comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)

//#ifdef __cplusplus
//extern "C" {
//#endif

//extern APP_EXPORT JTYPE J2534_SIM_API PassThruOpen(void *, unsigned long *);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruClose(unsigned long DeviceID);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruConnect(unsigned long, unsigned long, unsigned long, unsigned long, unsigned long *);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruDisconnect(unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruWriteMsgs(unsigned long, PASSTHRU_MSG *, unsigned long *, unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruReadMsgs(unsigned long, PASSTHRU_MSG *, unsigned long *, unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruStartPeriodicMsg(unsigned long, PASSTHRU_MSG *, unsigned long *, unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruStopPeriodicMsg(unsigned long, unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruStartMsgFilter(unsigned long, unsigned long, PASSTHRU_MSG *, PASSTHRU_MSG *, PASSTHRU_MSG *, unsigned long *);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruStopMsgFilter(unsigned long, unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruSetProgrammingVoltage(unsigned long, unsigned long, unsigned long);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruReadVersion(unsigned long, char *, char *, char *);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruGetLastError(char *);
//extern APP_EXPORT JTYPE J2534_SIM_API PassThruIoctl(unsigned long, unsigned long, void *, void *);

//#ifdef __cplusplus
//} // extern "C"
//#endif
