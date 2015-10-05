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
#include "f_ops.h"


#define DELIMITERS ". \n"

#ifdef _DEBUG
void ls(struct file *fs)
{
	while (fs != NULL) {
		while(fs->acls != NULL) {
			 printf("%s:%s.%s %d\n", fs->filename, fs->acls->user,
				fs->acls->group, fs->acls->permissions);
			 fs->acls =fs->acls->next;
		}
		fs = fs->next;
	}
	printf("-------\n");
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
	char *user;
	char *group;
	char *filename;

	size_t n = 12345;

	struct file *file_handle;

	while (1) {
		/* line set NULL: getline allocates appropriate buffer */
		line = NULL;
		if ((len = getline(&line, &n, input_stream)) == -1)
			goto malformed_line;

		printf("line:%s", line);
		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;

		if (len > LINE_LEN)
			goto malformed_line;

		len = get_user(line, &user, DELIMITERS);
		if (len < 0 )
			goto malformed_line;
		printf("User:<%s>\n", user);
		
		len = get_group(line, &group, DELIMITERS);
		if (len < 0)
			goto malformed_line;
		printf("Group:<%s>\n", group);

		len = get_filename(line, &filename, DELIMITERS);
		if (len > FILENAME_LEN)
			goto malformed_line;
		if (len > 0)
			printf("Filename:<%s>%d\n", filename, len);
		/*
		 * At this point it is quaranteed that we have
		 * proper user, group, and filename (components missing)
		 */
		if (len > 0) {
			file_handle = f_ops_create(fs, filename, user, group);
			if(!file_handle) {
				fprintf(stderr, "E: Failed to create <%s> <%s.%s>\n", filename, user, group);
				break;
			}
		} else {
			file_handle = f_ops_update(fs, file_handle->filename, user, group, READ_WRITE);
			if (!file_handle) {
				fprintf(stderr, "E: Failed to update <%s> <%s.%s>\n", filename, user, group);
				break;
			}
		}
		printf("--\n");
		free(line);
	}
	free(line);
	return -1;

malformed_line:
	fprintf(stderr, "\n");
	fprintf(stderr, "E: Malformed line: %s", line);
	fprintf(stderr, "E: Malformed user definition section\n");
	fprintf(stderr, "E: Setting environment stopped here... :-(\n");
	fprintf(stderr, "\n");
	free(line);
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
/*static int parse_file_operation_portion(FILE *input_stream)
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
*/

int main(int argc, char **argv)
{
	struct file *FS;

	FS = f_ops_mount(&FS);
#ifdef _DEBUG
	ls(FS);
#endif
	parse_user_definition_portion(&FS, stdin);
#ifdef _DEBUG
	ls(FS);
#endif
	//parse_file_operation(stdin);
	return 0;
}
