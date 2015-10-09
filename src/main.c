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


static inline int is_invalid_line(const char *line)
{
	int i, spaces = 0;

	for (i = 0; i < ((int)strlen(line)) - 1; i++)
		if (line[i] == ' ')
			spaces++;

	return spaces > 1 ? NOT_OK : OK;
}


/*
 * parse_user_definition: parses the user definition portion of the  file
 */
static int parse_user_definition_portion(struct file **fs, FILE *input_stream)
{
	int len;
	int nline;

	char *line;
	char *user;
	char *group;
	char *filename;
	char *linecopy;

	size_t n = 12345;

	struct file *file_handle;
	struct acl acl;

	nline = 0;
	while (1) {
		/* when line is NULL: getline allocates appropriate buffer */
		line = NULL;
		len = getline(&line, &n, input_stream);
		if (len == -1)
			goto end_of_section;

		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;

		linecopy = calloc(strlen(line) + 1, sizeof(char));
		if (linecopy == NULL) {
			perror("calloc");
			goto end_of_section;
		}
		strncpy(linecopy, line, strlen(line));
		nline++;

		if (is_invalid_line(line))
			goto malformed_line;

		len = get_user(line, &user, ".\n ");
		if (len < 0)
			goto malformed_line;

		len = get_group(NULL, &group, "\n ");
		if (len < 0)
			goto malformed_line;

		len = get_filename(NULL, &filename, "\n ");
		if (len < 0 || len > FILENAME_LEN)
			goto malformed_line;

		acl.user = user;
		acl.group = group;
		acl.permissions = READ_WRITE;

		DEBUG("User:%s, Group:%s, Filename:%s, len:%d\n",
		      acl.user, acl.group, filename, len);

		/*
		 * At this point it is quaranteed that we have
		 * proper user, group, and filename (components missing)
		 */
		if (len > 0) {
			file_handle = f_ops_create(fs, filename, &acl);
			if (file_handle)
				printf("%d\tY\tNo Error\n", nline);
			else
				printf("%d\tX\tError: "
				       "Failed to create: \"%s\"\n",
				       nline, filename);
		} else {
			file_handle = f_ops_update(fs,
						   file_handle->filename,
						   &acl);
			if (file_handle)
				printf("%d\tY\tNo Error\n", nline);
			else
				printf("%d\tX\tError: "
				       "Failed to update ACLs of: \"%s\"\n",
				       nline, file_handle->filename);
		}
		free(line);
		free(linecopy);
	}

end_of_section:
	free(line);
	return OK;

malformed_line:
	fprintf(stderr, "E: Malformed line: %s", line);
	fprintf(stderr, "E: Malformed user definition section\n");
	fprintf(stderr, "E: Parsing user definition section aborted\n");
	free(line);
	free(linecopy);
	return NOT_OK;

}


/*
 * parse_file_operation: parses the file operation portion of the  file
 */
static int parse_file_operation_portion(struct file **fs, FILE *input_stream)
{
	int len;
	int rval;
	char *cmd;
	char *line;
	char *user;
	char *perm;
	char *group;
	char *filename;
	char *linecopy;
	struct acl acl;
	size_t n = 12345;

	while (1) {
		line = NULL;
		len = getline(&line, &n, input_stream);
		if (len == -1)
			goto end_of_section;

		linecopy = calloc(strlen(line) + 1, sizeof(char));
		if (linecopy == NULL) {
			perror("calloc");
			goto end_of_section;
		}
		strncpy(linecopy, line, strlen(line));

		len = get_cmd(line, &cmd, "\n ");
		if (len < 0)
			goto malformed_line;

		len = get_user(NULL, &user, "\n.");
		if (len < 0)
			goto malformed_line;

		len = get_group(NULL, &group, "\n  ");
		if (len < 0)
			goto malformed_line;

		len = get_filename(NULL, &filename, "\n");
		if (len < 0 || len > FILENAME_LEN)
			goto malformed_line;

		acl.user = user;
		acl.group = group;
		DEBUG("cmd:%s, User:%s, Group:%s, Filename:%s\n",
		      cmd, acl.user, acl.group, filename);

		if (!strcmp(cmd, "READ")) {
			acl.permissions = READ;
			printf("%s. rval:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, &acl));
		} else if (!strcmp(cmd, "WRITE")) {
			acl.permissions = WRITE;
			printf("%s. rval:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, &acl));
		} else if (!strcmp(cmd, "DELETE")) {
			acl.permissions = WRITE;
			rval = f_ops_delete(fs, filename, &acl) ==
				NULL ? -1:0;
			printf("%s. rval:%d\n", linecopy, rval);

		} else if (!strcmp(cmd, "ACL")) {
			acl.permissions = WRITE;
			printf("%s. rval:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, &acl));

acl_loop:
			len = getline(&line, &n, input_stream);
			if (len == -1)
				goto malformed_line;
			if (len == 2 && strncmp(line, ".\n", 2) == 0)
				continue;
			else {
				len = get_user(line, &user, ".\n");
				if (len < 0)
					goto malformed_line;

				len = get_group(NULL, &group, "\n ");
				if (len < 0)
					goto malformed_line;

				len = get_perm(NULL, &perm, "\n");
				if (len < 0)
					goto malformed_line;
				/* TODO: set permissions */
				DEBUG("cmd:%s, User:%s, Group:%s, Perm:%d\n",
				      "ACL", acl.user, acl.group,
				      acl.permissions);
				goto acl_loop;
			}
		} else if (!strcmp(cmd, "CREATE")) {
			acl.permissions = WRITE;
			printf("%s:%d\n", linecopy,
			       -f_ops_acl_check(fs, filename, &acl));
create_loop:
			len = getline(&line, &n, input_stream);
			if (len == -1)
				goto malformed_line;
			if (len == 2 && strncmp(line, ".\n", 2) == 0)
				continue;
			else {
				len = get_user(line, &user, ".\n");
				if (len < 0)
					goto malformed_line;

				len = get_group(NULL, &group, "\n ");
				if (len < 0)
					goto malformed_line;

				len = get_perm(NULL, &perm, "\n");
				if (len < 0)
					goto malformed_line;

				DEBUG("cmd:%s, User:%s, Group:%s, Perm:%d\n",
				      "CREATE", acl.user, acl.group,
				      acl.permissions);
				goto create_loop;
			}
		} else {
			printf("Not implememnted\n");
		}
		free(line);
		free(linecopy);
	}
end_of_section:
	free(line);
	return OK;

malformed_line:
	fprintf(stderr, "E: Malformed line: %s", line);
	fprintf(stderr, "E: Malformed operations section\n");
	fprintf(stderr, "E: Parsing operation section aborted\n");
	free(line);
	free(linecopy);
	return NOT_OK;
}


#ifdef _DEBUG
void ls(struct file *fs)
{
	fprintf(stderr, "-------\n");
	while (fs != NULL) {
		struct acl *temp = fs->acls;

		while (temp != NULL) {
			 fprintf(stderr, "%s:%s.%s,%d\n",
				 fs->filename, temp->user,
				 temp->group, temp->permissions);
			 temp = temp->next;
		}
		fs = fs->next;
	}
	fprintf(stderr, "-------\n");
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
#ifdef _DEBUG
	ls(FS);
#endif
	f_ops_unmount(&FS);
#ifdef _DEBUG
	ls(FS);
#endif
	return OK;
abort:
	fprintf(stderr, "Simulation aborted\n");
	return NOT_OK;
}
