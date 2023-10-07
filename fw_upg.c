#include "fw_upg.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

void q_progress2str(double progress,char progress_c, 
                        char* progress_str, int size,
                        const char *heads)
{
    char progress_cs[200];
    memset(progress_cs, 0, 200);
    int progress_ = abs(progress)>1?1: progress * 100;
    int i = 0;
    for (; i < progress_/4; i++)
    {
        progress_cs[i] = progress_c;
    }
    snprintf(progress_str, size, "%s Progress:[%s] %d/100 ", heads, progress_cs, progress_);
}
int q_is_bigend(void)
{
   union test
   {
       char ua;
       short ub;
   };
   //big   l (h)
   //small l (l)
   union test ts;
   ts.ub = 1;
   if (ts.ua == 1)
   {
       return Q_FW_FALSE;
   }
   return Q_FW_TRUE;
}

void q_it2str(int send_end, int receive_end,
    char* send_data, int send_data_size,
    char* receive_data)
{
    memset(receive_data, 0, send_data_size);
    if (send_end == receive_end)
    {
        memcpy(receive_data, send_data, send_data_size);
        return;
    }
    for (int i = 0; i < send_data_size; i++)
    {
        receive_data[i] = send_data[send_data_size - i - 1];
    }
}

void q_str2hex(char* str, int strlen, char* hex)
{
    int i = 0;
    char newchar[100] = { 0 };
    for (i = 0; i < strlen; i++)
    {
        sprintf(newchar, "%02X ", str[i]);
        strcat(hex, newchar);
    }
}
int q_cs2it(char* str, int len, int big_end)
{
    if (len % 2 > 0) { return 0; }

    int temp = 0;
    char* tempc = (char*)&temp;
    if (big_end == 1)
    {
        for (int i = 4-len; i < len; i++)
        {
            tempc[i] = str[len - i - 1];
        }
    }
    else
    {
        for (int i = 4 - len; i < len; i++)
        {
            tempc[i] = str[i];
        }
    }
    return temp;
}

void q_print(const char* head, const char* end, fwu* fwu_, int type, const char* fmt, ...)
{

    char buffer[1024] = { 0 };

    if (head)
        strcat(buffer, head);
    //time
    fw_time time = fwu_->xTimeget();
    char t[100] = { 0 };
    fwu_->xTime2str(time, t, 100);
    snprintf(buffer+strlen(buffer), sizeof(buffer), "[%s]", t);

    switch (type)
    {
        case Q_MSG_DEBUG:
            strcat(buffer, "[DEBUG] ");
            break;
        case Q_MSG_INFO:
            strcat(buffer, "[INFO] ");
            break;
        case Q_MSG_WARNING:
            strcat(buffer, "[WARNING] ");
            break;
        case Q_MSG_ERROR:
            strcat(buffer, "[ERROR] ");
            break;
        case Q_MSG_PROGRESS:
            strcat(buffer, "[INFO]");
            break;
        case Q_MSG_OK:
            strcat(buffer, "[OK] ");
            break;
        case Q_MSG_UART_ERROR:
            strcat(buffer, "[ERROR] " );
            break;
        case Q_MSG_FILE_ERROR:
            strcat(buffer, "[ERROR] " );
            break;
        case Q_MSG_MANUAL_STOP:
            strcat(buffer, "[INFO]" );
            break;
        case Q_MSG_TIMEOUT:
            strcat(buffer, "[ERROR]");
            break;
        default:
            strcat(buffer, "");
            break;
    }
    va_list list;
    va_start(list, fmt);
    vsprintf(buffer +strlen(buffer), fmt, list);
    va_end(list);
    if(end)
    strcat(buffer, end);
    fwu_->xPrint(type, buffer );
}