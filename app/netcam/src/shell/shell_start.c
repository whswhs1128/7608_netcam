#include <stdio.h>
#include <stdlib.h>

#include "shell.h"
#include "adi_sys.h"
#include "shell_common.h"

//static int module_flag = 0x0;
//#define MODULE_SHELL_CLIENT_ENABLE      (0x1<<8)
//#define is_shell_client_enbale()    (module_flag&MODULE_SHELL_CLIENT_ENABLE)

int shell_initialize(void)
{
	int   shellflag = 0;
	//shellflag |= SHELL_SERVER_START;
	shellflag |= SHELL_SCREEN_START;
	//shellflag |= SHELL_CLIENT_START;
	shell_init(GADI_SYS_THREAD_PRIO_DEFAULT, shellflag);
	return 1;
}

void shell_main_task(void *data)
{  
	shell_initialize();
	ircut_register_testcase();
	wifi_register_testcase();
	isp_register_testcase();
    ptz_register_testcase();
	upgrade_register_testcase();
	eraseapp_register_testcase();
    ftp_register_testcase();
    bpt_register_testcase();
}


void shell_test_start(void)
{
    gadi_sys_thread_create(shell_main_task, NULL, GADI_SYS_THREAD_PRIO_DEFAULT,
        GADI_SYS_THREAD_STATCK_SIZE_DEFAULT, "ShellTask", NULL);

}

