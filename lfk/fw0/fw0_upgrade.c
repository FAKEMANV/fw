#include "fw0/fw0_upgrade.h"
#include "fw_upg.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GET_INIT(fw0_ptr)                  \
 fwu* fwu_ = &_fw_0->fwu_;                   \
 uart_t* uart_method = &fwu_->uart_t_;\
 io_object* uart_io = &uart_method->io_object_;

static int handshake( fwu_0* _fw_0, int timeout, int hr_timeout)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[handshake] Start...(please reset the module!)");

    int c_timeout = fwu_->common_timeout;
    int res = Q_MSG_OK;
    if (uart_io->xIsOpen(uart_io) != Q_FW_TRUE)
    {
        q_print("[handshake]", Q_FW_END, fwu_, Q_MSG_UART_ERROR, NULL);
        return Q_MSG_UART_ERROR;
    }
    fw_time start_t = fwu_->xTimeget();
    fw_time end_t =   fwu_->xTimeget();
 
    char  send_c = 0xA0;
    double total_ms = fwu_->xDifftime(start_t, end_t);
    char progress_c[200];
    uart_method->xClear(uart_method);//clear buffer
    while (total_ms <timeout)
    {
        end_t = fwu_->xTimeget();
        total_ms = fwu_->xDifftime(start_t, end_t);
        //print progress
        memset(progress_c, 0, sizeof(progress_c));
        q_progress2str(total_ms / (double)timeout, '#', progress_c, sizeof(progress_c), "[handshake]");
        q_print(NULL, "\r", fwu_, Q_MSG_PROGRESS, progress_c,"");
        char tempc = 0;
        if (fwu_->stop == Q_FW_TRUE)
        {
            q_print(Q_FW_END "[handshake] Loop send 0xA0 ", Q_FW_END, fwu_, Q_MSG_MANUAL_STOP, NULL);
            res= Q_MSG_MANUAL_STOP;
            break;
        }
        WRITE_WITH_CHECK(Q_FW_END "[handshake] Loop send 0xA0", fwu_, &send_c, 1, Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        break;

        if (uart_io->xRead(&tempc, 1, hr_timeout, &fwu_->uart_t_.io_object_) > 0)
        {
            if (tempc == 0x5F)
            {
                send_c = 0x0A;
                WRITE_WITH_CHECK(Q_FW_END "[handshake]  send 0x0A", fwu_, &send_c, 1, Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
                if (res != Q_MSG_OK)
                break;

                tempc = 0;
                uart_io->xRead(&tempc, 1, c_timeout, &fwu_->uart_t_.io_object_);
                if (tempc == 0xF5)
                {
                    send_c = 0x50;
                    WRITE_WITH_CHECK(Q_FW_END "[handshake]  send 0x50", fwu_, &send_c, 1, Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
                    if (res != Q_MSG_OK)
                    break;

                    tempc = 0;
                    uart_io->xRead(&tempc, 1, c_timeout, &fwu_->uart_t_.io_object_);
                    if (tempc == 0xAF)
                    {
                        send_c = 0x05;
                        WRITE_WITH_CHECK(Q_FW_END "[handshake]  send 0x05",fwu_, &send_c, 1, Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
                        if (res != Q_MSG_OK)
                        break;

                        tempc = 0;
                        uart_io->xRead(&tempc, 1, c_timeout, &fwu_->uart_t_.io_object_);
                        if (tempc == 0xFA)
                        {
                            memset(progress_c, 0, 200);
                            q_progress2str(1.0, '#', progress_c, 200, "[handshake]");
                            q_print(NULL, "\r", fwu_, Q_MSG_INFO, progress_c);
                            q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_INFO, "[handshake] Success");
                            res = Q_MSG_OK;
                            break;
                        }
                        else
                        {
                            q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
                                         "[handshake] send 0x05 receive:0x%X not receive 0xFA ", tempc);
                            res= Q_MSG_ERROR;
                            break;
                        }
                    }
                    else
                    {
                        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
                            "[handshake] send 0x50 receive:0x%X not receive 0xAF ", tempc);
                        res= Q_MSG_ERROR;
                        break;
                    }
                }
                else
                {
                    q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
                        "[handshake] send 0x0A receive:0x%X not receive 0xF5 ", tempc);
                    res= Q_MSG_ERROR;
                    break;
                }
            }
        }
    }
    if (total_ms >= timeout)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_TIMEOUT, "[handshake] timout!");
        res = Q_MSG_TIMEOUT;
    }
    return res;
}

static int disable_wdt(fwu_0* _fw_0, int wdt_register_add)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[Disable Module WDT] Start...");
    int res = Q_MSG_OK;
    char send_c = 0xD2;
    char temp_c = 0;
    int c_timeout = fwu_->common_timeout;
    //# s 0xD2 r 0xD2
    WRITE_WITH_CHECK("[Disable Module WDT]  send 0xD2",fwu_, &send_c, 1, Q_WMAX_TIMEOUT, Q_FW_FALSE, &res)
    if (res != Q_MSG_OK)
    return res;
    uart_io->xRead(&temp_c, 1, c_timeout, &fwu_->uart_t_.io_object_);
    if (temp_c != 0xD2)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Disable Module WDT] send 0xD2 receive:0x%X not receive 0xD2", temp_c);
        res = Q_MSG_ERROR;
        return res;
    }
    //# s WDT address r WDT address
    char tempcs[100] = {0};
    snprintf(tempcs, 100, "[Disable Module WDT]  send WDT register address: 0x%X", wdt_register_add);

    char send_cs[sizeof(wdt_register_add)] = {0};
    q_it2str(q_is_bigend(), fwu_->is_bigend, &wdt_register_add, sizeof(wdt_register_add), send_cs);
    WRITE_WITH_CHECK(tempcs,fwu_, send_cs, sizeof(send_cs), Q_WMAX_TIMEOUT, Q_FW_FALSE, &res)
    if (res != Q_MSG_OK)
    return res;

    char read_cs[sizeof(wdt_register_add)] = {0};
    uart_io->xRead(read_cs, sizeof(wdt_register_add), c_timeout, &fwu_->uart_t_.io_object_);
    int receive = q_cs2it(read_cs, sizeof(wdt_register_add), fwu_->is_bigend);
    if (receive != wdt_register_add)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Disable Module WDT] send 0x%X receive:0x%X not 0x%X", wdt_register_add, receive, wdt_register_add);
        res = Q_MSG_ERROR;
        return res;
    }
    //# s 0x00000001 r 0x00000001 r brom_status
    int send_ = 0x00000001;
    memset(send_cs, 0, sizeof(wdt_register_add));
    q_it2str(q_is_bigend(), fwu_->is_bigend, &send_, sizeof(send_), send_cs);
    WRITE_WITH_CHECK("[Disable Module WDT]  send 0x00000001 ",fwu_, send_cs, sizeof(send_cs), Q_WMAX_TIMEOUT, Q_FW_FALSE, &res)
    if (res != Q_MSG_OK)
    return res;

    memset(read_cs, 0, sizeof(read_cs));
    uart_io->xRead(read_cs, sizeof(read_cs), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(read_cs, sizeof(read_cs), fwu_->is_bigend);
    if (receive != send_)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Disable Module WDT] send 0x%X receive:0x%X not 0x%X", send_, receive, send_);
        res = Q_MSG_ERROR;
        return res;
    }
    short brom_error = 0x1000;
    char  temp_read2[2] = { 0 };
    uart_io->xRead(temp_read2, sizeof(temp_read2), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(temp_read2, sizeof(temp_read2), fwu_->is_bigend);
    if (receive >=brom_error)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Disable Module WDT] send 0x%X receive brom_status:0x%X greater than brom_error 0x%X", send_, receive, brom_error);
        res = Q_MSG_ERROR;
        return res;
    }
    //# s WDT value 0x0010 r WDT value r brom_status
    short send_short = 0x0010;
    char  read_short[2] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend, &send_short, sizeof(send_short), read_short);
    WRITE_WITH_CHECK("[Disable Module WDT]  send 0x0010 ",fwu_, read_short, sizeof(read_short), Q_WMAX_TIMEOUT, Q_FW_FALSE, &res)
    if (res != Q_MSG_OK)
    return res;

    memset(read_short, 0, sizeof(read_short));
    uart_io->xRead(read_short, sizeof(read_short), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(read_short, sizeof(read_short), fwu_->is_bigend);
    if (receive != send_short)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Disable Module WDT] send 0x%X receive:0x%X not 0x%X", send_short, receive, send_short);
        res = Q_MSG_ERROR;
        return res;
    }
    memset(temp_read2, 0, sizeof(temp_read2));
    uart_io->xRead(temp_read2, sizeof(temp_read2), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(temp_read2, sizeof(temp_read2), fwu_->is_bigend);
    if (receive >=brom_error )
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Disable Module WDT] send 0x%X receive brom_status:0x%X greater than brom_error 0x%X", send_short, receive, brom_error);
        res = Q_MSG_ERROR;
        return res;
    }

    return res;
}

static unsigned short da_compute_checksum(unsigned char* buf, unsigned int buf_len)
{
    unsigned short  checksum = 0;
    if (buf == NULL || buf_len == 0) {
        return 0;
    }
    int i = 0;
    for (i = 0; i < buf_len / 2; i++) {
        checksum ^= *(unsigned short*)(buf + i * 2);
    }
    if ((buf_len % 2) == 1) {
        checksum ^= buf[i * 2];
    }
    return checksum;
}
//uint16_t local_check_sum ^= DA_compute_checksum(data_buf, len);

static int send_da(fwu_0* _fw_0, char* da_data, int da_size)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[Send DA File to Module] Start...");
    int res = Q_MSG_OK;
    int c_timeout = fwu_->common_timeout;
    //# send 0xD7 r 0xD7
    char send_c = 0xD7;
    char read_c = 0;
    WRITE_WITH_CHECK("[Send DA File to Module] Send 0xD7",fwu_, &send_c, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    uart_io->xRead(&read_c, 1, c_timeout, &fwu_->uart_t_.io_object_);
    if (read_c != send_c)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Send DA File to Module] send 0xD7 receive:0x%X not receive 0xD7 ", read_c);
        res = Q_MSG_ERROR;
        return res;
    }
    // send da startaddress r da start address
    int da_start_address = 0x04204000;
    char da_start_ad_sc[4] = { 0 };
    char da_start_ad_rc[4] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend, 
                 &da_start_address, sizeof(da_start_address),
                 da_start_ad_rc);
    WRITE_WITH_CHECK("[Send DA File to Module] Send  da start address 0x04204000",fwu_, da_start_ad_rc, sizeof(da_start_ad_rc),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    memset(da_start_ad_rc, 0, sizeof(da_start_ad_rc));
    uart_io->xRead(da_start_ad_rc, sizeof(da_start_ad_rc), c_timeout, &fwu_->uart_t_.io_object_);
    int receive = q_cs2it(da_start_ad_rc, sizeof(da_start_ad_rc), fwu_->is_bigend);
    if (receive != da_start_address)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Send DA File to Module] send 0x04204000 receive:0x%X not receive 0x04204000 ", receive);
        res = Q_MSG_ERROR;
        return res;
    }
    //send da length r da length
    int da_length = da_size;
    char da_length_rc[4] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend,
        &da_length, sizeof(da_length),
        da_length_rc);

    char temp_length[100] = {0};
    snprintf(temp_length, 100, "[Send DA File to Module] Send  da start length :0x%X", da_length);
    WRITE_WITH_CHECK(temp_length,fwu_,
        da_length_rc, sizeof(da_length_rc),Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    memset(da_length_rc, 0, sizeof(da_length_rc));
    uart_io->xRead(da_length_rc, sizeof(da_length_rc), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(da_length_rc, sizeof(da_length_rc), fwu_->is_bigend);
    if (receive != da_length)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Send DA File to Module] send 0x%X receive:0x%X not receive 0x%X ", da_length, receive,da_length);
        res = Q_MSG_ERROR;
        return res;
    }
    //send 0x00000000 r 0x00000000 r brom_status
    int send_4 = 0x00000000;
    char send_4_rc[4] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend,
        &send_4, sizeof(send_4),
        send_4_rc);

    WRITE_WITH_CHECK("[Send DA File to Module] Send  0x00000000",fwu_,
        send_4_rc, sizeof(send_4_rc), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    memset(send_4_rc, 0, sizeof(send_4_rc));
    uart_io->xRead(send_4_rc, sizeof(send_4_rc), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(send_4_rc, sizeof(send_4_rc), fwu_->is_bigend);
    if (receive != send_4)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Send DA File to Module] send 0x%X receive:0x%X not receive 0x%X ", send_4, receive, send_4);
        res = Q_MSG_ERROR;
        return res;
    }
    short brom_error = 0x1000;
    char temp_read2[2] = { 0 };
    memset(temp_read2, 0, sizeof(temp_read2));
    uart_io->xRead(temp_read2, sizeof(temp_read2), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(temp_read2, sizeof(temp_read2), fwu_->is_bigend);
    if (receive >=brom_error)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Send DA File to Module] send 0x00000000 receive brom_status:0x%X greater than brom_error 0x%X", receive, brom_error);
        res = Q_MSG_ERROR;
        return res;
    }
    //send da date one package 1024bytes
    int remaind_size = da_size;
    char* send_da_ptr = da_data;
    int package_size = 1024;
    char progress_cs[200] = { 0 };
    while (remaind_size>0)
    {
        if (fwu_->stop == Q_FW_TRUE)
        {
            q_print(Q_FW_END "[Send DA File to Module] Loop send 0xA0 ", Q_FW_END, fwu_, Q_MSG_MANUAL_STOP, NULL);
            res = Q_MSG_MANUAL_STOP;
            break;
        }
        memset(progress_cs, 0, 200);
        q_progress2str((da_size-remaind_size) /(double) da_size, '#', progress_cs, 200, "[Send DA File to Module]");
        q_print(NULL, "\r", fwu_, Q_MSG_INFO, progress_cs);

        int send_size = remaind_size >= package_size ? package_size : remaind_size;
        WRITE_WITH_CHECK(Q_FW_END "[Send DA File to Module] Send DA Package",fwu_,
            send_da_ptr, send_size, Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
        send_da_ptr += send_size;
        remaind_size -= send_size;
        fwu_->xSleep(20);
    }
    q_print(NULL, NULL, fwu_, Q_MSG_ERROR, Q_FW_END);//add \r\n
    fwu_->xSleep(500);
    //#r brom checksum
    char bom_checksum[2] = { 0 };
    memset(bom_checksum, 0, sizeof(bom_checksum));
    uart_io->xRead(bom_checksum, sizeof(bom_checksum), c_timeout, &fwu_->uart_t_.io_object_);
        //caculate da checksum
    unsigned short c_da_checksum=  da_compute_checksum(da_data, da_size);
    unsigned short g_da_checksum =q_cs2it(bom_checksum, sizeof(bom_checksum), fwu_->is_bigend);
    if (c_da_checksum != g_da_checksum)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Send DA File to Module] checksum error receive checksum:0x%X calculate checksum 0x%X", c_da_checksum, g_da_checksum);
        res = Q_MSG_ERROR;
        return res;
    }
    //#r brom_status 
    memset(temp_read2, 0, sizeof(temp_read2));
    uart_io->xRead(temp_read2, sizeof(temp_read2), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(temp_read2, sizeof(temp_read2), fwu_->is_bigend);
    if (receive >= brom_error)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Send DA File to Module] [receive brom_status and 0x66] brom_status:0x%X greater than brom_error 0x%X", receive, brom_error);
        res = Q_MSG_ERROR;
        return res;
    }
    return res;
}

static int jump_da(fwu_0* _fw_0)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[Jump to DA] Start...");
    int res = Q_MSG_OK;
    int c_timeout = fwu_->common_timeout;

    //# s 0xD5 r 0xD5
    char send_c = 0xD5;
    char read_c = 0;
    WRITE_WITH_CHECK("[Jump to DA] Send 0xD5",fwu_, &send_c, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    uart_io->xRead(&read_c, 1, c_timeout, &fwu_->uart_t_.io_object_);
    if (read_c != send_c)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Jump to DA] send 0xD5 receive:0x%X not receive 0xD5 ", read_c);
        res = Q_MSG_ERROR;
        return res;
    }
    //# s DA start address r DA start address r brom status
    int send_4 = 0x04204000;
    char send_4_rc[4] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend,
        &send_4, sizeof(send_4),
        send_4_rc);

    WRITE_WITH_CHECK("[Jump to DA] Send DA start address  0x04204000",fwu_,
        send_4_rc, sizeof(send_4_rc), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    memset(send_4_rc, 0, sizeof(send_4_rc));
    uart_io->xRead(send_4_rc, sizeof(send_4_rc), c_timeout, &fwu_->uart_t_.io_object_);
    int receive = q_cs2it(send_4_rc, sizeof(send_4_rc), fwu_->is_bigend);
    if (receive != send_4)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Jump to DA] send 0x%X receive:0x%X not receive 0x%X ", send_4, receive, send_4);
        res = Q_MSG_ERROR;
        return res;
    }
    short brom_error = 0x1000;
    char temp_read2[2] = { 0 };
    memset(temp_read2, 0, sizeof(temp_read2));
    uart_io->xRead(temp_read2, sizeof(temp_read2), c_timeout, &fwu_->uart_t_.io_object_);
    receive = q_cs2it(temp_read2, sizeof(temp_read2), fwu_->is_bigend);
    if (receive >= brom_error)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[Jump to DA] receive brom_status:0x%X greater than brom_error 0x%X", receive, brom_error);
        res = Q_MSG_ERROR;
        return res;
    }
    return res;
}

static int sync_da(fwu_0* _fw_0, int baudrate)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[Synchronize with DA] Start...");
    int res = Q_MSG_OK;
    int c_timeout = fwu_->common_timeout;
    //#[0] r 0xC0
    char tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0xC0)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] receive:0x%X not receive 0xC0 ", tempc1);
        res = Q_MSG_ERROR;
    }
    //#[1] s 0x3F r 0x0C
     tempc1 = 0x3F;
    char read_temp1 = 0;
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x3F",fwu_, &tempc1, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    uart_io->xRead(&read_temp1, 1, c_timeout, &fwu_->uart_t_.io_object_);
    if (read_temp1 != 0x0C)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0x3F receive:0x%X not receive 0x0C ", read_temp1);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[2] s 0xF3 r 0x3F
    tempc1 = 0xF3;
    read_temp1 = 0;
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0xF3",fwu_, &tempc1, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    uart_io->xRead(&read_temp1, 1, c_timeout, &fwu_->uart_t_.io_object_);
    if (read_temp1 != 0x3F)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0xF3 receive:0x%X not receive 0x3F ", read_temp1);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[3] s 0xC0 r 0xF3
    tempc1 = 0xC0;
    read_temp1 = 0;
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0xC0",fwu_, &tempc1, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    uart_io->xRead(&read_temp1, 1, c_timeout, &fwu_->uart_t_.io_object_);
    if (read_temp1 != 0xF3)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0xC0 receive:0x%X not receive 0xF3 ", read_temp1);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[4] s 0x0C00 r 0x5A6969
    unsigned short temp_c2 = 0x0C00;
    char tempc2_[sizeof(temp_c2)] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend, &temp_c2, sizeof(temp_c2), tempc2_);
    char read_tempc3[3] = { 0 };
    char tempc3[] = { 0x5A ,0x69,0x69 };
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x0C00",fwu_, tempc2_, sizeof(tempc2_),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    uart_io->xRead(read_tempc3, sizeof(read_tempc3), c_timeout, &fwu_->uart_t_.io_object_);
    if (memcmp(read_tempc3, tempc3, sizeof(read_tempc3)))
    {
        char tempcs[100] = { 0 };
        q_str2hex(read_tempc3, sizeof(read_tempc3), tempcs);
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0x0C00 receive:%s not receive 0x5A6969 ", tempcs);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[5] s 0x5A00 r 0x69
    temp_c2 = 0x5A00;
    memset(tempc2_, 0, sizeof(tempc2_));
    q_it2str(q_is_bigend(), fwu_->is_bigend, &temp_c2, sizeof(temp_c2), tempc2_);
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x5A00",fwu_, tempc2_, sizeof(tempc2_),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    read_temp1 = 0;
    uart_io->xRead(&read_temp1, sizeof(read_temp1), c_timeout, &fwu_->uart_t_.io_object_);
    if (read_temp1 != 0x69)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0x5A00 receive:0x%X not receive 0x69 ", read_temp1);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[6] s 0x5A r 0x69
    tempc1 = 0x5A;
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x5A",fwu_, &tempc1, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    read_temp1 = 0;
    uart_io->xRead(&read_temp1, sizeof(read_temp1), c_timeout, &fwu_->uart_t_.io_object_);
    if (read_temp1 != 0x69)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0x5A receive:0x%X not receive 0x69 ", read_temp1);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[7] s 0x5A 0xC0 r 0xC0
   tempc1  = 0x5A;
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x5A",fwu_, &tempc1, sizeof(tempc1),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    tempc1 = 0xC0;
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0xC0 after send 0x5A",fwu_, &tempc1, sizeof(tempc1),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    read_temp1 = 0;
    //switch baudrate
    fwu_->uart_t_.xSetBaudRate(baudrate, &fwu_->uart_t_);
    uart_io->xRead(&read_temp1, sizeof(read_temp1), c_timeout, &fwu_->uart_t_.io_object_);
    if (read_temp1 != 0xC0)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0x5A 0xC0 receive:0x%X not receive 0xC0 ", read_temp1);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[8] s 0x5A r 0x5A69
    tempc1 = 0x5A;
    char read_tempc2[2] = { 0 };
    char tempc2[] = { 0x5A ,0x69};
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x5A",fwu_, &tempc1, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    uart_io->xRead(read_tempc2, sizeof(read_tempc2), c_timeout, &fwu_->uart_t_.io_object_);
    if (memcmp(read_tempc2, tempc2, sizeof(read_tempc2)))
    {
        char tempcs[100] = { 0 };
        q_str2hex(read_tempc2, sizeof(read_tempc2), tempcs);
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] send 0x5A receive:%s not receive 0x5A69 ", tempcs);
        res = Q_MSG_ERROR;
        return res;
    }
    //#[9] s 0x5A receive 
    WRITE_WITH_CHECK("[Synchronize with DA] Send 0x5A get flash INFO",fwu_, &tempc1, 1,
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
       //[flash Manufacturer ] 2bytes 
    memset(read_tempc2, 0, sizeof(read_tempc2));
    uart_io->xRead(read_tempc2, sizeof(read_tempc2), c_timeout, &fwu_->uart_t_.io_object_);
    short flash_Manufacturer =q_cs2it(read_tempc2, sizeof(read_tempc2), fwu_->is_bigend);
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO,
        "[Synchronize with DA] [flash Manufacturer ] 0x%X ", flash_Manufacturer);
        //[flash Device ID1 ] 2bytes 
    memset(read_tempc2, 0, sizeof(read_tempc2));
    uart_io->xRead(read_tempc2, sizeof(read_tempc2), c_timeout, &fwu_->uart_t_.io_object_);
    flash_Manufacturer = q_cs2it(read_tempc2, sizeof(read_tempc2), fwu_->is_bigend);
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO,
        "[Synchronize with DA] [flash Device ID1 ] 0x%X ", flash_Manufacturer);
    //[flash Device ID2 ] 2bytes 
    memset(read_tempc2, 0, sizeof(read_tempc2));
    uart_io->xRead(read_tempc2, sizeof(read_tempc2), c_timeout, &fwu_->uart_t_.io_object_);
    flash_Manufacturer = q_cs2it(read_tempc2, sizeof(read_tempc2), fwu_->is_bigend);
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO,
        "[Synchronize with DA] [flash Device ID2 ] 0x%X ", flash_Manufacturer);
    //[(Flash Mount Status ] 4bytes 
    char tempc4[4] = { 0 };
    uart_io->xRead(tempc4, sizeof(tempc4), c_timeout, &fwu_->uart_t_.io_object_);
    flash_Manufacturer = q_cs2it(tempc4, sizeof(tempc4), fwu_->is_bigend);
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO,
        "[Synchronize with DA] [Flash Mount Status ] 0x%X ", flash_Manufacturer);
    //[(Flash Start Address ] 4bytes 
    memset(tempc4, 0, sizeof(tempc4));
    uart_io->xRead(tempc4, sizeof(tempc4), c_timeout, &fwu_->uart_t_.io_object_);
    flash_Manufacturer = q_cs2it(tempc4, sizeof(tempc4), fwu_->is_bigend);
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO,
        "[Synchronize with DA] [Flash Start Address ] 0x%X ", flash_Manufacturer);
    //[(Flash Size ] 4bytes 
    memset(tempc4, 0, sizeof(tempc4));
    uart_io->xRead(tempc4, sizeof(tempc4), c_timeout, &fwu_->uart_t_.io_object_);
    flash_Manufacturer = q_cs2it(tempc4, sizeof(tempc4), fwu_->is_bigend);
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO,
        "[Synchronize with DA] [Flash Size ] 0x%X ", flash_Manufacturer);

    //#[]r 0x5A s 0x5A
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[Synchronize with DA] end receive:0x%X not receive 0x5A ", tempc1);
        res = Q_MSG_ERROR;
        return res;
    }

    tempc1 = 0x5A;
    WRITE_WITH_CHECK("[Synchronize with DA] end Send 0x5A",fwu_, &tempc1, 1,
    Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    return res;
}

static int format_flash(fwu_0* _fw_0, int begin_address, int format_length)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[format flash] Start...");
    int res = Q_MSG_OK;
    int c_timeout = fwu_->common_timeout;
    //# [] s 0xD400 s Format Physical Address s Format Length r 0x5A5A r 0x00000BCD
    char  temp_c2[] = { 0xD4,0x00 };
    WRITE_WITH_CHECK("[format flash] Send 0xD400",fwu_, temp_c2, sizeof(temp_c2),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    char  temp_c4[sizeof(begin_address)] = { 0 };
    q_it2str(q_is_bigend(), fwu_->is_bigend, &begin_address, sizeof(begin_address), temp_c4);
    WRITE_WITH_CHECK("[format flash] Send Format Physical Address",fwu_, temp_c4, sizeof(temp_c4),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    memset(temp_c4, 0, sizeof(temp_c4));
    q_it2str(q_is_bigend(), fwu_->is_bigend, &format_length, sizeof(format_length), temp_c4);
    WRITE_WITH_CHECK("[format flash] Send Format Length",fwu_, temp_c4, sizeof(temp_c4),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    // receive 0x5A
    char tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[format flash] Send Format Length 0x%X receive:0x%X not receive 0x5A ", format_length, tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[format flash] Send Format Length 0x%X receive :0x%X not receive sencond 0x5A ", format_length, tempc1);
        res = Q_MSG_ERROR;
        memset(temp_c2, 0, sizeof(temp_c2));
        uart_io->xRead(temp_c2, sizeof(temp_c2), c_timeout, &fwu_->uart_t_.io_object_);
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[format flash] Send Format Length 0x%X receive error code :0x%X ", format_length, temp_c2);
        return res;
    }
    memset(temp_c4, 0, sizeof(temp_c4));
    uart_io->xRead(temp_c4, sizeof(temp_c4), c_timeout, &fwu_->uart_t_.io_object_);
    int receive= q_cs2it(temp_c4, sizeof(temp_c4), fwu_->is_bigend);
    if (receive != 0x00000BCD)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[format flash]  receive:0x%X not receive 0x00000BCD ", format_length, receive);
        res = Q_MSG_ERROR;
        return res;
    }
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    char progress_cs[200] = { 0 };
    memset(progress_cs, 0, 200);
    q_progress2str(tempc1/100.0, '#', progress_cs, 200, "[format flash]");
    q_print(NULL, "\r", fwu_, Q_MSG_INFO, progress_cs);
    int receive_ = 0x00000BCD;
    do {

        receive_ = tempc1 == 0x63 ? 0x00000000 : 0x00000BCD;
        tempc1 = 0x5A;
        WRITE_WITH_CHECK(Q_FW_END "[format flash] send 0x5A ",fwu_, temp_c4, sizeof(temp_c4),
            Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
        memset(temp_c4, 0, sizeof(temp_c4));
        uart_io->xRead(temp_c4, sizeof(temp_c4), c_timeout, &fwu_->uart_t_.io_object_);
         receive = q_cs2it(temp_c4, sizeof(temp_c4), fwu_->is_bigend);
        if (receive != receive_)
        {
            q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR,
                "[format flash]  receive:0x%X not receive 0x%X  ", receive, receive_);
            res = Q_MSG_ERROR;
            return res;
        }

        tempc1 = 0;
        uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
        memset(progress_cs, 0, 200);
        q_progress2str(tempc1/100.0, '#', progress_cs, 200, "[format flash]");
        q_print( NULL, "\r", fwu_, Q_MSG_INFO, progress_cs);
    } while (tempc1 < 0x64);

    tempc1 = 0x5A;
    WRITE_WITH_CHECK(Q_FW_END "[format flash] end send 0x5A ",fwu_, &tempc1, sizeof(tempc1),
        Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR,
            "[format flash] end receive:0x%X not receive 0x5A ", tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    return res;
}
static unsigned int crc_compute_simple_checksum(unsigned char* buf, unsigned int buf_len) 
{
    unsigned int checksum = 0;
    if (buf == NULL || buf_len == 0) {
        return 0;
    }
    for (int i = 0; i < buf_len; i++) {
        checksum += *(buf + i);
    }
    return checksum;
}
//uint32_t packet_checksum = CRC_compute_simple_checksum(buf, buf_len);
//Total Checksum Code of the FW file :(IIC no need)
//uint32_t Total_FW_checksum += packet_checksum;

static int send_fw(fwu_0* _fw_0, fwu_0_rom* rom)
{
    GET_INIT(_fw_0)
    q_print(NULL, Q_FW_END, fwu_, Q_MSG_INFO, "[ Send FW File :%s] Start...", rom->name);
    int res = Q_MSG_OK;
    int c_timeout = fwu_->common_timeout;

    char tempc1 = 0xB2;
    WRITE_WITH_CHECK(Q_FW_END "[ Send FW File] Send 0xB2",fwu_,
        tempc1, sizeof(tempc1), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    int fw_address = rom->begin_address;
    int fw_length = rom->firewaresize;
    int fw_package = 0x00001000;
    char tempc4[4] = {0};

    char tempcs[200] = { 0 };
    snprintf(tempcs, sizeof(tempcs), "[ Send FW File] FW Flash Address 0x%X", fw_address);
    q_it2str(q_is_bigend(), fwu_->is_bigend, &fw_address, sizeof(fw_address), tempc4);
    WRITE_WITH_CHECK(tempcs,fwu_,
        tempc4, sizeof(tempc4), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    memset(tempcs, 0, sizeof(tempcs));
    snprintf(tempcs, sizeof(tempcs), "[ Send FW File] FW Total Length 0x%X", fw_length);
    memset(tempc4, 0, sizeof(tempc4));
    q_it2str(q_is_bigend(), fwu_->is_bigend, &fw_length, sizeof(fw_length), tempc4);
    WRITE_WITH_CHECK(tempcs,fwu_,
        tempc4, sizeof(tempc4), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }

    memset(tempcs, 0, sizeof(tempcs));
    snprintf(tempcs, sizeof(tempcs), "[ Send FW File] FW Packet Length 0x%X", fw_package);
    memset(tempc4, 0, sizeof(tempc4));
    q_it2str(q_is_bigend(), fwu_->is_bigend, &fw_package, sizeof(fw_package), tempc4);
    WRITE_WITH_CHECK(tempcs,fwu_,
        tempc4, sizeof(tempc4), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[ Send FW File]  receive :0x%X not equal first 0x5A ", tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(NULL, Q_FW_END, fwu_, Q_MSG_ERROR, "[ Send FW File]  receive :0x%X not equal second 0x5A", tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    //send fw packet
    int   remain_size  = rom->firewaresize;
    int   total_size   = rom->firewaresize;
    int   package_size = fw_package;
    char* fw_ptr = rom->fireware;
    char* temp_ptr = NULL;
    int total_crc = 0;
    char progress_cs[200] = { 0 };
    do
    {
        if (remain_size < package_size)
        {
            temp_ptr = (char*)malloc(package_size);
            memset(temp_ptr, 0xFF, package_size);
            memcpy(temp_ptr, fw_ptr, remain_size);
            fw_ptr = temp_ptr;
        }
        memset(progress_cs, 0, 200);
        q_progress2str((total_size- remain_size)/ (double)total_size, '#', progress_cs, 200, "[ Send FW File]");
        q_print(NULL, "\r", fwu_, Q_MSG_INFO, progress_cs);

        WRITE_WITH_CHECK(Q_FW_END "[ Send FW File] send Packet",fwu_,
            fw_ptr, package_size, Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (temp_ptr)
        {
            free(temp_ptr);
            temp_ptr = NULL;
        }
        if (res != Q_MSG_OK)
        {
            return res;
        }

        int crc=   crc_compute_simple_checksum(fw_ptr, package_size);
        total_crc += crc;
        memset(tempc4, 0, sizeof(tempc4));
        q_it2str(q_is_bigend(), fwu_->is_bigend, &crc, sizeof(crc), tempc4);
        WRITE_WITH_CHECK(Q_FW_END "[ Send FW File] send Packet crc",fwu_,
            tempc4, sizeof(tempc4), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
        if (res != Q_MSG_OK)
        {
            return res;
        }
        tempc1 = 0;
        uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
        if (tempc1 != 0x69)
        {
            q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR, "[ Send FW File]  receive :0x%X not equal second 0x69", tempc1);
            res = Q_MSG_ERROR;
            return res;
        }
        remain_size -= package_size;
        fw_ptr += package_size;
    } while (remain_size);
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR, "[ Send FW File] end receive :0x%X not equal  0x5A", tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    memset(tempc4, 0, sizeof(tempc4));
    q_it2str(q_is_bigend(), fwu_->is_bigend, &total_crc, sizeof(total_crc), tempc4);
    WRITE_WITH_CHECK(Q_FW_END "[ Send FW File] send Total FW Checksum",fwu_,
        tempc4, sizeof(tempc4), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    tempc1 = 0;
    uart_io->xRead(&tempc1, sizeof(tempc1), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc1 != 0x5A)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR, "[ Send FW File]  receive :0x%X not equal  0x5A after send total crc", tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    tempc1 = 0xA5;
    if (rom->is_bootloader == Q_FW_TRUE)
    {
        tempc1 = 0x5A;
    }
    char tempsed[100] = { 0 };
    snprintf(tempsed, 100, "[ Send FW File] send (0x5A or 0xA5):0x%X", tempc1);
    WRITE_WITH_CHECK(tempsed,fwu_,
        &tempc1, sizeof(tempc1), Q_WMAX_TIMEOUT, Q_FW_TRUE, &res)
    if (res != Q_MSG_OK)
    {
        return res;
    }
    char tempc2 = 0;
    uart_io->xRead(&tempc2, sizeof(tempc2), c_timeout, &fwu_->uart_t_.io_object_);
    if (tempc2 != 0x5A)
    {
        q_print(Q_FW_END, Q_FW_END, fwu_, Q_MSG_ERROR, "[ Send FW File] end receive :0x%X not equal  0x5A after send:0x%X", tempc2, tempc1);
        res = Q_MSG_ERROR;
        return res;
    }
    return Q_FW_TRUE;
}


void new_fw0(fwu_0** fwu_0_)
{
    fwu_0* fwu_0_copy = malloc(sizeof(fwu_0));
}
int init_fw0(fwu_0* q_fw)
{
    q_fw->xHandshake = handshake;
    q_fw->xDisable_wdt = disable_wdt;
    q_fw->xSend_da = send_da;
    q_fw->xJump_da = jump_da;
    q_fw->xSync_da = sync_da;
    q_fw->xFormat_flash = format_flash;
    q_fw->xSend_fw = send_fw;
}
