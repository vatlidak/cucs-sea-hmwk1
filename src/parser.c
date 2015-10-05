/*
 * Filename: src/parser.c
 *
 * Description: Implements a set of parser methods
 *
 * Copyright (C) 2015 V. Atlidakis
 *
 * COMS W4187 Fall 2015, Columbia University
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parser.h"


/*
 * get_user: parsing the user section from an input line
 *
 * line: the line to be parsed
 * user: the adress of a pointer that will point the start of user field
 * delimiters: the delimiters for the parsing
 *
 * return: the length of the token retrieved
 *
 * Note that this method will tokenize the line buffer and user will
 * be pointing a part of line buffer
 */
int get_user(char *line, char **user, char *delimiters)
{
	char *token;

	token = strtok(line, delimiters);
	if (token == NULL)
		return -1;

	*user = token;

	return strlen(*user);
}


/*
 * get_group: parsing the group section from an input line
 *
 * line: the line to be parsed
 * group: the adress of a pointer that will point the start of group field
 * delimiters: the delimiters for the parsing
 *
 * return: the length of the token retrieved
 *
 * Note that this method will tokenize the line buffer and group will
 * be pointing a part of line buffer
 */
int get_group(char *line, char **group, char *delimiters)
{
	char *token;

	token = strtok(NULL, delimiters);
	if (token == NULL)
		return -1;

	*group = token;

	return strlen(*group);
}


/*
 * get_filename: parsing the filename section from an input line
 *
 * line: the line to be parsed
 * group: the adress of a pointer that will point the start of group field
 * delimiters: the delimiters for the parsing
 *
 * return: the length of the token retrieved
 *
 * Note that this method will tokenize the line buffer and group will
 * be pointing a part of line buffer
 */
int get_filename(char *line, char **filename, char *delimiters)
{
	char *token;

	token = strtok(NULL, delimiters);
	if (token == NULL)
		return -1;

	*filename = token;

	return strlen(*filename);
}


/*
 * tokenize: parse line into args separated by delimiter(s)
 *
 * line: the line to be parsed
 * args the args retrieved by the line
 * delim: a set of delimiters
 *
 * return: zero on success, one otherwise
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
