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


#define READ_WRITE 1
#define OK 0
#define NOT_OK -1


struct acl {
	int permissions;
	char user[USERNAME_LEN];
	char group[GROUPNAME_LEN];
	struct acl *next;
};


struct file {
	struct acl  *acls;
	char file_name[FILENAME_LEN];
	struct file *next;
};



void ls(struct file *fs)
{
	while (fs != NULL) {
		printf("-%s\n", fs->file_name);
		fs = fs->next;
	}
}

struct file *get_file_handle(struct file *fs, char *file_name)
{
	while (fs != NULL) {
		if (!strcmp(fs->file_name, file_name))
			break;
		fs = fs->next;
	}

	return fs;
}


/* insert at start */
struct file *create_file(struct file **fs, char *file_name, char *user, char *group)
{
	struct file *f;

	if ((f = calloc(1, sizeof(struct file))) == NULL) {
		perror("calloc");
		return NULL;
	}
	if ((f->acls = calloc(1, sizeof(struct acl))) == NULL) {
	    perror("calloc");
	    return NULL;
	}
	f->acls->permissions = READ_WRITE;
	strcpy(f->acls->user, user);
	strcpy(f->acls->group, group);

	f->next = *fs;
	*fs = f;

	printf("Creating file:");
	printf("<%s>.<%s>.<%s>\n", file_name, user, group);

	return f;
}

void *update_acl(struct file *file_handle, char *file_name, char *user, char *group)
{
	printf("Updating file:");
	printf("<%s>.<%s>.<%s>\n", file_name, user, group);
	return NULL;
}


/*
 * parse_user_definition: parses the user definition portion of the  file
 *
 * @input_stream: the input to read the file from
 */
static int parse_user_definition(struct file **fs, FILE *input_stream)
{
	int len;
	char *line;
	char user[USERNAME_LEN + 1];
	char group[GROUPNAME_LEN + 1];
	char file_name[FILENAME_LEN + 1];

	void *file_handle;
	size_t n = 12345;


	while (1) {

		/* line set NULL: getline allocates appropriate buffer */
		line = NULL;
		if ((len = getline(&line, &n, input_stream)) == -1)
			break;
		
		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;

		/* get user.group */
		if (get_user(line, user) < 0 || get_group(line, group) < 0) {
			fprintf(stderr, "E: Malformed line: %s", line);
			continue;
		}
	
	//	printf("<%s>.<%s>\n", user, group);
		
		if (get_file_name(line, file_name)) {
			file_handle = create_file(fs, file_name, user, group);
		}
		else
			update_acl(file_handle, file_name, user, group);
		
		/*
		 * Here we create the ACLS
		 * For each file, add the users and groups
		 */

		free(line);
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
	size_t n = 12345;

	line = NULL;
	while ((len = getline(&line, &n, input_stream)) > 0) {
		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;
		printf("%d:%s\n", len, line);
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
	struct file *FS;

	FS = NULL;

	parse_user_definition(&FS, stdin);
	ls(FS);
	//parse_file_operation(stdin);
	return 0;
}


