#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "bplustree.h"

static const char *shortOptions = "hi:r:s:d";

static struct option longOptions[] =
{
    {"help",       0, NULL, 'h'},
    {"insert",     1, NULL, 'i'},
    {"remove",     1, NULL, 'r'},
    {"search",     1, NULL, 's'},
    {"dump",       0, NULL, 'd'},
    {0,            0, NULL, 0}
};
extern LOG_BPLUS_TREE_INFO_S g_stLogTree;
extern LOG_BPLUS_TREE_INFO_S g_stLogTmpTree;
static int handle_bpt_command(int argc, char* argv[]);

static void Usage(void)
{
    printf("Usage: bpt [option]\n");
    printf("    -h help.\n");
    printf("    -i insert key. e.g. i 1 4-7 9\n");
    printf("    -r remove key. e.g. r 1-100\n");
    printf("    -s search by key. e.g. s 41-60\n");
    printf("    -d dump the tree structure.\n");
}

int bpt_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "bpt",
        handle_bpt_command,
        "bpt command",
        "---------------------------------------------------------------------\n"
        "bpt -i \n"
        "   brief : bpt insert\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "bpt -r \n"
        "   brief : bpt remove\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "bpt -s \n"
        "   brief : bpt search\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "bpt -d \n"
        "   brief : bpt dump\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

static int handle_bpt_command(int argc, char* argv[])
{
	int ret = 0;
    int option_index;//ch;
    char ch;
    int n = 0;
    long test = 0;
    value_t *param_get;
    value_t param;
    memset(&param, 0, sizeof(param));
    /*change parameters when giving input options.*/
    optind = 1;
    while (1)
    {
        option_index = 0;
        ch = getopt_long(argc, argv, shortOptions, longOptions, &option_index);
		
        if (ch == 255)
    	{
            break;
    	}
        
        switch (ch)
        {
            case 'h':
            case '?':
                Usage();
                break;
            case 'd':
                bplus_tree_dump(g_stLogTree.pLogIndexTree);
                break;
            case 'i':
                n = atoi(optarg);
                param.u32Index = n;
                param.u32LogType = 1;                    
                sprintf(param.logContent, "%d", n);
                bplus_tree_put(g_stLogTree.pLogIndexTree, n, &param);
                bplus_refresh_boot(g_stLogTree.pLogIndexTree);
                break;
            case 'r':
                n = atoi(optarg);
                #if 0
                bplus_tree_put(g_stLogTree.pLogIndexTree, n, NULL);
                bplus_refresh_boot(g_stLogTree.pLogIndexTree);
                #else
                param_get = bplus_tree_get(g_stLogTree.pLogIndexTree, n);
                if(param_get != NULL)
                {
                    if(n == 1)
                    {
                        printf("1g_stLogTree===key:%d, u32CurIndex:%d, u32DelIndex:%d, u32TotalIndex:%d\n", n, param_get->u32CurIndex, param_get->u32DelIndex, param_get->u32TotalIndex);
                    }
                    else
                    {
                        printf("2g_stLogTmpTree====key:%d, index:%d, logtime:%d.%d, logContent:%s\n", n, param_get->u32Index, param_get->logTime, param_get->logTime_msec, param_get->logContent);
                    }
                }
                else
                    printf("1g_stLogTree====key:%d\n", n);
                #endif
                break;
            case 's':
                n = atoi(optarg);
                param_get = bplus_tree_get(g_stLogTmpTree.pLogIndexTree, n);
                if(param_get != NULL)
                {
                    if(n == 1)
                    {
                        printf("2g_stLogTmpTree===key:%d, u32CurIndex:%d, u32DelIndex:%d, u32TotalIndex:%d\n", n, param_get->u32CurIndex, param_get->u32DelIndex, param_get->u32TotalIndex);
                    }
                    else
                    {
                        printf("2g_stLogTmpTree====key:%d, index:%d, logtime:%d.%d, logContent:%s\n", n, param_get->u32Index, param_get->logTime, param_get->logTime_msec, param_get->logContent);
                    }
                }
                else
                    printf("2g_stLogTmpTree====key:%d\n", n);
                break;
            default:
                break;
        }
    }
exit:
    //bplus_tree_deinit(tree);
    optind = 1;
    return 0;
}
