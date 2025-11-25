#pragma once

#define PROG_MD_CLI     0
#define PROG_MD_SVR     1
#define DEF_PORT_NO     2080
#define FNAME_SZ        150
#define PROG_DEF_FNAME  "test.c"
#define PROG_DEF_SVR_ADDR   "127.0.0.1"

typedef struct prog_config{
    int     prog_mode;
    int     port_number;
    char    svr_ip_addr[16];
    char    file_name[128];
} prog_config;

typedef struct dp_pdu_ext {
    int     proto_ver;
    int     mtype;
    int     seqnum;
    int     dgram_sz;
    int     err_num;
} dp_pdu_ext;

typedef enum {
   FILE_CREATE,
   DATA,
   FILE_CLOSE
} FILE_MSG_TYPE;

typedef struct File_Transfer_PDU{
    FILE_MSG_TYPE fat;
    char payload[];
} File_Transfer_PDU;