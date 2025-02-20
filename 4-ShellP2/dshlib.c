#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"
/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */


static int last_rc = 0;

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {

    	cmd_buff->argc = 0;
    	while (*cmd_line && isspace((unsigned char)*cmd_line))
		cmd_line++;

    	if (*cmd_line == '\0')
		return WARN_NO_CMDS;
    	
    	char *end = cmd_line + strlen(cmd_line) - 1;
    	while (end > cmd_line && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
    	}

    	char *p = cmd_line;
    	while (!( *p == '\0' )) {

		while (!( *p == '\0' ) && isspace((unsigned char)*p))
	    		p++;

		if (*p == '\0')
	    		break;

		if (*p == '\"') {

	    		p++;
	    		char *start = p;
	    		while (!( *p == '\0' ) && !(*p == '\"'))
				p++;

	    		if (*p == '\"') {

				*p = '\0';
				p++;
	    		}

	    		if (cmd_buff->argc < CMD_ARGV_MAX - 1)
				cmd_buff->argv[cmd_buff->argc++] = start;
	    		else
				return ERR_CMD_OR_ARGS_TOO_BIG;

		} else {

	    		char *start = p;
	    		while (!( *p == '\0' ) && !isspace((unsigned char)*p))
				p++;

	    		if (!( *p == '\0' )) {

				*p = '\0';
				p++;
	    		}

	    		if (cmd_buff->argc < CMD_ARGV_MAX - 1)
				cmd_buff->argv[cmd_buff->argc++] = start;
	    		else
				return ERR_CMD_OR_ARGS_TOO_BIG;
		}
    	}

    	cmd_buff->argv[cmd_buff->argc] = NULL;
    	return OK;
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {

    	if (cmd->argc > 0 && strcmp(cmd->argv[0], "rc") == 0) {
		printf("%d\n", last_rc);
		return BI_EXECUTED;
    	}

    	if (cmd->argc > 0 && strcmp(cmd->argv[0], "cd") == 0) {

		switch (cmd->argc) {

	    		case 1:
				return BI_EXECUTED;

	    		case 2:
				if (!(chdir(cmd->argv[1]) == 0))
		    			PRINT_CD_ERROR(cmd->argv[1]);
				return BI_EXECUTED;

	    		default:
				PRINT_CD_TOO_MANY_ARGS();
				return BI_EXECUTED;

		}

    	}

    	return BI_NOT_BI;
}

int exec_local_cmd_loop() {

    	char cmd_line[SH_CMD_MAX];
    	cmd_buff_t cmd;
    	while (1) {

		printf("%s", SH_PROMPT);
		if (fgets(cmd_line, sizeof(cmd_line), stdin) == NULL) {

	    		printf("\n");
	    		break;
		}

		cmd_line[strcspn(cmd_line, "\n")] = '\0';
		int parse_status = build_cmd_buff(cmd_line, &cmd);
		if (parse_status != OK) {

	    		if (parse_status == WARN_NO_CMDS)
				printf(CMD_WARN_NO_CMD);

	    		continue;
		}
		
		if (strcmp(cmd.argv[0], EXIT_CMD) == 0)
	    		break;
		
		if (exec_built_in_cmd(&cmd) == BI_EXECUTED)
	    		continue;
		
		pid_t pid = fork();
		if (pid < 0) {
	    		PRINT_FORK_ERROR();
	    		continue;
		} else if (pid == 0) {

	    		execvp(cmd.argv[0], cmd.argv);
	    		PRINT_EXEC_ERROR();
	    		exit(ERR_EXEC_CMD);

		} else {
	    		int status;
	    		waitpid(pid, &status, 0);
	    		last_rc = WEXITSTATUS(status);
		}

    	}

    	return OK;
}
