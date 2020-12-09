#include "StdAfx.h"

#define F_HIJACK

// simple simulation when no hardware
//#define SIMULATION_MODE 1

#define FAIL	( 0 )
#define PASS	( 1 )

// where to write a log file too ( c:\ usually needs admin)
#define LOG_FILE_FILENAME "D:\\j2534_logger.txt"

#include "j2534_sim.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no color needed
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
//#define _MSC_VER XXX
#include "spdlog/sinks/msvc_sink.h"

#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <shlobj.h>
#include <sstream>

#define J2534_STATUS_NOERROR NO_ERROR
#define ulDataSize DataSize
#define J2534IOCTLID long
#define J2534_PROTOCOL long
#define J2534IOCTLPARAMID long
#define J2534ERROR long

// helper functions
//static const char *GetJ2534IOCTLIDText ( J2534IOCTLID enumIoctlID );
//static const char *GetJ2534_PROTOCOLText ( J2534_PROTOCOL protocol );
//static const char* GetJ2534IOCTLPARAMIDText ( J2534IOCTLPARAMID value );
//static void GetJ2534ErrorText ( J2534ERROR err );

//helllo have we heard of varags.

std::shared_ptr<spdlog::logger> kDefaultLogger;

static const std::string localFile(const std::string& fullPath)
{
    return std::strchr(fullPath.c_str(), '/') ? std::strrchr(fullPath.c_str(), '/') + 1 : fullPath;
}

namespace Constants {
    const char CONFIG_FILESUBDIR[]          = "log";
    const char APP_SETTINGSVARIANT_STR[]    = "SapaProject";
    const char LOG_FILE_NAME[]              = "j2534SimLog.txt";
}
#define LOG_FILE_SIZE       (64*1024*1024)
#define LOG_FILE_NUMBER     16

static std::string getLogFilePath()
{
    int csidl = CSIDL_LOCAL_APPDATA;
    char path[MAX_PATH];
    if (!SHGetSpecialFolderPathA(0, path, csidl, false)) {
        return Constants::LOG_FILE_NAME;
    }

    std::stringstream ss;
    ss << path
       << "/"
       << Constants::APP_SETTINGSVARIANT_STR
       << "/"
       << Constants::CONFIG_FILESUBDIR
       << "/"
       << Constants::LOG_FILE_NAME;
    return ss.str();
}

static void setupLogger()
{
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(getLogFilePath(), LOG_FILE_SIZE, LOG_FILE_NUMBER);
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S:%e] [%n] [%t] [%l] %v");
    //file_sink->set_level(spdlog::level::trace);

    //auto sinkK   = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    kDefaultLogger = std::make_shared<spdlog::logger>("cds", file_sink);

    kDefaultLogger->set_level(spdlog::level::trace);

}

static void LogMsg1 ( const char *str )
{
    kDefaultLogger->debug("{}", str);
}

static void LogMsg2 ( const char *str, const char *str1 )
{
    kDefaultLogger->debug("{} {}", str, str1);
}

static const char *GetJ2534IOCTLIDText ( J2534IOCTLID enumIoctlID )
{
    char buffer[16] = {0};

    switch ( enumIoctlID ) {

        case GET_CONFIG:
            return "GET_CONFIG";

        case SET_CONFIG:
            return "SET_CONFIG";

        case READ_VBATT:
            return "READ_VBATT";

        case FIVE_BAUD_INIT:
            return "FIVE_BAUD_INIT";

        case FAST_INIT:
            return "FAST_INIT";

#ifdef SET_PIN_USE

        case SET_PIN_USE:
            return "SET_PIN_USE";
#endif

        case CLEAR_TX_BUFFER:
            return "CLEAR_TX_BUFFER";

        case CLEAR_RX_BUFFER:
            return "CLEAR_RX_BUFFER";

        case CLEAR_PERIODIC_MSGS:
            return "CLEAR_PERIODIC_MSGS";

        case CLEAR_MSG_FILTERS:
            return "CLEAR_MSG_FILTERS";

        case CLEAR_FUNCT_MSG_LOOKUP_TABLE:
            return "CLEAR_FUNCT_MSG_LOOKUP_TABLE";

        case ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
            return "ADD_TO_FUNCT_MSG_LOOKUP_TABLE";

        case DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
            return "DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE";

        case READ_PROG_VOLTAGE:
            return "READ_PROG_VOLTAGE";

        default:
#ifdef F_HIJACK
            sprintf_s ( buffer, sizeof ( buffer ), "unknow(%ld)", enumIoctlID);
            return buffer;
#else
            return "unknow";
#endif

    }
}

static const char *GetJ2534_PROTOCOLText ( J2534_PROTOCOL protocol )
{
    char buffer[16] = {0};

    switch ( protocol ) {

        case J1850VPW:								// J1850VPW Protocol
            return "J1850VPW";

        case J1850PWM:								// J1850PWM Protocol
            return "J1850PWM";

        case ISO9141:								// ISO9141 Protocol
            return "ISO9141";

        case ISO14230:								// ISO14230 Protocol
            return "ISO14230";

        case CAN:									// CAN Protocol
            return "CAN";

        case ISO15765:
            return "ISO15765";

        case SCI_A_ENGINE:
            return "SCI_A_ENGINE";

        case SCI_A_TRANS:
            return "SCI_A_TRANS";

        case SCI_B_ENGINE:
            return "SCI_B_ENGINE";

        case SCI_B_TRANS:
            return "SCI_B_TRANS";

#ifdef ISO9141_FORD

        case ISO9141_FORD:							// ISO9141 FORD Protocol
            return "ISO9141_FORD";

        case UBP:									// UBP Protocol
            return "UBP";

        case DDL:									// DDL Protocol
            return "DDL";

        //**** NOTE ****
        //		 ALWAYS ADD NEW PROTOCOL
        //		 BEFORE J2534_PROTOCOL_NUM
        //		 SO THAT THIS VALUE AUTOMATICALLY
        //		 GETS UPDATED WHEN A NEW PROTOCOL
        //		 IS ADDED.
        case J2534_PROTOCOL_NUM:
#endif
#ifdef F_HIJACK
        case ISO15765_FD_PS:
            return "ISO15765_FD_PS";
        case CAN_FD_PS:
            return "CAN_FD_PS";
#endif

        default:
#ifdef F_HIJACK
                sprintf_s ( buffer, sizeof ( buffer ), "unknown(%ld)", protocol);
                return buffer;
#else
                return "unknown";
#endif

    }
}

static const char* GetJ2534IOCTLPARAMIDText ( J2534IOCTLPARAMID value )
{
    char buffer[16] = {0};

    switch ( value  ) {
        case DATA_RATE:
            return "DATA_RATE";

        case LOOPBACK:
            return "LOOPBACK";

        case NODE_ADDRESS:
            return "NODE_ADDRESS";

        case NETWORK_LINE:
            return "NETWORK_LINE";

        case P1_MIN:
            return "P1_MIN";

        case P1_MAX:
            return "P1_MAX";

        case P2_MIN:
            return "P2_MIN";

        case P2_MAX:
            return "P2_MAX";

        case P3_MIN:
            return "P3_MIN";

        case P3_MAX:
            return "P3_MAX";

        case P4_MIN:
            return "P4_MIN";

        case P4_MAX:
            return "P4_MAX";

        case W1:
            return "W1";

        case W2:
            return "W2";

        case W3:
            return "W3";

        case W4:
            return "W4";

        case W5:
            return "W5";

        case TIDLE:
            return "TIDLE";

        case TINIL:
            return "TINIL";

        case TWUP:
            return "TWUP";

        case PARITY:
            return "PARITY";

        case BIT_SAMPLE_POINT:
            return "BIT_SAMPLE_POINT";

        case SYNC_JUMP_WIDTH:
            return "SYNC_JUMP_WIDTH";

        case W0:
            return "W0";

        case T1_MAX:
            return "T1_MAX";

        case T2_MAX:
            return "T2_MAX";

        case T4_MAX:
            return "T4_MAX";

        case T5_MAX:
            return "T5_MAX";

        case ISO15765_BS:
            return "ISO15765_BS";

        case ISO15765_STMIN:
            return "ISO15765_STMIN";

        case DATA_BITS:
            return "DATA_BITS";

        default:
#ifdef F_HIJACK
            sprintf_s ( buffer, sizeof ( buffer ), "unknown(%ld)", value);
            return buffer;
#else
            return "GetJ2534IOCTLPARAMIDText unknown";
#endif

    }
}

/*
 * GetJ2534ErrorText - Convert error code to text
 *	Input: J2534ERROR err
 */

static void GetJ2534ErrorText ( J2534ERROR err )
{

    switch ( err ) {
        case STATUS_NOERROR						:         //    0x00
            LogMsg1 ( "STATUS_NOERROR\n" );
            break;

        case ERR_NOT_SUPPORTED					:         //    0x01
            LogMsg1 ( "ERR_NOT_SUPPORTED\n" );
            break;

        case ERR_INVALID_CHANNEL_ID					:         //    0x02
            LogMsg1 ( "ERR_INVALID_CHANNEL_ID\n" );
            break;

        case ERR_INVALID_PROTOCOL_ID					:         //    0x03
            LogMsg1 ( "ERR_INVALID_PROTOCOL_ID\n" );
            break;

        case ERR_NULL_PARAMETER					:         //    0x04
            LogMsg1 ( "ERR_NULL_PARAMETER\n" );
            break;

        case ERR_INVALID_IOCTL_VALUE					:         //    0x05
            LogMsg1 ( "ERR_INVALID_IOCTL_VALUE\n" );
            break;

        case ERR_INVALID_FLAGS					:         //    0x06
            LogMsg1 ( "ERR_INVALID_FLAGS\n" );
            break;

        case ERR_FAILED						:         //    0x07
            LogMsg1 ( "ERR_FAILED\n" );
            break;

        case ERR_DEVICE_NOT_CONNECTED				:         //    0x08
            LogMsg1 ( "ERR_DEVICE_NOT_CONNECTED\n" );
            break;

        case ERR_TIMEOUT						:         //    0x09
            LogMsg1 ( "ERR_TIMEOUT\n" );
            break;

        case ERR_INVALID_MSG						:         //    0x0A
            LogMsg1 ( "ERR_INVALID_MSG\n" );
            break;

        case ERR_INVALID_TIME_INTERVAL				:         //    0x0B
            LogMsg1 ( "ERR_INVALID_TIME_INTERVAL\n" );
            break;

        case ERR_EXCEEDED_LIMIT					:         //    0x0C
            LogMsg1 ( "ERR_EXCEEDED_LIMIT\n" );
            break;

        case ERR_INVALID_MSG_ID					:         //    0x0D
            LogMsg1 ( "ERR_INVALID_MSG_ID\n" );
            break;

        case ERR_DEVICE_IN_USE					:         //    0x0E
            LogMsg1 ( "ERR_DEVICE_IN_USE\n" );
            break;

        case ERR_INVALID_IOCTL_ID					:         //    0x0F
            LogMsg1 ( "ERR_INVALID_IOCTL_ID\n" );
            break;

        case ERR_BUFFER_EMPTY					:         //    0x10
            LogMsg1 ( "ERR_BUFFER_EMPTY\n" );
            break;

        case ERR_BUFFER_FULL						:         //    0x11
            LogMsg1 ( "ERR_BUFFER_FULL\n" );
            break;

        case ERR_BUFFER_OVERFLOW					:         //    0x12
            LogMsg1 ( "ERR_BUFFER_OVERFLOW\n" );
            break;

        case ERR_PIN_INVALID						:         //    0x13
            LogMsg1 ( "ERR_PIN_INVALID\n" );
            break;

        case ERR_CHANNEL_IN_USE					:         //    0x14
            LogMsg1 ( "ERR_CHANNEL_IN_USE\n" );
            break;

        case ERR_MSG_PROTOCOL_ID					:         //    0x15
            LogMsg1 ( "ERR_MSG_PROTOCOL_ID\n" );
            break;

        case ERR_INVALID_FILTER_ID					:         //    0x16
            LogMsg1 ( "ERR_INVALID_FILTER_ID\n" );
            break;

        case ERR_NO_FLOW_CONTROL					:         //    0x17
            LogMsg1 ( "ERR_NO_FLOW_CONTROL - No ISO15765 flow control filter is set, or no filter matches the header of an outgoing message\n" );
            break;

        case ERR_NOT_UNIQUE						:         //    0x18
            LogMsg1 ( "ERR_NOT_UNIQUE\n" );
            break;

        case ERR_INVALID_BAUDRATE					:         //    0x19
            LogMsg1 ( "ERR_INVALID_BAUDRATE\n" );
            break;

        case ERR_INVALID_DEVICE_ID					:         //    0x1A
            LogMsg1 ( "ERR_INVALID_DEVICE_ID\n" );
            break;
    }
}

BOOL APIENTRY DllMain ( HANDLE hModule,
                        DWORD  ul_reason_for_call,
                        LPVOID lpReserved
                      )
{

    switch ( ul_reason_for_call ) {
        case DLL_PROCESS_ATTACH:
            setupLogger();
            LogMsg1 ( "call DLL_PROCESS_ATTACH\n" );
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

void PrintBuffer ( int length, unsigned char *data )
{
    char buffer[1024];
    char buffer2[1024];
    int i;

    if ( data == NULL ) { return; }

    if ( length == 0 ) { return; }


    memset ( buffer, 0, sizeof ( buffer ) );
    memset ( buffer2, 0, sizeof ( buffer2 ) );

    for ( i = 0; i < length; i++ ) {

        sprintf_s ( buffer2, sizeof ( buffer2 ), "0x%02x ", *data++ );

        strcat_s ( buffer, sizeof ( buffer ), buffer2 );
    }

    LogMsg1 ( buffer );
}

// Declaration of Exported functions.

/*
	long PassThruConnect(unsigned long ProtocolID, unsigned long Flags, unsigned long *pChannelID);

	Description
		The PassThruConnect function is used to establish a logical communication channel between the User Application
		and the vehicle network (via the PassThru device) using the specified network layer protocol and selected
		protocol options.

	Parameters
		ProtocolID
			The protocol identifier selects the network layer protocol that will be used for the communications channel.
		Flags
			Protocol specific options that are defined by bit fields. This parameter is usually set to zero.
		pChannelID
			Pointer to an unsigned long (4 bytes) that receives the handle to the open communications channel. The returned
			handle is used as an argument to other PassThru functions which require a communications channel reference.
		See Also
			PassThruDisconnect

	Example
		unsigned long status;
		unsigned long Flags;
		unsigned long ChannelID;	/* Logical channel identifier returned by PassThruConnect
		char errstr[256];

		*
		** Select Extended CAN Data Frame and Network Address Extension.
		*
		Flags = (CAN_29BIT_ID | ISO15765_ADDR_TYPE);

		status = PassThruConnect(CAN, Flags, &ChannelID);
		if (status != STATUS_NOERROR)
		{
			*
			** PassThruConnect failed! Get descriptive error string.
			*
			PassThruGetLastError(&errstr[0]);

			*
			** Display Error dialog box and/or write error description to Log file.
			*
		}


*/

typedef long ( CALLBACK * PTOPEN ) ( void *, unsigned long * );
typedef long ( CALLBACK * PTCLOSE ) ( unsigned long );

//#define J2534ERROR long


typedef long ( CALLBACK * PTCONNECT ) ( unsigned long, unsigned long, unsigned long, unsigned long, unsigned long * );
typedef long ( CALLBACK * PTDISCONNECT ) ( unsigned long );
typedef long ( CALLBACK * PTREADMSGS ) ( unsigned long, void *, unsigned long *, unsigned long );
typedef long ( CALLBACK * PTWRITEMSGS ) ( unsigned long, void *, unsigned long *, unsigned long );
typedef long ( CALLBACK * PTSTARTPERIODICMSG ) ( unsigned long, void *, unsigned long *, unsigned long );
typedef long ( CALLBACK * PTSTOPPERIODICMSG ) ( unsigned long, unsigned long );
typedef long ( CALLBACK * PTSTARTMSGFILTER ) ( unsigned long, unsigned long, void *, void *, void *, unsigned long * );
typedef long ( CALLBACK * PTSTOPMSGFILTER ) ( unsigned long, unsigned long );
typedef long ( CALLBACK * PTSETPROGRAMMINGVOLTAGE ) ( unsigned long, unsigned long, unsigned long );
typedef long ( CALLBACK * PTREADVERSION ) ( unsigned long, char *, char *, char * );
typedef long ( CALLBACK * PTGETLASTERROR ) ( char * );
typedef long ( CALLBACK * PTIOCTL ) ( unsigned long, unsigned long, void *, void * );
// Drew Tech specific function calls
typedef long ( CALLBACK * PTLOADFIRMWARE ) ( void );
typedef long ( CALLBACK * PTRECOVERFIRMWARE ) ( void );
typedef long ( CALLBACK * PTREADIPSETUP ) ( unsigned long DeviceID, char *host_name, char *ip_addr, char *subnet_mask,
        char *gateway, char *dhcp_addr );
typedef long ( CALLBACK * PTWRITEIPSETUP ) ( unsigned long DeviceID, char *host_name, char *ip_addr, char *subnet_mask,
        char *gateway, char *dhcp_addr );
typedef long ( CALLBACK * PTREADPCSETUP ) ( char *host_name, char *ip_addr );
typedef long ( CALLBACK * PTGETPOINTER ) ( long vb_pointer );
typedef long ( CALLBACK * PTGETNEXTCARDAQ ) ( char **name, unsigned long * version, char **addr );

extern unsigned long gUserNumEcusReprgm;
extern unsigned long gRespTimeOutofRange;
extern unsigned long gRespTimeTooSoon;
extern unsigned long gRespTimeTooLate;
extern unsigned long gDetermineProtocol;

typedef struct globData {
    PTOPEN pPassThruOpen;
    PTCLOSE pPassThruClose;
    PTCONNECT pPassThruConnect;
    PTDISCONNECT pPassThruDisconnect;
    PTREADMSGS pPassThruReadMsgs;
    PTWRITEMSGS pPassThruWriteMsgs;
    PTSTARTPERIODICMSG pPassThruStartPeriodicMsg;
    PTSTOPPERIODICMSG pPassThruStopPeriodicMsg;
    PTSTARTMSGFILTER pPassThruStartMsgFilter;
    PTSTOPMSGFILTER pPassThruStopMsgFilter;
    PTSETPROGRAMMINGVOLTAGE pPassThruSetProgrammingVoltage;
    PTREADVERSION pPassThruReadVersion;
    PTGETLASTERROR pPassThruGetLastError;
    PTIOCTL pPassThruIoctl;
    PTLOADFIRMWARE pPassThruLoadFirmware;
    PTRECOVERFIRMWARE pPassThruRecoverFirmware;
    PTREADIPSETUP pPassThruReadIPSetup;
    PTWRITEIPSETUP pPassThruWriteIPSetup;
    PTREADPCSETUP pPassThruReadPCSetup;
    PTGETPOINTER pPassThruGetPointer;
    PTGETNEXTCARDAQ pPassThruGetNextCarDAQ;
} globData_t;

typedef struct stPassThrough {

    unsigned long ulChannel;
    globData data;

} stPassThrough_t;


static int Load_J2534DLL (  const char *szLibrary, globData * data );

// eek, very bad for a dll. fix pls!
static stPassThrough_t *pGlobalPtr = NULL;

// this loads the original DLL, set the paths in here to the DLL you want to log.
//
int Load_J2534DLL ( void )
{
    stPassThrough *pPtr;
    //load the real dll (change to registry key, or popup dialog

    LogMsg1 ( "Load_J2534DLL\n" );

    // SCANDAQ
    //allocate some memory
    pPtr = pGlobalPtr = ( stPassThrough* ) malloc ( sizeof ( stPassThrough ) );

    if ( pPtr == NULL ) {
        return ERR_FAILED;
    }

#ifdef F_HIJACK
    memset(pPtr, 0, sizeof(stPassThrough));
#endif

#ifdef SIMULATION_MODE
    return 1;
#endif

    Load_J2534DLL ( "C:\\Program Files (x86)\\GM MDI Software\\Products\\MDI 2\\Dynamic Link Libraries\\BVTX4J32.dll", &pPtr->data );

    return 1;
}

static inline stPassThrough *GetGlobalstPassThrough(unsigned long ChannelID)
{
    stPassThrough *pPtr = pGlobalPtr;
//    stPassThrough *pPtr = ( stPassThrough* ) ChannelID ;
    
    return pPtr;
}

JTYPE  PassThruOpen ( void* pName, unsigned long * pDeviceID )
{

    #pragma FUNC_EXPORT

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = STATUS_NOERROR ;
    {
        char buffer[1024];

        sprintf_s ( buffer, sizeof ( buffer ), "PassThruOpen(%s,0x%lx)\n", pName, pDeviceID );
        LogMsg1 ( buffer );
    }


    if ( pPtr == NULL )
    { Load_J2534DLL(); }

	{
		char buffer[128];
	
		sprintf_s(buffer, sizeof(buffer), "dll has been loaded, %lx, %lx\n", pGlobalPtr, pGlobalPtr->data.pPassThruOpen);
		LogMsg1 (buffer);
	}
	

    if ( pGlobalPtr && pGlobalPtr->data.pPassThruOpen ) {
		pPtr = pGlobalPtr;
        ret = ( J2534ERROR ) pPtr->data.pPassThruOpen ( pName, pDeviceID );

    	{
			char buffer[128];

			sprintf_s(buffer, sizeof(buffer), "DeviceID assigned = %ul, ret = %d\n", *pDeviceID, ret);
			LogMsg1 (buffer);
        }

        GetJ2534ErrorText ( ret );
    }
#ifdef F_HIJACK
    else {
        if ( pDeviceID ) {
            *pDeviceID = ( unsigned long ) 0;
        }
    }
#endif

    return ret;
}


JTYPE  J2534_SIM_API  PassThruClose ( unsigned long DeviceID )
{
    #pragma FUNC_EXPORT

    J2534ERROR ret = J2534_STATUS_NOERROR;

    LogMsg1 ( "PassThruClose\n" );

    if ( pGlobalPtr && pGlobalPtr->data.pPassThruClose ) {
        ret = ( J2534ERROR ) pGlobalPtr->data.pPassThruClose ( DeviceID );

        GetJ2534ErrorText ( ret );
    }

    return ret;

}

JTYPE PassThruConnect ( unsigned long DeviceID, unsigned long ProtocolID, unsigned long Flags, unsigned long Baudrate, unsigned long * pChannelID )
{
    #pragma FUNC_EXPORT

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = pGlobalPtr ;

    if ( pPtr == NULL ) {
        Load_J2534DLL();
        pPtr = pGlobalPtr ;
    }

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruConnect 0404: DeviceID = %ul, ProtocolID = %d, Flags =%d, BaudRate = %d\n", DeviceID, ProtocolID, Flags, Baudrate );
    LogMsg1 ( buffer );

    LogMsg2 ( "protocol = %s\n", GetJ2534_PROTOCOLText ( ( J2534_PROTOCOL ) ProtocolID ) );

    if ( pPtr && pPtr->data.pPassThruConnect )	{
        LogMsg1 ( "pPtr->data.pPassThruConnect != NULL\n" );
        ret = ( J2534ERROR ) pPtr->data.pPassThruConnect ( DeviceID, ProtocolID, Flags, Baudrate, &pPtr->ulChannel );
    }

    else {

        pPtr->ulChannel = 0;
    }

#if !defined ( SIMULATION_MODE )
    if ( pChannelID ) {
        *pChannelID = ( unsigned long ) pPtr->ulChannel;
    }
#else
    if ( pChannelID ) {
        *pChannelID = ( unsigned long ) pGlobalPtr;
    }
#endif

    sprintf_s(buffer, sizeof(buffer), "ChannelID assigned = %d\n", *pChannelID);
    LogMsg1 (buffer);

    return ret;
}

JTYPE PassThruDisconnect ( unsigned long ChannelID )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    //LogMsg1("PassThruDisconnect\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr )
    { ChannelID = pPtr->ulChannel; }

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruDisconnect : ChannelID = %d\n", ChannelID );
    LogMsg1 ( buffer );

    pPtr = pGlobalPtr;


    if ( pPtr && pPtr->data.pPassThruDisconnect ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruDisconnect ( ChannelID );
    }

    return ret;

}

JTYPE PassThruReadMsgs ( unsigned long ChannelID, PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    //LogMsg1("PassThruReadMsgs\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruReadMsgs : ChannelID = %lu, N = %lu, T=%lu\n", ChannelID, *pNumMsgs, Timeout );
    LogMsg1 ( buffer );

    pPtr = pGlobalPtr;

    if ( pPtr && pPtr->data.pPassThruReadMsgs ) {

        //	Sleep(20);
        ret = ( J2534ERROR ) pPtr->data.pPassThruReadMsgs ( ChannelID, pMsg, pNumMsgs, Timeout );
		sprintf_s ( buffer, sizeof ( buffer ), "PassThruReadMsgs : ret = %d, N = %lu\n", ret, *pNumMsgs );
    	LogMsg1 ( buffer );
    }

    if ( ret == J2534_STATUS_NOERROR && pMsg->ulDataSize ) {

        sprintf_s ( buffer, sizeof ( buffer ), "pMSg(%d,0x%lx,%d,%d)\n", ChannelID, pMsg, *pNumMsgs, Timeout );
        LogMsg1 ( buffer );
        LogMsg2 ( "\tprotocol    = %s\n", GetJ2534_PROTOCOLText ( ( J2534_PROTOCOL ) pMsg->ProtocolID ) );

        sprintf_s ( buffer, sizeof ( buffer ), "\trx status   = %lu\n", pMsg->RxStatus );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttx flags    = %lu\n", pMsg->TxFlags );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttime stamp  = %lu\n", pMsg->Timestamp );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\textra data  = %lu\n", pMsg->ExtraDataIndex );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata size   = %lu\n", pMsg->DataSize );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata        = 0x%lx = { ", pMsg->Data );
        LogMsg1 ( buffer );
        PrintBuffer ( pMsg->DataSize, &pMsg->Data[0] );
        LogMsg1 ( "}\n" );

        sprintf_s ( buffer, sizeof ( buffer ), "\textradata        = 0x%lx = { ", pMsg->Data );
        LogMsg1 ( buffer );
        PrintBuffer ( pMsg->DataSize, &pMsg->Data[pMsg->ExtraDataIndex] );
        LogMsg1 ( "}\n" );

    } else
        if ( ret != ERR_BUFFER_EMPTY ) {
            GetJ2534ErrorText ( ret );

        } else {
//		LogMsg1("");


        }

    return ret;

}

JTYPE PassThruWriteMsgs ( unsigned long ChannelID, PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout )
{
    #pragma FUNC_EXPORT

    J2534ERROR ret = J2534_STATUS_NOERROR;

#ifdef F_HIJACK
    LogMsg1("PassThruWriteMsgs\n");
#endif

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruWriteMsgs : ChannelID = %d\n", ChannelID );
    LogMsg1 ( buffer );

    pPtr = pGlobalPtr;


    sprintf_s ( buffer, sizeof ( buffer ), "PassThruWriteMsgs(%d,0x%lx,%d,%d)\n", ChannelID, pMsg, *pNumMsgs, Timeout );
    LogMsg1 ( buffer );

    LogMsg2 ( "\tprotocol    = %s\n", GetJ2534_PROTOCOLText ( ( J2534_PROTOCOL ) pMsg->ProtocolID ) );


// not used for write

//	_RPT1(_CRT_WARN,"\tex data ind = %lu\n",pMsg->ExtraDataIndex);
//	_RPT1(_CRT_WARN,"\ttimestamp   = %lu\n",pMsg->TimeStamp);


    sprintf_s ( buffer, sizeof ( buffer ), "\trx status   = %lu\n", pMsg->RxStatus );
    LogMsg1 ( buffer );
    sprintf_s ( buffer, sizeof ( buffer ), "\ttx flags    = %lu\n", pMsg->TxFlags );
    LogMsg1 ( buffer );
    sprintf_s ( buffer, sizeof ( buffer ), "\ttime stamp  = %lu\n", pMsg->Timestamp );
    LogMsg1 ( buffer );
    sprintf_s ( buffer, sizeof ( buffer ), "\textra data  = %lu\n", pMsg->ExtraDataIndex );
    LogMsg1 ( buffer );

    sprintf_s ( buffer, sizeof ( buffer ), "\tdata size   = %lu\n", pMsg->DataSize );
    LogMsg1 ( buffer );
    sprintf_s ( buffer, sizeof ( buffer ), "\tdata        = 0x%lx = { ", pMsg->Data );
    LogMsg1 ( buffer );

    if ( pMsg->Data != NULL ) {


        PrintBuffer ( pMsg->DataSize, &pMsg->Data[0] );

        PrintBuffer ( pMsg->DataSize, &pMsg->Data[pMsg->ExtraDataIndex] );

    }

    LogMsg1 ( "}\n" );

    if ( pPtr && pPtr->data.pPassThruWriteMsgs ) {

//		Sleep( 10 );

        ret = ( J2534ERROR ) pPtr->data.pPassThruWriteMsgs ( ChannelID, pMsg, pNumMsgs, Timeout );

		sprintf_s ( buffer, sizeof ( buffer ), "PassThruWriteMsgs : ret = %d, N = %lu\n", ret, *pNumMsgs );
		LogMsg1 ( buffer );

    }

    return ret;
}

JTYPE PassThruStartPeriodicMsg ( unsigned long ChannelID, PASSTHRU_MSG * pMsg,
                                 unsigned long * pMsgID, unsigned long TimeInterval )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    LogMsg1("PassThruStartPeriodicMsg\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruStartPeriodicMsg : ChannelID = %d\n", ChannelID );
    LogMsg1 ( buffer );

    pPtr = pGlobalPtr;

    LogMsg1 ( "PassThruStartPeriodicMsg" );

    if ( pPtr && pPtr->data.pPassThruStartPeriodicMsg ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruStartPeriodicMsg ( ChannelID, pMsg, pMsgID, TimeInterval );
    }

    GetJ2534ErrorText ( ret );

    return ret;
}

JTYPE PassThruStopPeriodicMsg ( unsigned long ChannelID, unsigned long MsgID )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    LogMsg1("PassThruStopPeriodicMsg\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;
    ChannelID = pPtr->ulChannel;

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruStopPeriodicMsg : ChannelID = %d\n", ChannelID );
    LogMsg1 ( buffer );

    pPtr = pGlobalPtr;

    LogMsg1 ( "PassThruStartPeriodicMsg" );

    if ( pPtr && pPtr->data.pPassThruStopPeriodicMsg ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruStopPeriodicMsg ( ChannelID, MsgID );
    }

    GetJ2534ErrorText ( ret );

    return ret;
}

JTYPE PassThruStartMsgFilter ( unsigned long ChannelID,
                               unsigned long FilterType, PASSTHRU_MSG * pMsg, PASSTHRU_MSG * pPatternMsg,
                               PASSTHRU_MSG * pFlowControlMsg, unsigned long * pMsgID )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    LogMsg1("PassThruStartMsgFilter\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID );

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruStartMsgFilter : ChannelID = %d\n", ChannelID );
    LogMsg1 ( buffer );

    sprintf_s ( buffer, sizeof ( buffer ), "FilterType = %d\n", FilterType );
    LogMsg1 ( buffer );

    if ( pMsg ) {
        LogMsg1 ( "MaskMsg\n" );
        sprintf_s ( buffer, sizeof ( buffer ), "ProtocolID = %d\n", pMsg->ProtocolID );
        LogMsg1 ( buffer );

        LogMsg2 ( "\tprotocol    = %s\n", GetJ2534_PROTOCOLText ( ( J2534_PROTOCOL ) pMsg->ProtocolID ) );

        sprintf_s ( buffer, sizeof ( buffer ), "\trx status   = %lu\n", pMsg->RxStatus );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttx flags    = %lu\n", pMsg->TxFlags );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttime stamp  = %lu\n", pMsg->Timestamp );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\textra data  = %lu\n", pMsg->ExtraDataIndex );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata size   = %lu\n", pMsg->DataSize );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata        = 0x%lx = { ", pMsg->Data );
        LogMsg1 ( buffer );
        PrintBuffer ( pMsg->DataSize, &pMsg->Data[0] );
        LogMsg1 ( "}\n" );


        sprintf_s ( buffer, sizeof ( buffer ), "\textradata   = 0x%lx = { ", pMsg->Data );
        LogMsg1 ( buffer );
#ifdef F_HIJACK
        // GDS request with a wrong value which is large, so we prevent from the problem by temporary.
        if (pMsg->ExtraDataIndex < 20)
#endif        
        PrintBuffer ( pMsg->ExtraDataIndex, &pMsg->Data[pMsg->ExtraDataIndex] );
        LogMsg1 ( "}\n" );
    }

    if ( pPatternMsg ) {
        LogMsg1 ( "pPatternMsg\n" );
        LogMsg2 ( "\tprotocol    = %s\n", GetJ2534_PROTOCOLText ( ( J2534_PROTOCOL ) pPatternMsg->ProtocolID ) );

        sprintf_s ( buffer, sizeof ( buffer ), "\trx status   = %lu\n", pPatternMsg->RxStatus );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttx flags    = %lu\n", pPatternMsg->TxFlags );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttime stamp  = %lu\n", pPatternMsg->Timestamp );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\textra data  = %lu\n", pPatternMsg->ExtraDataIndex );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata size   = %lu\n", pPatternMsg->DataSize );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata        = 0x%lx = { ", pPatternMsg->Data );
        LogMsg1 ( buffer );
        PrintBuffer ( pPatternMsg->DataSize, &pPatternMsg->Data[0] );
        LogMsg1 ( "}\n" );


        sprintf_s ( buffer, sizeof ( buffer ), "\textradata   = 0x%lx = { ", pPatternMsg->Data );
        LogMsg1 ( buffer );
#ifdef F_HIJACK
        // GDS request with a wrong value which is large, so we prevent from the problem by temporary.
        if (pPatternMsg->ExtraDataIndex < 20)
#endif
        PrintBuffer ( pPatternMsg->ExtraDataIndex, &pPatternMsg->Data[pPatternMsg->ExtraDataIndex] );
        LogMsg1 ( "}\n" );
    }

    if ( pFlowControlMsg ) {
        LogMsg1 ( "pFlowControlMsg\n" );
        LogMsg2 ( "\tprotocol    = %s\n", GetJ2534_PROTOCOLText ( ( J2534_PROTOCOL ) pFlowControlMsg->ProtocolID ) );

        sprintf_s ( buffer, sizeof ( buffer ), "\trx status   = %lu\n", pFlowControlMsg->RxStatus );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttx flags    = %lu\n", pFlowControlMsg->TxFlags );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\ttime stamp  = %lu\n", pFlowControlMsg->Timestamp );
        LogMsg1 ( buffer );
        sprintf_s ( buffer, sizeof ( buffer ), "\textra data  = %lu\n", pFlowControlMsg->ExtraDataIndex );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata size   = %lu\n", pFlowControlMsg->DataSize );
        LogMsg1 ( buffer );

        sprintf_s ( buffer, sizeof ( buffer ), "\tdata        = 0x%lx = { ", pFlowControlMsg->Data );
        LogMsg1 ( buffer );
        PrintBuffer ( pFlowControlMsg->DataSize, &pFlowControlMsg->Data[0] );
        LogMsg1 ( "}\n" );


        sprintf_s ( buffer, sizeof ( buffer ), "\textradata   = 0x%lx = { ", pFlowControlMsg->Data );
        LogMsg1 ( buffer );
#ifdef F_HIJACK
        // GDS request with a wrong value which is large, so we prevent from the problem by temporary.
        if (pFlowControlMsg->ExtraDataIndex < 20)
#endif
        PrintBuffer ( pFlowControlMsg->ExtraDataIndex, &pFlowControlMsg->Data[pFlowControlMsg->ExtraDataIndex] );
        LogMsg1 ( "}\n" );
    }

#if !defined ( SIMULATION_MODE )

    if ( pPtr && pPtr->data.pPassThruStartMsgFilter ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruStartMsgFilter ( pPtr->ulChannel, FilterType,
                pMsg, pPatternMsg, pFlowControlMsg, pMsgID );
    }

#else
    ret  = 0;
#endif

    GetJ2534ErrorText ( ret );

    return ret;
}

JTYPE PassThruStopMsgFilter ( unsigned long ChannelID, unsigned long ulFilterID )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    LogMsg1("PassThruStopMsgFilter\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruStopMsgFilter : ChannelID = %d, MsgID = %d\n", ChannelID, ulFilterID );
    LogMsg1 ( buffer );

#if !defined ( SIMULATION_MODE )

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID );

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }


    if ( pPtr ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruStopMsgFilter ( pPtr->ulChannel, ulFilterID );
    }

    GetJ2534ErrorText ( ret );

#endif



    return ret;
}

JTYPE PassThruSetProgrammingVoltage ( unsigned long DeviceID, unsigned long Pin, unsigned long Voltage )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    LogMsg1("PassThruSetProgrammingVoltage\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;
    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruSetProgrammingVoltage : DeviceID = %d, Pin = %d, Voltage = %d\n", DeviceID, Pin, Voltage );
    LogMsg1 ( buffer );


#if !defined ( SIMULATION_MODE )

    stPassThrough *pPtr = pGlobalPtr;

    if ( pPtr ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruSetProgrammingVoltage ( DeviceID, Pin, Voltage );
    }

    GetJ2534ErrorText ( ret );

#endif




    return ret;
}

JTYPE PassThruReadVersion ( unsigned long DeviceID, char *pchFirmwareVersion, char *pchDllVersion, char *pchApiVersion )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    LogMsg1("PassThruReadVersion\n");
#endif

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = pGlobalPtr ;

    char buffer[1024];
    sprintf_s ( buffer, sizeof ( buffer ), "PassThruReadVersion : DeviceID = %ul\n", DeviceID );
    LogMsg1 ( buffer );

#if !defined ( SIMULATION_MODE )

    if ( pPtr && pPtr->data.pPassThruReadVersion ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruReadVersion ( DeviceID, pchFirmwareVersion, pchDllVersion, pchApiVersion );

        LogMsg1 ( pchFirmwareVersion );
        LogMsg1 ( pchDllVersion );
        LogMsg1 ( pchApiVersion );
		LogMsg1 ("\n");

    }

    else {

        if ( pchFirmwareVersion ) { strcpy_s ( pchFirmwareVersion, 4, "1.00" ); }

        if ( pchDllVersion ) { strcpy_s ( pchDllVersion, 4, "1.00" ); }

        if ( pchApiVersion ) { strcpy_s ( pchApiVersion, 4, "1.00" ); }
    }

#else

    strcpy ( pchFirmwareVersion, "1.00" );
    strcpy ( pchDllVersion, "1.00" );
    strcpy ( pchApiVersion, "1.00" );

#endif


    return ret;;

}


JTYPE PassThruGetLastError ( char *pErrorDescription )
{
    #pragma FUNC_EXPORT

    J2534ERROR ret = J2534_STATUS_NOERROR;

    return ret;
}

static void SetIoCtlOutputValue(unsigned long enumIoctlID, void *pInput, void *pOutput)
{
        char buffer[16] = {0};
    
        switch ( enumIoctlID ) {
    
            case GET_CONFIG:
                break;
    
            case SET_CONFIG:
                break;
    
            case READ_VBATT:
                *(unsigned long*)pOutput = 12;
                break;
    
            case FIVE_BAUD_INIT:
                break;
    
            case FAST_INIT:
                break;
    
#ifdef SET_PIN_USE
            case SET_PIN_USE:
                break;
#endif
    
            case CLEAR_TX_BUFFER:
                break;
    
            case CLEAR_RX_BUFFER:
                break;
    
            case CLEAR_PERIODIC_MSGS:
                break;
    
            case CLEAR_MSG_FILTERS:
                break;
    
            case CLEAR_FUNCT_MSG_LOOKUP_TABLE:
                break;
    
            case ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
                break;
    
            case DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
                break;
    
            case READ_PROG_VOLTAGE:
                break;
    
            default:
                //sprintf_s ( buffer, sizeof ( buffer ), "unknow(%ld)", enumIoctlID);
                break;
       }

    return;
}

JTYPE PassThruIoctl ( unsigned long ChannelID, unsigned long IoctlID,
                      void *pInput, void *pOutput )
{
    #pragma FUNC_EXPORT

#ifdef F_HIJACK
    //LogMsg1("PassThruIoctl\n");
#endif

    int ret;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    //if ( pPtr ) {
    //    ChannelID = pPtr->ulChannel;
    //}


    {
        char buffer[1024];

        sprintf_s ( buffer, sizeof ( buffer ), "PassThruIoctl(%ul,%s,0x%lx,0x%lx)\n", ChannelID, GetJ2534IOCTLIDText ( IoctlID ), pInput, pOutput );
        LogMsg1 ( buffer );
    }

#if !defined ( SIMULATION_MODE )

    if ( pPtr && pPtr->data.pPassThruIoctl ) {
        char buffer[1024];
        
        ret = ( J2534ERROR ) pPtr->data.pPassThruIoctl ( ChannelID, IoctlID, pInput, pOutput );
        
        sprintf_s ( buffer, sizeof ( buffer ), "%d = pPassThruIoctl()\n", ret );
        LogMsg1 ( buffer );

        GetJ2534ErrorText ( ret );

        if (pInput) {
            auto sconfigs = reinterpret_cast<SCONFIG_LIST *>(pInput);
            for (unsigned long i = 0; i < sconfigs->NumOfParams; i++) {
				auto configPtr = sconfigs->ConfigPtr + i;
				if (configPtr) {
                	char buffer[128];
                	sprintf_s ( buffer, sizeof ( buffer ), "output pInput[%lu] P = %lu, v = %lu\n", i, configPtr->Parameter, configPtr->Value);
                	LogMsg1 ( buffer );
				}
            }
        }
        if (pOutput) {
                char buffer[128];
                sprintf_s ( buffer, sizeof ( buffer ), "output *pOutput = %lu\n", *reinterpret_cast<unsigned long *>(pOutput));
                LogMsg1 ( buffer );
        }
    }
#else
    SetIoCtlOutputValue(IoctlID, NULL, pOutput);
#endif


    return ret;
}

static int Load_J2534DLL (  const char *szLibrary, globData * data )
{
    HINSTANCE hDLL;

    memset ( data, 0, sizeof ( globData ) );

    /**********************************************************************/
    //
    //	Once user selects the specifie API & hardware, load the vendor
    //	supplied API calls.
    //
    /**********************************************************************/

    if ( ( hDLL = LoadLibraryA ( szLibrary ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot load %s\n", szLibrary );
        return ( FAIL );
    }

    /// 0404
    if ( ( data->pPassThruOpen = ( PTOPEN ) GetProcAddress ( hDLL, "PassThruOpen" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruOpen function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruClose = ( PTCLOSE ) GetProcAddress ( hDLL, "PassThruClose" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruClose function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruConnect = ( PTCONNECT ) GetProcAddress ( hDLL, "PassThruConnect" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruConnect function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruDisconnect = ( PTDISCONNECT ) GetProcAddress ( hDLL, "PassThruDisconnect" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruDisconnect function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruReadMsgs = ( PTREADMSGS ) GetProcAddress ( hDLL, "PassThruReadMsgs" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruReadMsgs function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruWriteMsgs = ( PTWRITEMSGS ) GetProcAddress ( hDLL, "PassThruWriteMsgs" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruWriteMsgs function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruStartPeriodicMsg = ( PTSTARTPERIODICMSG ) GetProcAddress ( hDLL, "PassThruStartPeriodicMsg" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruStartPeriodicMsg function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruStopPeriodicMsg = ( PTSTOPPERIODICMSG ) GetProcAddress ( hDLL, "PassThruStopPeriodicMsg" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruStopPeriodicMsg function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruStartMsgFilter = ( PTSTARTMSGFILTER ) GetProcAddress ( hDLL, "PassThruStartMsgFilter" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruStartMsgFilter function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruStopMsgFilter = ( PTSTOPMSGFILTER ) GetProcAddress ( hDLL, "PassThruStopMsgFilter" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruStopMsgFilter function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruSetProgrammingVoltage = ( PTSETPROGRAMMINGVOLTAGE ) GetProcAddress ( hDLL, "PassThruSetProgrammingVoltage" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruSetProgrammingVoltage function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruReadVersion = ( PTREADVERSION ) GetProcAddress ( hDLL, "PassThruReadVersion" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruReadVersion function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruGetLastError = ( PTGETLASTERROR ) GetProcAddress ( hDLL, "PassThruGetLastError" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruGetLastError function in %s\n", szLibrary );
        return ( FAIL );
    }

    if ( ( data->pPassThruIoctl = ( PTIOCTL ) GetProcAddress ( hDLL, "PassThruIoctl" ) ) == NULL ) {
        LogMsg2 ( "ERROR: Cannot find PassThruIoctl function in %s\n", szLibrary );
        return ( FAIL );
    }

    return ( PASS );
}

JTYPE PassThruExConfigureWiFi ( void )
{
    LogMsg1 ( "PassThruExConfigureWiFi" );
    __asm int 3

    return 0;
}
JTYPE PassThruExDeviceWatchdog ( void )
{
    LogMsg1 ( "PassThruExDeviceWatchdog" );
    __asm int 3
    return 0;
}

JTYPE PassThruExDownloadCypheredFlashData ( void )
{
    LogMsg1 ( "PassThruExDownloadCypheredFlashData" );
    __asm int 3
    return 0;
}

JTYPE PassThruExEraseFlash ( void )
{
    LogMsg1 ( "PassThruExEraseFlash" );
    __asm int 3
    return 0;
}

JTYPE PassThruExInitiateCypheredFlashDownload ( void )
{
    LogMsg1 ( "" );
    __asm int 3
    return 0;
}

JTYPE PassThruExReadFlash ( void )
{
    LogMsg1 ( "PassThruExReadFlash" );
    __asm int 3
    return 0;
}

JTYPE PassThruExResetFlash ( void )
{
    LogMsg1 ( "PassThruExResetFlash" );
    __asm int 3
    return 0;
}

JTYPE PassThruExRunSelfTest ( void )
{
    LogMsg1 ( "PassThruExRunSelfTest" );
    __asm int 3
    return 0;
}

JTYPE PassThruExWriteFlash ( void )
{
    LogMsg1 ( "PassThruExWriteFlash" );
    __asm int 3
    return 0;
}

