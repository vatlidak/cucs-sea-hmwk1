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

	/* parse user name field */
	for (len = 0; len < USERNAME_LEN + 1 && line[len] != '.'; len++)
		;
	if (len == USERNAME_LEN + 1)
		return -1;

	strncpy(user, line, len);
	user[len] = '\0';

	return len;
}

int get_group(const char *line, char *group)
{
	int len;

	/* move past user name field */
	for (len = 0; len < USERNAME_LEN + 1 && line[len] != '.'; len++)
		;
	if (len == USERNAME_LEN + 1)
		return -1;
	line = line + len + 1;
	
	/* parse group name field */
	for (len = 0; len < GROUPNAME_LEN + 1 && line[len] != ' '
	     && line[len] != '\n'; len++)
		strncpy(group, line, len);
	
	if (len == GROUPNAME_LEN + 1)
		return -1;

	strncpy(group, line, len);
	group[len] = '\0';

	return len;
}

int get_file_name(const char *line, char *file_name)
{
	int len;

	/*
	 * move past user name and group name field and return if
	 * file name starting point not found
	 */
	for (len = 0; len < LINE_LEN + 1 &&
	     line[len] != '/' && line[len] != '\n'; len++)
		;
	if (len == LINE_LEN + 1 || line[len + 1] == '\n')
		return -1;
	line = line + len;
	
	/* parse file name field */
	for (len = 0; len < LINE_LEN + 1 && line[len] != '\n'; len++)
		;
	if (len == LINE_LEN + 1)
		return -1;

	strncpy(file_name, line, len);
	file_name[len] = '\0';

	return len;
}
