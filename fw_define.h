#ifndef __FIEMWARE_UPGRADE_DEFINE__
#define __FIEMWARE_UPGRADE_DEFINE__

#ifdef _MSC_VER
    #define EXPORT __declspec(dllexport) /* for Windows DLL */
#else
    #define EXPORT
#endif // _MSC_VER

#define LFK_FW0 0


#ifdef LFK_FW0
    #define FWU_0 "fwu_0";
#endif



#ifdef __cplusplus
extern "C" {
#endif // _cplusplus
    //msg type

#define Q_MSG_OK             0
#define Q_MSG_ERROR         -1
#define Q_MSG_UART_ERROR    -2
#define Q_MSG_FILE_ERROR    -3
#define Q_MSG_MANUAL_STOP   -4
#define Q_MSG_TIMEOUT       -5
#define Q_MSG_PROGRESS      -6
#define Q_MSG_DEBUG         -7
#define Q_MSG_WARNING       -8
#define Q_MSG_INFO          -9

//IO mode
#define Q_IO_READ 1
#define Q_IO_WRITE 2
#define Q_IO_RW (Q_IO_READ|Q_IO_WRITE)
//bool type
#define Q_FW_TRUE 1
#define Q_FW_FALSE 0
//const 
#ifdef WIN32
#define Q_FW_END "\r\n"
#else
#define Q_FW_END "\n"
#endif // WIN32

#define Q_WMAX_TIMEOUT 5000
#define Q_RMAX_TIMEOUT 5000

//##################### time struct ######################/
    typedef long long  fw_time_t;
    typedef struct     fw_time   fw_time;
//#####################virtual io#############################//
    typedef struct     io_object io_object;
    typedef struct     uart_s uart_t;
    typedef struct     fwu fwu;

    EXPORT extern void new_fwu(fwu**);
    EXPORT extern void free_fwu(fwu**);

#ifdef FWU_0 //speical code
    typedef struct fwu_0_rom  fwu_0_rom;
    typedef struct fwu_0      fwu_0;
#endif 



#ifdef __cplusplus
}
#endif // _cplusplus
#endif // !__FIEMWARE_UPGRADE_DEFINE__
