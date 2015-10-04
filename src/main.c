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
#include "env_ops.h"


#ifdef _DEBUG
void ls(struct file *fs)
{
	while (fs != NULL) {
		while(fs->acls != NULL) {
			 printf("%s: %s.%s %d\n", fs->filename, fs->acls->user,
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
	char user[USERNAME_LEN + 1];
	char group[GROUPNAME_LEN + 1];
	char filename[FILENAME_LEN + 1];

	size_t n = 12345;


	while (1) {
		/* line set NULL: getline allocates appropriate buffer */
		line = NULL;
		if ((len = getline(&line, &n, input_stream)) == -1)
			break;

		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;

		if (len > LINE_LEN)
			goto malformed_line;

		len = get_user(line, user);
		if (len < 0 || len > USERNAME_LEN)
			goto malformed_line;

		len = get_group(line, group);
		if (len < 0 || len > GROUPNAME_LEN)
			goto malformed_line;
	
		len = get_filename(line, filename);
		if (len < 0 || len > FILENAME_LEN)
			goto malformed_line;
		/*
		 * At this point it is quaranteed that we have a null
		 * terminated line (including a newline character).
		 */
		char abs_path[FILENAME_LEN + 1];
		printf("<%s>.<%s>:<%s>\n", user, group, filename);
		if (len) {
			int i=0;
			char **components;
			tokenize(filename, &components, "/");
			abs_path[0] = '\0';
			while ( components[i] != NULL) {
				if (strlen(components[i]) > COMPONENT_LEN)
					break;
				sprintf(abs_path, "%s/%s",abs_path, components[i]);
				printf("Component:<%s>\n", abs_path);
				i++;
				//env_ops_create(fs, abs_path, user, group);
			}
			printf("--\n");
		}
	//	else {
	//		env_ops_update(env_ops_get_handle(*fs, abs_path),
	//			       user, group, READ_WRITE);
	//	}

		free(line);
		continue;

malformed_line:
		fprintf(stderr, "E: Malformed line: %s", line);
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

	FS = env_ops_mount(&FS);
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
