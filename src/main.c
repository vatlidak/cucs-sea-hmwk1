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

#ifdef _DEBUG
#define DEBUG(fmt, ...) fprintf(stdout, fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif


int env_is_set;


/*
 * parse_user_definition: parses the user definition portion of the  file
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
		/* when line is NULL: getline allocates appropriate buffer */
		line = NULL;
		len = getline(&line, &n, input_stream);
		if (len == -1)
			goto malformed_line;

		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;

		len = get_user(line, &user, ".\n");
		if (len < 0)
			goto malformed_line;

		len = get_group(NULL, &group, "\n ");
		if (len < 0)
			goto malformed_line;

		DEBUG("User:%s, Group:%s\n", user, group);

		len = get_filename(NULL, &filename, ".\n ");
		if (len > FILENAME_LEN)
			goto malformed_line;
		/*
		 * At this point it is quaranteed that we have
		 * proper user, group, and filename (components missing)
		 */
		if (len > 0) {
			file_handle = f_ops_create(fs, filename, user, group);
			if (!file_handle) {
				fprintf(stderr,
					"E: Failed to create <%s> <%s.%s>\n",
					filename, user, group);
				break;
			}
		} else {
			file_handle = f_ops_update(fs, file_handle->filename,
						   user, group, READ_WRITE);
			if (!file_handle) {
				fprintf(stderr,
					"E: Failed to update <%s> <%s.%s>\n",
					filename, user, group);
				break;
			}
		}
		free(line);
	}
	fprintf(stderr, "E: Parsing user definition section aborted\n");
	free(line);
	return -1;

malformed_line:
	fprintf(stderr, "\n");
	fprintf(stderr, "E: Malformed line: %s", line);
	fprintf(stderr, "E: Malformed user definition section\n");
	fprintf(stderr, "E: Parsing aborted\n");
	fprintf(stderr, "\n");
	free(line);
	return -1;

end_of_section:
	free(line);
	env_is_set = 1;
	return 0;
}


/*
 * parse_file_operation: parses the file operation portion of the  file
 */
static int parse_file_operation_portion(struct file **fs, FILE *input_stream)
{
	int len;
	char *cmd;
	char *line;
	char *user;
	char *perm;
	char *group;
	char *filename;
	char *linecopy;
	size_t n = 12345;

	line = NULL;
	while ((len = getline(&line, &n, input_stream)) > 0) {

		linecopy = calloc(strlen(line), sizeof(char));
		if (linecopy == NULL) {
			perror("calloc");
			goto end_of_section;
		}
		strcpy(linecopy, line);
		linecopy[strlen(line)-1] = '\0';

		len = get_cmd_user_group_filename(line, &cmd, &user, &group,
						  &filename);
		DEBUG("cmd:%s, User:%s, Group:%s, Filename:%s\n",
		      cmd, user, group, filename);

		if (len < 0)
			goto malformed_line;
		if (strlen(filename) > FILENAME_LEN)
			goto malformed_line;

		if (!strcmp(cmd, "READ")) {
			printf("%s:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, user,
					       group, READ));
		} else if (!strcmp(cmd, "WRITE")) {
			printf("%s:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, user,
					       group, WRITE));
		} else if (!strcmp(cmd, "DELETE")) {
			printf("%s:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, user,
					       group, WRITE));
		} else if (!strcmp(cmd, "ACL")) {

			printf("%s:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, user,
					       group, WRITE));
acl_loop:
			len = getline(&line, &n, input_stream);
			if (len == -1)
				goto malformed_line;
			if (len == 2 && strncmp(line, ".\n", 2) == 0)
				continue;
			else {
				len = get_user_group_perm(line, &user,
							      &group, &perm);
				if (len < 0)
					goto malformed_line;
				if (strlen(filename) > FILENAME_LEN)
					goto malformed_line;
				DEBUG("cmd:%s, User:%s, Group:%s, Perm:%s\n",
				      "ACL", user, group, perm);
				goto acl_loop;
			}
		} else if (!strcmp(cmd, "CREATE")) {
			printf("%s:%d\n", linecopy,
			       f_ops_acl_check(fs, filename, user,
					       group, WRITE));
create_loop:
			len = getline(&line, &n, input_stream);
			if (len == -1)
				goto malformed_line;
			if (len == 2 && strncmp(line, ".\n", 2) == 0)
				continue;
			else {
				len = get_user_group_perm(line, &user,
							      &group, &perm);
				if (len < 0)
					goto malformed_line;
				if (strlen(filename) > FILENAME_LEN)
					goto malformed_line;
				DEBUG("cmd:%s, User:%s, Group:%s, Perm:%s\n",
				      "CREATE", user, group, perm);
				goto create_loop;
			}
		} else {
			printf("Not implememnted\n");
		}

		free(line);
		line = NULL;
	}
	goto end_of_section;

malformed_line:
	fprintf(stderr, "E: Malformed operations section\n");
	return -1;

end_of_section:
	free(line);
	return 0;
}


#ifdef _DEBUG
void ls(struct file *fs)
{
	printf("-------\n");
	while (fs != NULL) {
		struct acl *temp = fs->acls;
		while (temp != NULL) {
			 printf("%s:%s.%s-%d\n", fs->filename, temp->user,
				temp->group, temp->permissions);
			 temp = temp->next;
		}
		fs = fs->next;
	}
	printf("-------\n");
}
#endif


int main(int argc, char **argv)
{
	int rval;
	struct file *FS;

	FS = f_ops_mount(&FS);

	rval = parse_user_definition_portion(&FS, stdin);
	if (rval)
		goto abort;
#ifdef _DEBUG
	ls(FS);
#endif
	parse_file_operation_portion(&FS, stdin);
	return 0;
abort:
	fprintf(stderr, "Simulation aborted\n");
	return -1;
}
