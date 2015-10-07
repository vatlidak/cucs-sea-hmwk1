/*
 * Filename: src/parser.c
 *
 * Description: Implements a set of parser methods
 *
 * Note that these methods will tokenize the line buffer
 * and pointers will be pointing some part of line buffer
 *
 * Copyright (C) 2015 V. Atlidakis
 *
 * COMS W4187 Fall 2015, Columbia University
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"


int get_user(char *line, char **user, char *delimiters)
{
	char *token;

	token = strtok(line, delimiters);
	if (token == NULL)
		return -1;

	*user = token;

	return strlen(*user);
}


int get_group(char *line, char **group, char *delimiters)
{
	char *token;

	token = strtok(line, delimiters);
	if (token == NULL)
		return -1;

	*group = token;

	return strlen(*group);
}


int get_filename(char *line, char **filename, char *delimiters)
{
	char *token;

	token = strtok(line, delimiters);
	if (token == NULL)
		return 0;

	*filename = token;

	return strlen(*filename);
}


int get_cmd(char *line, char **cmd, char *delimiters)
{
	char *token;

	token = strtok(line, delimiters);
	if (token == NULL)
		return -1;

	*cmd = token;

	return strlen(*cmd);
}


int get_perm(char *line, char **perm, char *delimiters)
{
	char *token;

	token = strtok(line, delimiters);
	if (token == NULL)
		return -1;

	*perm = token;

	return strlen(*perm);
}
