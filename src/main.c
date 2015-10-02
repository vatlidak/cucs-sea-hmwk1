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

#define FILENAME_LEN 256
#define USERNAME_LEN 16
#define GROUPNAME_LEN 16

struct acl {
	int permissions;
};

struct group {
	char group[GROUPNAME_LEN];
};

struct user {
	char username[USERNAME_LEN];
	struct group *groups;
};

struct file {
	char finename[FILENAME_LEN];
	struct acl  *acls;
};

struct file_system {
	struct user *users;
	struct file *files;
};

/*
 * parse_user_definition: parses the user definition portion of the  file
 *
 * @input_stream: the input to read the file from
 */
static int parse_user_definition(FILE *input_stream)
{
	int len;
	char *line;
	size_t n= 12345;

	/*
	 * Note that since line is set NULL, getline() will allocate
	 * an appropriate buffer for storing the line and n is unused
	 */
	line = NULL;
	while ((len=getline(&line, &n, input_stream)) > 0) {
		if (len == 2 && strncmp(line,".\n",2) == 0)
			goto end_of_section;
		free(line);
		line = NULL;
	}
	
	fprintf(stderr, "E: Malformed user definition section\n");
	return -1;

end_of_section:
	free(line);
	return 0;
}


/*
 * parse_file_operation: parses the file operation portion of the  file
 *
 * @input_stream: the input to read the file from
 */
static int parse_file_operation(FILE *input_stream)
{
	int len;
	char *line;
	size_t n= 12345;

	line = NULL;
	while ((len=getline(&line, &n, input_stream)) > 0) {
		if (len == 2 && strncmp(line,".\n",2) == 0)
			goto end_of_section;
		free(line);
		line = NULL;
	}
	
	fprintf(stderr, "E: Malformed user definition section\n");
	return -1;

end_of_section:
	free(line);
	return 0;


	return 0;
}

int main(int argc, char **argv)
{
	parse_user_definition(stdin);
	parse_file_operation(stdin);
	return 0;
}


