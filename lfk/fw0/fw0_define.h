#ifndef __FWU_0__DEFINE__
#define __FWU_0__DEFINE__

#include "fw_define.h"
#include "fw_upg.h"

struct fwu_0_rom
{
    char* fireware;
    int firewaresize;
    const char* name;
    int begin_address;
    int is_bootloader;//1 true
};
struct fwu_0
{
    fwu fwu_;
    int (*xHandshake)(fwu_0*, int /*total handshake timeout(ms)*/,int /*handshake read timeout*/);
    int (*xDisable_wdt)(fwu_0*, int wdt_register_add);
    int (*xSend_da)(fwu_0*, char* DA, int da_size);
    int (*xJump_da)(fwu_0*);
    int (*xSync_da)(fwu_0*, int baudrate);
    int (*xFormat_flash)(fwu_0*, int begin_address,int format_length);
    int (*xSend_fw)(fwu_0*, fwu_0_rom* rom);
};

#endif // !__FWU_0__DEFINE__
