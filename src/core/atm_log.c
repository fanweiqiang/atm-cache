#include <atm_core.h>
/*
 * Private
 * */
static void 
atm_log_rout_raw(atm_uint_t lv, char *msg);


static const char * ATM_LOG_LV_ENUM[] = {
    "OFF",
    "ALL",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
};


static const char *
atm_log_lvstr(atm_uint_t level) 
{
    atm_uint_t lvs_size = 0;

    lvs_size = sizeof(ATM_LOG_LV_ENUM)/sizeof(char *);
    if (level >= lvs_size)
        level = lvs_size - 1;
    return ATM_LOG_LV_ENUM[level];
}


static void 
atm_log_rout_raw(atm_uint_t lv, char *msg) 
{
    if (msg != NULL) {
        printf("[%s]:%s\n",atm_log_lvstr(lv),msg);
    } else {
        printf("Invalid msg for logger\n");
    }
}


/* ---------------------IMPLEMENTATIONS--------------------------- */
/*
 * Public
 * */
void 
atm_log(char *fmt, ...) 
{
    atm_uint_t size = ATM_LOG_MAXLEN;
    char msg[size];
    memset(msg, 0, size);

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, size, fmt, args);
    va_end(args);

    atm_log_rout_raw(ATM_LOG_INFO, msg);
}


void 
atm_log_rout(atm_uint_t level, char *fmt, ...) 
{
    atm_uint_t size = ATM_LOG_MAXLEN;
    char msg[size];
    memset(msg, 0, size);

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg, size, fmt, args);
    va_end(args);

    atm_log_rout_raw(level, msg);
}
