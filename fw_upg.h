#ifndef __QUPG__
#define __QUPG__
#include "fw_define.h"

#ifdef __cplusplus
extern "C" {
#endif // _cplusplus

//##################### time struct ######################/

struct fw_time /* time struct */
{
    fw_time_t time;/* time (s) expressed by standard time_t */
    double    ms;  /* fraction of second under 1 s */
};
//#####################virtual io#############################//

struct  io_object
{
    int (*xOpen) (const char* name, int baudrate, int /*open_mode*/, io_object*);
    int (*xColse)(io_object*);
    int (*xIsOpen)(io_object*);
    int (*xRead) (char*/*read_data*/, int/*read_size*/, int /*timeout ms*/, io_object*);
    int (*xWrite)(char*/*write_data*/, int/*data_size*/, int /*timeout ms*/, io_object*);
    int (*xInputSize) (io_object*);
    int (*xOutputSize) (io_object*);
    void * userdata;
};

struct  uart_s
{
    io_object io_object_;

    int (*xSetBaudRate)(int  /*baudrate*/, uart_t*);
    int (*xGetBaudRate)(int* /*baudrate*/, uart_t*);

    int (*xSetDataBits)(int  /*baudrate*/, uart_t*);
    int (*xGetDataBits)(int* /*baudrate*/, uart_t*);

    int (*xSetParity)(int  /*Parity*/, uart_t*);
    int (*xGetParity)(int* /*Parity*/, uart_t*);

    int (*xSetStopBits)(int  /*StopBits*/, uart_t*);
    int (*xGetStopBits)(int* /*StopBits*/, uart_t*);

    int (*xSetFlowControl)(int  /*FlowControl*/, uart_t*);

    int (*xSetDTR)(int  /*DataTerminalReady*/, uart_t*);
    int (*xGetDTR)(int* /*DataTerminalReady*/, uart_t*);

    int (*xSetRTS)(int  /*RequestToSend*/, uart_t*);
    int (*xGetRTS)(int* /*RequestToSend*/, uart_t*);

    char* (*xGetPortName)(uart_t*);

    int (*xClear)(uart_t*);
};

struct  fwu
{
    uart_t uart_t_;
    //############## time func ###########################//
    void (*xTime2str)(fw_time, char*, int n);
    fw_time(*xTimeget)(void);
    fw_time_t(*xDifftime)(fw_time /*start time*/, fw_time/*end time*/);

    //print
    int (*xPrint)(int /*msg type*/, const  char*);

    int  stop;/*1 true 0 false*/
    //system
    char* (*xGetLastError)(int */*error code*/);
    void (*xSleep)(int /*ms*/);
    //common settings
    int common_timeout/*ms*/;
    int is_bigend;/*module  big end or small end*/
};

//################### help marco ############################

#define WRITE_WITH_CHECK(head_str,fwu_,Date,size,timeout,has_progress,res)           { \
    *(res)=Q_MSG_OK;                                                                      \
    if (fwu_->uart_t_.io_object_.xWrite(Date,size,timeout,&fwu_->uart_t_.io_object_)!=size)                                                        \
    {                                                                                         \
        q_print(has_progress == Q_FW_TRUE ? Q_FW_END : " ",                       \
             Q_FW_END, fwu_->xPrint, Q_MSG_UART_ERROR, "%s", head_str);                     \
        *(res) =Q_MSG_UART_ERROR;                                                         \
    }                                                                                         \
}                                                               

//##################### help_func ############################//
EXPORT int q_is_bigend(void);

EXPORT void q_it2str(int send_end, int receive_end,
    char* send_data, int data_size,
    char* receive_data);

EXPORT void q_str2hex(char* str, int len, char* hex);


EXPORT int q_cs2it(char* str, int len, int big_end/*reveive*/);

//##################### _print ############################//

EXPORT void q_progress2str(double progress,
    char progress_c,
    char* progress_str,
    int size,
    const char* heads);

typedef int (*xprint)(int /*msg type*/, const  char*);
EXPORT void q_print(const char* head, const char* end, fwu *, int type, const char* fmt, ...);

//################# fwu #####################################//

EXPORT extern void init_fw(fwu *fwu_);


#ifdef __cplusplus
}
#endif // _cplusplus

#endif // !__QUPG__
