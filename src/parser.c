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
	//printf("line:<%s>\n", line);

	/* remember, line contains \n  which doesn't count*/
	//if ((
	len = strlen(line);
	//) != 0) {
	strncpy(filename, line, len-1);
	filename[len-1] = '\0';
	//}
	return len - 1;
}

int get_filename_components(const char *filename, char *component, int start)
{
	int len;

	for (len = 0; start + len < strlen(filename)
	     && filename[start + len] != '\n'; len++) {
		if (filename[start + len] == '/' && len != 0)
			break;
	}
	if (len == FILENAME_LEN)
		return -1;

	strncpy(component, filename, start + len);
	component[start + len] = '\0';
		
	printf("start:%d,len:%d,component:%s\n", start, len, component);
	return len;
}
