#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
	clist->num = 0;
	char *cmd_array[CMD_MAX];
    	int count = 0;
    	char *token = strtok(cmd_line, PIPE_STRING);
    	while (!(token == NULL)) {
		while (*token && isspace((unsigned char)*token))
	    		token++;
		char *end = token + strlen(token) - 1;
		while (end > token && isspace((unsigned char)*end))
		{
	    		*end = '\0';
	    		end--;
		}
		if (strlen(token) > 0) {
	    		if (count >= CMD_MAX)
				return ERR_TOO_MANY_COMMANDS;
	    		cmd_array[count++] = token;
		}
		token = strtok(NULL, PIPE_STRING);
    	}
    	for (int i = 0; i < count; i++) {

		char tmp[SH_CMD_MAX];
		strncpy(tmp, cmd_array[i], SH_CMD_MAX);
		tmp[SH_CMD_MAX - 1] = '\0';
		char *word = strtok(tmp, " ");

		if (word == NULL)
	    		continue;

		if (strlen(word) >= EXE_MAX)
	    		return ERR_CMD_OR_ARGS_TOO_BIG;

		strcpy(clist->commands[clist->num].exe, word);
		clist->commands[clist->num].args[0] = '\0';

		word = strtok(NULL, " ");
		while (!(word == NULL)){
	    		size_t current_length = strlen(clist->commands[clist->num].args);
	    		size_t token_length = strlen(word);

	    		if (current_length + token_length >= ARG_MAX)
				return ERR_CMD_OR_ARGS_TOO_BIG;

	    		for (size_t j = 0; j < token_length; j++)
				clist->commands[clist->num].args[current_length + j] = word[j];

	    		clist->commands[clist->num].args[current_length + token_length] = '\0';
	    		word = strtok(NULL, " ");
		}
		clist->num++;
    	}
    	return OK;
}
