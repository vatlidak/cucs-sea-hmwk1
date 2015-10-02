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
#include "fops.h"


#ifdef _DEBUG
void ls(struct file *fs)
{
	while (fs != NULL) {
		while(fs->acls != NULL) {
			 printf("%s: %s.%s %d\n", fs->file_name, fs->acls->user, fs->acls->group, fs->acls->permissions);
			 fs->acls =fs->acls->next;
		}
		fs = fs->next;
	}
}
#endif


/*
 * parse_user_definition: parses the user definition portion of the  file
 *
 * @input_stream: the input to read the file from
 */
static int parse_user_definition_portion(struct file **fs, FILE *input_stream)
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
	
		if (get_file_name(line, file_name)) {
			file_handle = fops_create(fs, file_name, user, group);
		}
		else
			fops_update(file_handle, user, group, READ_WRITE);
		
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
static int parse_file_operation_portion(FILE *input_stream)
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

	FS = fops_mount(&FS);

	parse_user_definition_portion(&FS, stdin);
#ifdef _DEBUG
	printf("\n");
	printf("-----------------------------\n");
	printf("\n");
	ls(FS);
	printf("\n");
	printf("---%s\n", fops_get_handle(FS, "/home/bollinger")->file_name);
#endif
	//parse_file_operation(stdin);
	return 0;
}
