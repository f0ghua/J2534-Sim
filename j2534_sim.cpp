#include "StdAfx.h"
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
#include <stdexcept>

#define F_HIJACK

// simple simulation when no hardware
//#define SIMULATION_MODE 1

#define FAIL	( 0 )
#define PASS	( 1 )


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

//std::shared_ptr<spdlog::logger> kDefaultLogger;

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    size_t size = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}

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

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(file_sink);

    auto kDefaultLogger = std::make_shared<spdlog::logger>("multi_sink", begin( sinks ), end( sinks ));
    spdlog::register_logger(kDefaultLogger);
    spdlog::set_default_logger(kDefaultLogger);
    spdlog::set_level(spdlog::level::trace);
}

//static void LogMsg1 ( const char *str )
//{
//    kDefaultLogger->debug("{}", str);
//}

//static void LogMsg2 ( const char *str, const char *str1 )
//{
//    kDefaultLogger->debug("{} {}", str, str1);
//}

static char const *getJ2534ProtocolText(unsigned long protocol)
{
    static char buffer[128] = {0};

    memset(buffer, 0, sizeof(buffer));
    switch ( protocol ) {
        case J1850VPW:								// J1850VPW Protocol
            sprintf_s(buffer, sizeof(buffer), "J1850VPW(%lu)", protocol);
            break;

        case J1850PWM:								// J1850PWM Protocol
            sprintf_s(buffer, sizeof(buffer), "J1850PWM(%lu)", protocol);
            break;

        case ISO9141:								// ISO9141 Protocol
            sprintf_s(buffer, sizeof(buffer), "ISO9141(%lu)", protocol);
            break;

        case ISO14230:								// ISO14230 Protocol
            sprintf_s(buffer, sizeof(buffer), "ISO14230(%lu)", protocol);
            break;

        case CAN:									// CAN Protocol
            sprintf_s(buffer, sizeof(buffer), "CAN(%lu)", protocol);
            break;

        case ISO15765:
            sprintf_s(buffer, sizeof(buffer), "ISO15765(%lu)", protocol);
            break;

        case SCI_A_ENGINE:
            sprintf_s(buffer, sizeof(buffer), "SCI_A_ENGINE(%lu)", protocol);
            break;

        case SCI_A_TRANS:
            sprintf_s(buffer, sizeof(buffer), "SCI_A_TRANS(%lu)", protocol);
            break;

        case SCI_B_ENGINE:
            sprintf_s(buffer, sizeof(buffer), "SCI_B_ENGINE(%lu)", protocol);
            break;

        case SCI_B_TRANS:
            sprintf_s(buffer, sizeof(buffer), "SCI_B_TRANS(%lu)", protocol);
            break;

        case J1850VPW_PS:
            sprintf_s(buffer, sizeof(buffer), "J1850VPW_PS(%lu)", protocol);
            break;

        case J1850PWM_PS:
            sprintf_s(buffer, sizeof(buffer), "J1850PWM_PS(%lu)", protocol);
            break;

        case ISO9141_PS:
            sprintf_s(buffer, sizeof(buffer), "ISO9141_PS(%lu)", protocol);
            break;

        case ISO14230_PS:
            sprintf_s(buffer, sizeof(buffer), "ISO14230_PS(%lu)", protocol);
            break;

        case CAN_PS:
            sprintf_s(buffer, sizeof(buffer), "CAN_PS(%lu)", protocol);
            break;

        case ISO15765_PS:
            sprintf_s(buffer, sizeof(buffer), "ISO15765_PS(%lu)", protocol);
            break;

        case J2610_PS:
            sprintf_s(buffer, sizeof(buffer), "J2610_PS(%lu)", protocol);
            break;

        case SW_ISO15765_PS:
            sprintf_s(buffer, sizeof(buffer), "SW_ISO15765_PS(%lu)", protocol);
            break;

        case SW_CAN_PS:
            sprintf_s(buffer, sizeof(buffer), "SW_CAN_PS(%lu)", protocol);
            break;

        case GM_UART_PS:
            sprintf_s(buffer, sizeof(buffer), "GM_UART_PS(%lu)", protocol);
            break;

        case ISO15765_FD_PS:
            sprintf_s(buffer, sizeof(buffer), "ISO15765_FD_PS(%lu)", protocol);
            break;

        case CAN_FD_PS:
            sprintf_s(buffer, sizeof(buffer), "CAN_FD_PS(%lu)", protocol);
            break;

        default:
            sprintf_s ( buffer, sizeof ( buffer ), "unknown(%lu)", protocol);
            break;
    }

    return buffer;
}

static char const *getJ2534IoctlIdText (unsigned long ioctlId)
{
    static char buffer[128] = {0};

    memset(buffer, 0, sizeof(buffer));
    switch (ioctlId) {

        case GET_CONFIG:
            sprintf_s(buffer, sizeof(buffer), "GET_CONFIG(%lu)", ioctlId);
            break;
        case SET_CONFIG:
            sprintf_s(buffer, sizeof(buffer), "SET_CONFIG(%lu)", ioctlId);
            break;
        case READ_VBATT:
            sprintf_s(buffer, sizeof(buffer), "READ_VBATT(%lu)", ioctlId);
            break;
        case FIVE_BAUD_INIT:
            sprintf_s(buffer, sizeof(buffer), "FIVE_BAUD_INIT(%lu)", ioctlId);
            break;
        case FAST_INIT:
            sprintf_s(buffer, sizeof(buffer), "FAST_INIT(%lu)", ioctlId);
            break;
        case SET_PIN_USE:
            sprintf_s(buffer, sizeof(buffer), "SET_PIN_USE(%lu)", ioctlId);
            break;
        case CLEAR_TX_BUFFER:
            sprintf_s(buffer, sizeof(buffer), "CLEAR_TX_BUFFER(%lu)", ioctlId);
            break;
        case CLEAR_RX_BUFFER:
            sprintf_s(buffer, sizeof(buffer), "CLEAR_RX_BUFFER(%lu)", ioctlId);
            break;
        case CLEAR_PERIODIC_MSGS:
            sprintf_s(buffer, sizeof(buffer), "CLEAR_PERIODIC_MSGS(%lu)", ioctlId);
            break;
        case CLEAR_MSG_FILTERS:
            sprintf_s(buffer, sizeof(buffer), "CLEAR_MSG_FILTERS(%lu)", ioctlId);
            break;
        case CLEAR_FUNCT_MSG_LOOKUP_TABLE:
            sprintf_s(buffer, sizeof(buffer), "CLEAR_FUNCT_MSG_LOOKUP_TABLE(%lu)", ioctlId);
            break;
        case ADD_TO_FUNCT_MSG_LOOKUP_TABLE:
            sprintf_s(buffer, sizeof(buffer), "ADD_TO_FUNCT_MSG_LOOKUP_TABLE(%lu)", ioctlId);
            break;
        case DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE:
            sprintf_s(buffer, sizeof(buffer), "DELETE_FROM_FUNCT_MSG_LOOKUP_TABLE(%lu)", ioctlId);
            break;
        case READ_PROG_VOLTAGE:
            sprintf_s(buffer, sizeof(buffer), "READ_PROG_VOLTAGE(%lu)", ioctlId);
            break;
        default:
            sprintf_s(buffer, sizeof(buffer), "unknow(%lu)", ioctlId);
            break;
    }

    return buffer;
}

static char const *getJ2534IoctlParamIdText(unsigned long param)
{
    static char buffer[128] = {0};

    memset(buffer, 0, sizeof(buffer));

    switch (param) {
        case DATA_RATE:
            sprintf_s(buffer, sizeof(buffer), "DATA_RATE(%lu)", param);
            break;
        case LOOPBACK:
            sprintf_s(buffer, sizeof(buffer), "LOOPBACK(%lu)", param);
            break;
        case NODE_ADDRESS:
            sprintf_s(buffer, sizeof(buffer), "NODE_ADDRESS(%lu)", param);
            break;
        case NETWORK_LINE:
            sprintf_s(buffer, sizeof(buffer), "NETWORK_LINE(%lu)", param);
            break;
        case P1_MIN:
            sprintf_s(buffer, sizeof(buffer), "P1_MIN(%lu)", param);
            break;
        case P1_MAX:
            sprintf_s(buffer, sizeof(buffer), "P1_MAX(%lu)", param);
            break;
        case P2_MIN:
            sprintf_s(buffer, sizeof(buffer), "P2_MIN(%lu)", param);
            break;
        case P2_MAX:
            sprintf_s(buffer, sizeof(buffer), "P2_MAX(%lu)", param);
            break;
        case P3_MIN:
            sprintf_s(buffer, sizeof(buffer), "P3_MIN(%lu)", param);
            break;
        case P3_MAX:
            sprintf_s(buffer, sizeof(buffer), "P3_MAX(%lu)", param);
            break;
        case P4_MIN:
            sprintf_s(buffer, sizeof(buffer), "P4_MIN(%lu)", param);
            break;
        case P4_MAX:
            sprintf_s(buffer, sizeof(buffer), "P4_MAX(%lu)", param);
            break;
        case W1:
            sprintf_s(buffer, sizeof(buffer), "W1(%lu)", param);
            break;
        case W2:
            sprintf_s(buffer, sizeof(buffer), "W2(%lu)", param);
            break;
        case W3:
            sprintf_s(buffer, sizeof(buffer), "W3(%lu)", param);
            break;
        case W4:
            sprintf_s(buffer, sizeof(buffer), "W4(%lu)", param);
            break;
        case W5:
            sprintf_s(buffer, sizeof(buffer), "W5(%lu)", param);
            break;
        case TIDLE:
            sprintf_s(buffer, sizeof(buffer), "TIDLE(%lu)", param);
            break;
        case TINIL:
            sprintf_s(buffer, sizeof(buffer), "TINIL(%lu)", param);
            break;
        case TWUP:
            sprintf_s(buffer, sizeof(buffer), "TWUP(%lu)", param);
            break;
        case PARITY:
            sprintf_s(buffer, sizeof(buffer), "PARITY(%lu)", param);
            break;
        case BIT_SAMPLE_POINT:
            sprintf_s(buffer, sizeof(buffer), "BIT_SAMPLE_POINT(%lu)", param);
            break;
        case SYNC_JUMP_WIDTH:
            sprintf_s(buffer, sizeof(buffer), "SYNC_JUMP_WIDTH(%lu)", param);
            break;
//        case W0:
//            return "W0";
        case T1_MAX:
            sprintf_s(buffer, sizeof(buffer), "T1_MAX(%lu)", param);
            break;
        case T2_MAX:
            sprintf_s(buffer, sizeof(buffer), "T2_MAX(%lu)", param);
            break;
        case T4_MAX:
            sprintf_s(buffer, sizeof(buffer), "T4_MAX(%lu)", param);
            break;
        case T5_MAX:
            sprintf_s(buffer, sizeof(buffer), "T5_MAX(%lu)", param);
            break;
        case ISO15765_BS:
            sprintf_s(buffer, sizeof(buffer), "ISO15765_BS(%lu)", param);
            break;
        case ISO15765_STMIN:
            sprintf_s(buffer, sizeof(buffer), "ISO15765_STMIN(%lu)", param);
            break;
        case DATA_BITS:
            sprintf_s(buffer, sizeof(buffer), "DATA_BITS(%lu)", param);
            break;
        default:
            sprintf_s(buffer, sizeof(buffer), "unknow(%lu)", param);
            break;
    }

    return buffer;
}

/*
 * GetJ2534ErrorText - Convert error code to text
 *	Input: J2534ERROR err
 */

static char const * getJ2534ErrorText ( J2534ERROR err )
{
    static char buffer[128] = {0};
    memset(buffer, 0, sizeof(buffer));

    switch ( err ) {
        case STATUS_NOERROR						:         //    0x00
            sprintf_s(buffer, sizeof(buffer), "STATUS_NOERROR(%lu)", err);
            break;

        case ERR_NOT_SUPPORTED					:         //    0x01
            sprintf_s(buffer, sizeof(buffer), "ERR_NOT_SUPPORTED(%lu)", err);
            break;

        case ERR_INVALID_CHANNEL_ID					:         //    0x02
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_CHANNEL_ID(%lu)", err);
            break;

        case ERR_INVALID_PROTOCOL_ID					:         //    0x03
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_PROTOCOL_ID(%lu)", err);
            break;

        case ERR_NULL_PARAMETER					:         //    0x04
            sprintf_s(buffer, sizeof(buffer), "ERR_NULL_PARAMETER(%lu)", err);
            break;

        case ERR_INVALID_IOCTL_VALUE					:         //    0x05
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_IOCTL_VALUE(%lu)", err);
            break;

        case ERR_INVALID_FLAGS					:         //    0x06
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_FLAGS(%lu)", err);
            break;

        case ERR_FAILED						:         //    0x07
            sprintf_s(buffer, sizeof(buffer), "ERR_FAILED(%lu)", err);
            break;

        case ERR_DEVICE_NOT_CONNECTED				:         //    0x08
            sprintf_s(buffer, sizeof(buffer), "ERR_DEVICE_NOT_CONNECTED(%lu)", err);
            break;

        case ERR_TIMEOUT						:         //    0x09
            sprintf_s(buffer, sizeof(buffer), "ERR_TIMEOUT(%lu)", err);
            break;

        case ERR_INVALID_MSG						:         //    0x0A
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_MSG(%lu)", err);
            break;

        case ERR_INVALID_TIME_INTERVAL				:         //    0x0B
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_TIME_INTERVAL(%lu)", err);
            break;

        case ERR_EXCEEDED_LIMIT					:         //    0x0C
            sprintf_s(buffer, sizeof(buffer), "ERR_EXCEEDED_LIMIT(%lu)", err);
            break;

        case ERR_INVALID_MSG_ID					:         //    0x0D
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_MSG_ID(%lu)", err);
            break;

        case ERR_DEVICE_IN_USE					:         //    0x0E
            sprintf_s(buffer, sizeof(buffer), "ERR_DEVICE_IN_USE(%lu)", err);
            break;

        case ERR_INVALID_IOCTL_ID					:         //    0x0F
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_IOCTL_ID(%lu)", err);
            break;

        case ERR_BUFFER_EMPTY					:         //    0x10
            sprintf_s(buffer, sizeof(buffer), "ERR_BUFFER_EMPTY(%lu)", err);
            break;

        case ERR_BUFFER_FULL						:         //    0x11
            sprintf_s(buffer, sizeof(buffer), "ERR_BUFFER_FULL(%lu)", err);
            break;

        case ERR_BUFFER_OVERFLOW					:         //    0x12
            sprintf_s(buffer, sizeof(buffer), "ERR_BUFFER_OVERFLOW(%lu)", err);
            break;

        case ERR_PIN_INVALID						:         //    0x13
            sprintf_s(buffer, sizeof(buffer), "ERR_PIN_INVALID(%lu)", err);
            break;

        case ERR_CHANNEL_IN_USE					:         //    0x14
            sprintf_s(buffer, sizeof(buffer), "ERR_CHANNEL_IN_USE(%lu)", err);
            break;

        case ERR_MSG_PROTOCOL_ID					:         //    0x15
            sprintf_s(buffer, sizeof(buffer), "ERR_MSG_PROTOCOL_ID(%lu)", err);
            break;

        case ERR_INVALID_FILTER_ID					:         //    0x16
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_FILTER_ID(%lu)", err);
            break;

        case ERR_NO_FLOW_CONTROL					:         //    0x17
            sprintf_s(buffer, sizeof(buffer), "ERR_NO_FLOW_CONTROL(%lu)", err);
            break;

        case ERR_NOT_UNIQUE						:         //    0x18
            sprintf_s(buffer, sizeof(buffer), "ERR_NOT_UNIQUE(%lu)", err);
            break;

        case ERR_INVALID_BAUDRATE					:         //    0x19
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_BAUDRATE(%lu)", err);
            break;

        case ERR_INVALID_DEVICE_ID					:         //    0x1A
            sprintf_s(buffer, sizeof(buffer), "ERR_INVALID_DEVICE_ID(%lu)", err);
            break;
        default:
            sprintf_s(buffer, sizeof(buffer), "unknow(%lu)", err);
            break;
    }

    return buffer;
}

static void printPASSTHRU_MSG(PASSTHRU_MSG *pMsg, char const *msgName = "PASSTHRU_MSG:", char const *indent = "  ");
static void printPASSTHRU_MSG(PASSTHRU_MSG *pMsg, char const *msgName, char const *indent)
{
    if (pMsg != nullptr) {
        SPDLOG_TRACE("{}{}", indent, msgName);
        SPDLOG_TRACE("{}{}pMsg->ProtocolID = {}", indent, indent, getJ2534ProtocolText(pMsg->ProtocolID));
        SPDLOG_TRACE("{}{}pMsg->RxStatus = 0x{:x}", indent, indent, pMsg->RxStatus);
        SPDLOG_TRACE("{}{}pMsg->TxFlags = 0x{:x}", indent, indent, pMsg->TxFlags);
        SPDLOG_TRACE("{}{}pMsg->DataSize = {}", indent, indent, pMsg->DataSize);
        SPDLOG_TRACE("{}{}pMsg->Data = [{:n}]", indent, indent, spdlog::to_hex(pMsg->Data, pMsg->Data+pMsg->DataSize));
        SPDLOG_TRACE("{}{}pMsg->ExtraDataIndex = {}", indent, indent, pMsg->ExtraDataIndex);
        SPDLOG_TRACE("{}{}pMsg->Timestamp = {}", indent, indent, pMsg->Timestamp);
    }
}

#if defined ( SIMULATION_MODE )
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
#endif

// Global functions ------------------------------------------------------------------------------

BOOL APIENTRY DllMain ( HANDLE hModule,
                        DWORD  ul_reason_for_call,
                        LPVOID lpReserved
                      )
{

    (void)hModule;
    (void)lpReserved;

    switch ( ul_reason_for_call ) {
        case DLL_PROCESS_ATTACH:
            setupLogger();
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }

    return TRUE;
}

//void PrintBuffer ( int length, unsigned char *data )
//{
//    char buffer[1024];
//    char buffer2[1024];
//    int i;

//    if ( data == NULL ) { return; }

//    if ( length == 0 ) { return; }


//    memset ( buffer, 0, sizeof ( buffer ) );
//    memset ( buffer2, 0, sizeof ( buffer2 ) );

//    for ( i = 0; i < length; i++ ) {

//        sprintf_s ( buffer2, sizeof ( buffer2 ), "0x%02x ", *data++ );

//        strcat_s ( buffer, sizeof ( buffer ), buffer2 );
//    }

//    LogMsg1 ( buffer );
//}

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

    SPDLOG_TRACE("Load_J2534DLL");

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
    (void)ChannelID;

    stPassThrough *pPtr = pGlobalPtr;
//    stPassThrough *pPtr = ( stPassThrough* ) ChannelID ;
    
    return pPtr;
}

JTYPE  PassThruOpen ( void* pName, unsigned long * pDeviceID )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruOpen(pName = {}, pDeviceID = 0x{:x})", pName?(char *)pName:"null", (uint32_t)pDeviceID);

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = STATUS_NOERROR ;
    if ( pPtr == NULL )
    {
        Load_J2534DLL();
    }

    if ( pGlobalPtr && pGlobalPtr->data.pPassThruOpen ) {
		pPtr = pGlobalPtr;
        ret = ( J2534ERROR ) pPtr->data.pPassThruOpen ( pName, pDeviceID );
    }
#ifdef F_HIJACK
    else {
        if ( pDeviceID ) {
            *pDeviceID = ( unsigned long ) 0;
        }
    }
#endif

    SPDLOG_TRACE("PassThruOpen exit with deviceId = {}, ret = {}", *pDeviceID, getJ2534ErrorText(ret));

    return ret;
}


JTYPE  J2534_SIM_API  PassThruClose ( unsigned long DeviceID )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruClose(DeviceID = {})", DeviceID);

    J2534ERROR ret = J2534_STATUS_NOERROR;

    if ( pGlobalPtr && pGlobalPtr->data.pPassThruClose ) {
        ret = ( J2534ERROR ) pGlobalPtr->data.pPassThruClose ( DeviceID );
    }

    SPDLOG_TRACE("PassThruClose exit with deviceId = {}, ret = {}", DeviceID, getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruConnect ( unsigned long DeviceID, unsigned long ProtocolID, unsigned long Flags, unsigned long Baudrate, unsigned long * pChannelID )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruConnect(DeviceID = {}, ProtocolID = {}, Flags = 0x{:x}, BaudRate = {}, pChannelID = 0x{:x})",
                 DeviceID, getJ2534ProtocolText(ProtocolID), Flags, Baudrate, (uint32_t)pChannelID);

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = pGlobalPtr ;

    if ( pPtr == NULL ) {
        Load_J2534DLL();
        pPtr = pGlobalPtr ;
    }

    if ( pPtr && pPtr->data.pPassThruConnect )	{
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

    SPDLOG_TRACE("PassThruConnect assigned channelID = {}, ret = {}", *pChannelID, getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruDisconnect ( unsigned long ChannelID )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruDisconnect(ChannelID = {})", ChannelID);

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr )
    { ChannelID = pPtr->ulChannel; }

    pPtr = pGlobalPtr;


    if ( pPtr && pPtr->data.pPassThruDisconnect ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruDisconnect ( ChannelID );
    }

    SPDLOG_TRACE("PassThruClose exit with ret = {}", getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruReadMsgs ( unsigned long ChannelID, PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruReadMsgs(ChannelID = {}, N = {}, T = {})",
                 ChannelID, *pNumMsgs, Timeout);

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    pPtr = pGlobalPtr;

    if ( pPtr && pPtr->data.pPassThruReadMsgs ) {

        //	Sleep(20);
        ret = ( J2534ERROR ) pPtr->data.pPassThruReadMsgs ( ChannelID, pMsg, pNumMsgs, Timeout );

        SPDLOG_TRACE("DLL's PassThruReadMsgs return ret = {}, N = {}", ret, *pNumMsgs);
    }

    if ( ret == J2534_STATUS_NOERROR && pMsg->ulDataSize ) {
        SPDLOG_TRACE("PassThruReadMsgs has been read {} messages", *pNumMsgs);
        for (unsigned long i = 0; i < *pNumMsgs; i++) {
            auto msgName = string_format("pMsg[%lu]", i);
            printPASSTHRU_MSG(pMsg+i, msgName.c_str());
        }
    } else if ( ret != ERR_BUFFER_EMPTY ) {

    } else {

    }

    SPDLOG_TRACE("PassThruReadMsgs exit with ret = {}", getJ2534ErrorText(ret));
    return ret;

}

JTYPE PassThruWriteMsgs ( unsigned long ChannelID, PASSTHRU_MSG * pMsg, unsigned long * pNumMsgs, unsigned long Timeout )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruWriteMsgs(ChannelID = {}, *pNumMsgs = {}, Timeout = {})",
                 ChannelID, *pNumMsgs, Timeout);
    for (unsigned long i = 0; i < *pNumMsgs; i++) {
        auto msgName = string_format("pMsg[%lu]", i);
        printPASSTHRU_MSG(pMsg+i, msgName.c_str());
    }

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    pPtr = pGlobalPtr;

    if ( pPtr && pPtr->data.pPassThruWriteMsgs ) {

//		Sleep( 10 );
        SPDLOG_TRACE("DLL's PassThruWriteMsgs(ChannelID = {}, *pNumMsgs = {}, Timeout = {})",
                     ChannelID, *pNumMsgs, Timeout);

        ret = ( J2534ERROR ) pPtr->data.pPassThruWriteMsgs ( ChannelID, pMsg, pNumMsgs, Timeout );

        SPDLOG_TRACE("DLL's PassThruWriteMsgs ret = {}, N = {}", getJ2534ErrorText(ret), *pNumMsgs);
    }

    SPDLOG_TRACE("PassThruWriteMsgs ret = {}, N = {}", getJ2534ErrorText(ret), *pNumMsgs);

    return ret;
}

JTYPE PassThruStartPeriodicMsg ( unsigned long ChannelID, PASSTHRU_MSG * pMsg,
                                 unsigned long * pMsgID, unsigned long TimeInterval )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruStartPeriodicMsg(ChannelID = {}, TimeInterval = {})",
                 ChannelID, TimeInterval);
    printPASSTHRU_MSG(pMsg, "pMsg");

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

    pPtr = pGlobalPtr;

    if ( pPtr && pPtr->data.pPassThruStartPeriodicMsg ) {
        SPDLOG_TRACE("DLL's PassThruStartPeriodicMsg(ChannelID = {}, TimeInterval = {})",
                     ChannelID, TimeInterval);

        ret = ( J2534ERROR ) pPtr->data.pPassThruStartPeriodicMsg ( ChannelID, pMsg, pMsgID, TimeInterval );

        SPDLOG_TRACE("DLL's PassThruStartPeriodicMsg ret = {}", getJ2534ErrorText(ret));
    }

    SPDLOG_TRACE("PassThruStartPeriodicMsg ret = {}", getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruStopPeriodicMsg ( unsigned long ChannelID, unsigned long MsgID )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruStopPeriodicMsg(ChannelID = {}, MsgID = {})",
                 ChannelID, MsgID);

    J2534ERROR ret = J2534_STATUS_NOERROR;

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;
    ChannelID = pPtr->ulChannel;

    pPtr = pGlobalPtr;

    if ( pPtr && pPtr->data.pPassThruStopPeriodicMsg ) {
        SPDLOG_TRACE("DLL's PassThruStopPeriodicMsg(ChannelID = {}, MsgID = {})",
                     ChannelID, MsgID);

        ret = ( J2534ERROR ) pPtr->data.pPassThruStopPeriodicMsg ( ChannelID, MsgID );

        SPDLOG_TRACE("DLL's PassThruStopPeriodicMsg ret = {}", getJ2534ErrorText(ret));
    }

    SPDLOG_TRACE("PassThruStopPeriodicMsg ret = {}", getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruStartMsgFilter ( unsigned long ChannelID,
                               unsigned long FilterType, PASSTHRU_MSG * pMsg, PASSTHRU_MSG * pPatternMsg,
                               PASSTHRU_MSG * pFlowControlMsg, unsigned long * pMsgID )
{
    #pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruStartMsgFilter(ChannelID = {}, FilterType = {})", ChannelID, FilterType);
    printPASSTHRU_MSG(pMsg, "pMaskMsg");
    printPASSTHRU_MSG(pPatternMsg, "pPatternMsg");
    printPASSTHRU_MSG(pFlowControlMsg, "pFlowControlMsg");

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID );

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }

#if !defined ( SIMULATION_MODE )

    if ( pPtr && pPtr->data.pPassThruStartMsgFilter ) {

        SPDLOG_TRACE("DLL's PassThruStartMsgFilter(ChannelID = {}, FilterType = {})", ChannelID, FilterType);

        ret = ( J2534ERROR ) pPtr->data.pPassThruStartMsgFilter ( pPtr->ulChannel, FilterType,
                pMsg, pPatternMsg, pFlowControlMsg, pMsgID );

        SPDLOG_TRACE("DLL's PassThruStartMsgFilter ret = {}, FilterID = {}", getJ2534ErrorText(ret), *pMsgID);
    }

#else
    ret  = 0;
#endif

    SPDLOG_TRACE("PassThruStartMsgFilter ret = {}, FilterID = {}", getJ2534ErrorText(ret), *pMsgID);

    return ret;
}

JTYPE PassThruStopMsgFilter ( unsigned long ChannelID, unsigned long ulFilterID )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruStopMsgFilter(ChannelID = {}, FilterID = {})", ChannelID, ulFilterID);

    J2534ERROR ret = J2534_STATUS_NOERROR;

#if !defined ( SIMULATION_MODE )

    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID );

    if ( pPtr ) {
        ChannelID = pPtr->ulChannel;
    }


    if ( pPtr ) {
        SPDLOG_TRACE("DLL's PassThruStopMsgFilter(ChannelID = {}, FilterID = {})", ChannelID, ulFilterID);

        ret = ( J2534ERROR ) pPtr->data.pPassThruStopMsgFilter ( pPtr->ulChannel, ulFilterID );

        SPDLOG_TRACE("DLL's PassThruStopMsgFilter ret = {}", getJ2534ErrorText(ret));
    }

#endif

    SPDLOG_TRACE("PassThruStopMsgFilter ret = {}", getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruSetProgrammingVoltage ( unsigned long DeviceID, unsigned long Pin, unsigned long Voltage )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruSetProgrammingVoltage(DeviceID = {}, Pin = {}, Voltage = {})", DeviceID, Pin, Voltage);

    J2534ERROR ret = J2534_STATUS_NOERROR;

#if !defined ( SIMULATION_MODE )

    stPassThrough *pPtr = pGlobalPtr;

    if ( pPtr ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruSetProgrammingVoltage ( DeviceID, Pin, Voltage );
    }

#endif

    SPDLOG_TRACE("PassThruSetProgrammingVoltage ret = {}", getJ2534ErrorText(ret));

    return ret;
}

JTYPE PassThruReadVersion ( unsigned long DeviceID, char *pchFirmwareVersion, char *pchDllVersion, char *pchApiVersion )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruReadVersion(DeviceID = {})", DeviceID);

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = pGlobalPtr ;

#if !defined ( SIMULATION_MODE )

    if ( pPtr && pPtr->data.pPassThruReadVersion ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruReadVersion ( DeviceID, pchFirmwareVersion, pchDllVersion, pchApiVersion );

        SPDLOG_TRACE("DLL's PassThruReadVersion ret = {}, Firmware = {}, Dll = {}, Api = {})",
                     getJ2534ErrorText(ret), pchFirmwareVersion, pchDllVersion, pchApiVersion);
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

    SPDLOG_TRACE("PassThruReadVersion ret = {}, Firmware = {}, Dll = {}, Api = {})",
                 getJ2534ErrorText(ret), pchFirmwareVersion, pchDllVersion, pchApiVersion);

    return ret;;

}


JTYPE PassThruGetLastError ( char *pErrorDescription )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruGetLastError()");

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = pGlobalPtr;

    if ( pPtr && pPtr->data.pPassThruGetLastError ) {
        ret = ( J2534ERROR ) pPtr->data.pPassThruGetLastError ( pErrorDescription );

        SPDLOG_TRACE("DLL's PassThruGetLastError ret = {}, pErrorDescription = {})",
                     getJ2534ErrorText(ret), pErrorDescription);
    }

    SPDLOG_TRACE("PassThruGetLastError ret = {}, pErrorDescription = {})",
                 getJ2534ErrorText(ret), pErrorDescription);
    return ret;
}

JTYPE PassThruIoctl ( unsigned long ChannelID, unsigned long IoctlID,
                      void *pInput, void *pOutput )
{
#pragma FUNC_EXPORT

    SPDLOG_TRACE("PassThruIoctl(ChannelID = {}, IoctlID = {}, pInput = 0x{:x}, pOutput = 0x{:x})",
                 ChannelID, getJ2534IoctlIdText(IoctlID), reinterpret_cast<std::size_t>(pInput), reinterpret_cast<std::size_t>(pOutput));

    J2534ERROR ret = J2534_STATUS_NOERROR;
    stPassThrough *pPtr = GetGlobalstPassThrough( ChannelID ) ;

    //if ( pPtr ) {
    //    ChannelID = pPtr->ulChannel;
    //}

#if !defined ( SIMULATION_MODE )

    if ( pPtr && pPtr->data.pPassThruIoctl ) {
        SPDLOG_TRACE("DLL's PassThruIoctl(ChannelID = {}, IoctlID = {}, pInput = 0x{:x}, pOutput = 0x{:x})",
                     ChannelID, getJ2534IoctlIdText(IoctlID), reinterpret_cast<std::size_t>(pInput), reinterpret_cast<std::size_t>(pOutput));

        
        ret = ( J2534ERROR ) pPtr->data.pPassThruIoctl ( ChannelID, IoctlID, pInput, pOutput );
        
        SPDLOG_TRACE("DLL's PassThruIoctl ret = {}", getJ2534ErrorText(ret));
        if (pInput) {
            auto sconfigs = reinterpret_cast<SCONFIG_LIST *>(pInput);
            for (unsigned long i = 0; i < sconfigs->NumOfParams; i++) {
                SPDLOG_TRACE("pInput SCONFIG.Parameter = {}, Value = {}",
                             sconfigs->ConfigPtr[i].Parameter, sconfigs->ConfigPtr[i].Value);
            }
        }
        if (pOutput) {
                SPDLOG_TRACE("pOutput Value = {}", *reinterpret_cast<unsigned long *>(pOutput));
        }

    }
#else
    SetIoCtlOutputValue(IoctlID, NULL, pOutput);
#endif

    SPDLOG_TRACE("PassThruIoctl ret = {}", getJ2534ErrorText(ret));
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
        SPDLOG_TRACE("ERROR: Cannot load {}", szLibrary);
        return ( FAIL );
    }

    /// 0404
    if ( ( data->pPassThruOpen = ( PTOPEN ) GetProcAddress ( hDLL, "PassThruOpen" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruOpen function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruClose = ( PTCLOSE ) GetProcAddress ( hDLL, "PassThruClose" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruClose function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruConnect = ( PTCONNECT ) GetProcAddress ( hDLL, "PassThruConnect" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruConnect function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruDisconnect = ( PTDISCONNECT ) GetProcAddress ( hDLL, "PassThruDisconnect" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruDisconnect function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruReadMsgs = ( PTREADMSGS ) GetProcAddress ( hDLL, "PassThruReadMsgs" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruReadMsgs function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruWriteMsgs = ( PTWRITEMSGS ) GetProcAddress ( hDLL, "PassThruWriteMsgs" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruWriteMsgs function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruStartPeriodicMsg = ( PTSTARTPERIODICMSG ) GetProcAddress ( hDLL, "PassThruStartPeriodicMsg" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruStartPeriodicMsg function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruStopPeriodicMsg = ( PTSTOPPERIODICMSG ) GetProcAddress ( hDLL, "PassThruStopPeriodicMsg" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruStopPeriodicMsg function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruStartMsgFilter = ( PTSTARTMSGFILTER ) GetProcAddress ( hDLL, "PassThruStartMsgFilter" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruStartMsgFilter function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruStopMsgFilter = ( PTSTOPMSGFILTER ) GetProcAddress ( hDLL, "PassThruStopMsgFilter" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruStopMsgFilter function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruSetProgrammingVoltage = ( PTSETPROGRAMMINGVOLTAGE ) GetProcAddress ( hDLL, "PassThruSetProgrammingVoltage" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruSetProgrammingVoltage function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruReadVersion = ( PTREADVERSION ) GetProcAddress ( hDLL, "PassThruReadVersion" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruReadVersion function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruGetLastError = ( PTGETLASTERROR ) GetProcAddress ( hDLL, "PassThruGetLastError" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruGetLastError function in {}", szLibrary);
        return ( FAIL );
    }

    if ( ( data->pPassThruIoctl = ( PTIOCTL ) GetProcAddress ( hDLL, "PassThruIoctl" ) ) == NULL ) {
        SPDLOG_TRACE("ERROR: Cannot find PassThruIoctl function in {}", szLibrary);
        return ( FAIL );
    }

    SPDLOG_TRACE("DLL {} is loaded successfully", szLibrary);

    return ( PASS );
}

JTYPE PassThruExConfigureWiFi ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );

    __asm int 3

    return 0;
}
JTYPE PassThruExDeviceWatchdog ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExDownloadCypheredFlashData ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExEraseFlash ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExInitiateCypheredFlashDownload ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExReadFlash ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExResetFlash ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExRunSelfTest ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

JTYPE PassThruExWriteFlash ( void )
{
    SPDLOG_TRACE ( "{}", __FUNCTION__ );
    __asm int 3
    return 0;
}

