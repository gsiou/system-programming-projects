#include "types.h"
#include "utils.h"

void * worker_job(void * arg);

void * master_job(void * arg);

void execute_cmd(struct server_command scmd, char *response);

#define UKNOWN              "Error. Uknown command"
#define SYNTAX              "Error. Invalid command syntax"
#define CRT_DONE            "Success. Account creation (%s:%d)"
#define CRT_FAIL            "Error. Account creation failed (%s:%s)"
#define PRINT_DONE          "Success. Balance (%s:%d)"
#define PRINT_FAIL          "Error. Balance (%s)"
#define TRANSFER_DONE       "Success. Transfer addition (%s:%s:%s:%d)"
#define TRANSFER_FAIL       "Error. Transfer addition failed (%s:%s:%s:%d)"
#define PRINT_MULTI_DONE    "Success. Multi-Balance %s"
#define PRINT_MULTI_FAIL    "Error. Multi-Balance %s"
#define TRANSFER_MULTI_DONE "Success. Multi-Transfer addition %s"
#define TRANSFER_MULTI_FAIL "Error. Multi-Transfer addition failed %s"
