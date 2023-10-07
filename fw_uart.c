
#include <stdlib.h>
#include <time.h>
#include "libserialport/libserialport.h"
#include "libuv/include/uv.h"
#include "fw_define.h"
#include "fw_upg.h"
#include "fw0/fw0_define.h"

//############# fwu #####################//
static fw_time xTimeget(void)
{
	uv_timespec64_t timespec64_t={0,0.0};
	uv_clock_gettime(UV_CLOCK_REALTIME, &timespec64_t);
	double ms = timespec64_t.tv_nsec / 1000000.0;//ms
	fw_time fw_time_;
	fw_time_.time = timespec64_t.tv_sec;
	fw_time_.ms = ms;
	return fw_time_;
}
static fw_time_t xDifftime(fw_time start_time, fw_time end_time)
{
	return ((end_time.time - start_time.time) * 1000 + (end_time.ms - start_time.ms));
}
static void xTime2str(fw_time fw_time_ , char* buffer, int n)
{
	fw_time time_=xTimeget();
	struct tm* tm_ = localtime(&time_.time);
	char tempc[200] = {0};
	strftime(tempc,200, "%Y-%m-%d %H:%M:%S", tm_);//"%Y-%m-%d %H:%M:%S"
	snprintf(buffer, n, "%s.%d", tempc, (int)(time_.ms*1000));
}
static int xPrint(int msg_type, const  char* msg)
{
	printf("%s",msg);
	return fflush(stdout);
}
static  char*  xGetLastError(int *errorcode)
{
	*errorcode = sp_last_error_code();
	return sp_last_error_message();
}
static void xSleep(int ms)
{
	uv_sleep(ms);
}
//############ base io ###################//
static int xOpen (const char*name,int baudrate, int open_mode, io_object* io_object_)
{
	struct sp_port* sp_port_ = io_object_->userdata;
	return sp_open(sp_port_, open_mode);
}

static int xColse(io_object* io_object_)
{
	struct sp_port* sp_port_ = io_object_->userdata;
	return sp_close(sp_port_);
}
static int xIsOpen(io_object* io_object_)
{
	return  io_object_->userdata!=NULL?1:0;
}
static int xRead(char* read_data, int read_size, int timeout, io_object* io_object_)
{
	struct sp_port* sp_port_ = io_object_->userdata;
	return sp_blocking_read(sp_port_, read_data, read_size, timeout);
}
static int xWrite(char* write_data, int data_size, int timeout, io_object* io_object_)
{
	struct sp_port* sp_port_ = io_object_->userdata;
	return sp_blocking_write(sp_port_, write_data, data_size, timeout);
}
static int xInputSize(io_object* io_object_)
{
	struct sp_port* sp_port_ = io_object_->userdata;
	return sp_input_waiting(sp_port_);
}

static int xOutputSize(io_object* io_object_)
{
	struct sp_port* sp_port_ = io_object_->userdata;
	return sp_output_waiting(sp_port_);
}
//############ uart ######################//
static int xSetBaudRate(int  baudrate, uart_t* uart_t_)
{
	struct sp_port *sp_port_=	uart_t_->io_object_.userdata;
	return sp_set_baudrate(sp_port_, baudrate);
}
static int xGetBaudRate(int* baudrate, uart_t*uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	struct sp_port_config* config;
	sp_new_config(&config);
	sp_get_config(sp_port_, config);
	sp_get_config_baudrate(config, baudrate);
	return *baudrate;
}

static int xSetDataBits(int  baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_set_bits (sp_port_, baudrate);
}
static int xGetDataBits(int* baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	struct sp_port_config* config;
	sp_new_config(&config);
	sp_get_config(sp_port_, config);
	sp_get_config_bits(config, baudrate);
	return *baudrate;
}

static int xSetParity(int  baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_set_parity(sp_port_, baudrate);
}
static int xGetParity(int* baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	struct sp_port_config* config;
	sp_new_config(&config);
	sp_get_config(sp_port_, config);
	sp_get_config_parity(config, baudrate);
	return *baudrate;
}

static int xSetStopBits(int  baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_set_stopbits(sp_port_, baudrate);
}
static int xGetStopBits(int* baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	struct sp_port_config* config;
	sp_new_config(&config);
	sp_get_config(sp_port_, config);
	sp_get_config_stopbits(config, baudrate);
	return *baudrate;
}

static int xSetFlowControl(int  baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_set_flowcontrol(sp_port_, baudrate);
}

static int xSetDTR(int  baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_set_dtr(sp_port_, baudrate);
}
static int xGetDTR(int* baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	struct sp_port_config* config;
	sp_new_config(&config);
	sp_get_config(sp_port_, config);
	sp_get_config_dtr(config, baudrate);
	return *baudrate;
}

static int xSetRTS(int  baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_set_rts(sp_port_, baudrate);
}
static int xGetRTS(int* baudrate, uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	struct sp_port_config* config;
	sp_new_config(&config);
	sp_get_config(sp_port_, config);
	sp_get_config_rts(config, baudrate);
	return *baudrate;
}

static char* xGetPortName(uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_get_port_name(sp_port_);
}

static int xClear(uart_t* uart_t_)
{
	struct sp_port* sp_port_ = uart_t_->io_object_.userdata;
	return sp_flush(sp_port_, SP_BUF_BOTH);
}


void init_fw(fwu* fwu_)
{
	uart_t* uart_t_ = &fwu_->uart_t_;
	uart_t_->xSetBaudRate = xSetBaudRate;
	uart_t_->xGetBaudRate = xGetBaudRate;
	uart_t_->xSetDataBits = xSetDataBits;
	uart_t_->xGetDataBits = xGetDataBits;
	uart_t_->xSetParity = xSetParity;
	uart_t_->xGetParity = xGetParity;
	uart_t_->xSetStopBits = xSetStopBits;
	uart_t_->xGetStopBits = xGetStopBits;
	uart_t_->xSetFlowControl = xSetFlowControl;
	uart_t_->xSetDTR = xSetDTR;
	uart_t_->xGetDTR = xGetDTR;
	uart_t_->xSetRTS = xSetRTS;
	uart_t_->xGetRTS = xGetRTS;
	uart_t_->xGetPortName = xGetPortName;
	uart_t_->xClear = xClear;

	io_object* ioobject = &uart_t_->io_object_;
	fwu_->xTimeget = xTimeget;
	fwu_->xTime2str = xTime2str;
	fwu_->xDifftime = xDifftime;
	fwu_->xPrint = xPrint;
	fwu_->xGetLastError = xGetLastError;
	fwu_->xSleep = xSleep;

	ioobject->xOpen = xOpen;
	ioobject->xColse = xColse;
	ioobject->xIsOpen = xIsOpen;
	ioobject->xRead = xRead;
	ioobject->xWrite = xWrite;
	ioobject->xInputSize = xInputSize;
	ioobject->xOutputSize = xOutputSize;
}

