/*
 * Filename
 *
 * Description
 *
 * Copyright (C) 2015 V. Atlidakis
 *
 * COMS W4187 Fall 2015, Columbia University
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"


int get_user(const char *line, char *user)
{
	int len;

	user[0] = '\0';
	/* parse user name field */
	for (len = 0; len < strlen(line) && line[len] != '.'; len++)
		;
	if (line[len] != '.')
		goto error;

	strncpy(user, line, len);
	user[len] = '\0';

	return len;
error:
	return -1;
}

int get_group(const char *line, char *group)
{
	int len;

	group[0]='\0';
	/* move past user name field */
	for (len = 0; len < strlen(line) && line[len] != '.'; len++)
		;
	if (line[len] != '.')
		goto error;
	line = line + len + 1;
	
	/* parse group name field */
	for (len = 0; len < strlen(line) && line[len] != ' '
	     && line[len] != '\n'; len++)
		;

	if (!len)
		goto error;


	strncpy(group, line, len);
	group[len] = '\0';

	return len;
error:
	return -1;
}

int get_filename(const char *line, char *filename)
{
	int len;

	filename[0] = '\0';
	/*
	 * move past user name and group name field and return if
	 * file name starting point not found
	 */
	for (len = 0; len < strlen(line); len++)
		if (line[len] == ' ')
			break;

	//printf("@@<%c>", line[len]);
	if (line[len] == '\0')
		return 0;

	//printf("line:<%s>\n", line);
	line = line + len + 1;
	//printf("line:<C:%c>\n", *line);

	/* strip possible spaces */
	while (*line == ' ')
		line++;

	/* remember, line contains \n  which doesn't count*/
	len = strlen(line);
	strncpy(filename, line, len-1);
	filename[len-1] = '\0';
	return len - 1;
}

/*
 * @tokenize - Parse line into args separated by delimiter(s)
 *
 * @line - The line to be parsed
 * @args - The args retrieved by the line
 * @delim - A set of delimiters
 *
 * An array, whose starting address is pointed by "*args",
 * is allocated to store any args found in line. This array
 * should by freed by the calling method.
 */
int tokenize(char *line, char ***args, char *delim)
{
	int i;
	int ntok;
	char *copy;

	/*
	 * strtok libc function modifies the initial buffer it is
	 * requested to tokenize. Thus, we make a copy of the
	 * initial buffer and use it to count number of args
	 * and in the next iteration then retrieve the args.
	 */
	if (line == NULL)
		return -1;
	copy = calloc(strlen(line) + 1, sizeof(char));
	if (copy == NULL) {
		perror("calloc");
		return -1;
	}
	strcpy(copy, line);
	if (strtok(copy, delim) == NULL)
		return -1;
	ntok = 1;
	while (strtok(NULL, delim) != NULL)
		ntok++;
	free(copy);
	copy = calloc(strlen(line) + 1, sizeof(char));
	if (copy == NULL) {
		perror("caloc");
		return -1;
	}
	strcpy(copy, line);
	*args = calloc(ntok + 1, sizeof(char *));
	if (*args == NULL) {
		perror("calloc");
		return -1;
	}
	**args = strtok(copy, delim);
	for (i = 1; i < ntok; i++)
		*(*args + i) = strtok(NULL, delim);
	*(*args + i) = NULL;

	return 0;
}
