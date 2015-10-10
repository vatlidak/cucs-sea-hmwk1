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
void ls(struct file *fs)
{
	fprintf(stderr, "-------\n");
	fprintf(stderr, "#filename:user.group,permissions,children\n");
	while (fs != NULL) {
		struct acl *temp = fs->acls;

		while (temp != NULL) {
			 fprintf(stderr, "%s:%s.%s,%d,%d\n",
				 fs->filename, temp->user,
				 temp->group, temp->permissions,
				 fs->children);
			 temp = temp->next;
		}
		fs = fs->next;
	}
	fprintf(stderr, "-------\n");
}
#endif


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
	int len, nlines;
	char *line, *user,  *group, *filename, *linecopy;
	size_t n = 12345;
	struct file *file_handle;
	struct acl acl_rw, acl_r;

	nlines = 0;
	while (1) {
		/* when line is NULL: getline allocates appropriate buffer */
		line = NULL;
		len = getline(&line, &n, input_stream);
		if (len == -1)
			goto end_of_section;

		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto end_of_section;

		linecopy = strdup(line);
		if (linecopy == NULL) {
			perror("calloc");
			goto end_of_section;
		}
		linecopy[strlen(line)-1] = '\0';
		nlines++;

		if (is_invalid_line(line))
			goto malformed_line;

		len = get_user(line, &user, ".\n");
		if (len < 0)
			goto malformed_line;

		len = get_group(NULL, &group, "\n ");
		if (len < 0)
			goto malformed_line;

		len = get_filename(NULL, &filename, "\n");
		if (len < 0 || len > FILENAME_LEN)
			goto malformed_line;

		memset(&acl_rw, 0, sizeof(struct acl));
		acl_rw.user = user;
		acl_rw.group = group;
		acl_rw.permissions = READ_WRITE;

		memset(&acl_r, 0, sizeof(struct acl));
		acl_r.user = "*";
		acl_r.group = "*";
		acl_r.permissions = READ;

		DEBUG("User:%s, Group:%s, Filename:%s, len:%d\n",
		      acl_rw.user, acl_rw.group, filename, len);
		/*
		 * At this point it is quaranteed that we have
		 * proper user, group, and filename (components missing)
		 */
		if (len > 0) {
			file_handle = f_ops_create(fs, filename, &acl_rw);
			f_ops_update(fs, filename, &acl_r);
			if (file_handle)
				printf("%d\tY\tOK\n", nlines);
			else
				printf("%d\tX\tE: "
				       "Failed to create: \"%s\"\n",
				       nlines, filename);
		} else if (!len && file_handle) {

			if (f_ops_update(fs, file_handle->filename, &acl_rw))
				printf("%d\tY\tOK\n", nlines);
			else
				printf("%d\tX\tE: "
				       "Failed to update ACLs of: \"%s\"\n",
				       nlines, file_handle->filename);
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
	int ncmds;

	char *cmd;
	char *line;
	char *user;
	char *perm;
	char *group;
	char *filename;
	char *linecopy;

	struct acl acl;
	size_t n = 12345;


	ncmds = 0;
	while (1) {
		line = NULL;
		len = getline(&line, &n, input_stream);
		if (len == -1)
			goto end_of_section;

		linecopy = strdup(line);
		if (linecopy == NULL) {
			perror("calloc");
			goto end_of_section;
		}
		linecopy[strlen(line)-1] = '\0';
		ncmds++;

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
			rval = f_ops_acl_check(fs, filename, &acl);
			if (rval)
				printf("%d\tN\t%s\n", ncmds, linecopy);
			else
				printf("%d\tY\t%s\n", ncmds, linecopy);
		} else if (!strcmp(cmd, "WRITE")) {
			acl.permissions = WRITE;
			rval = f_ops_acl_check(fs, filename, &acl);
			if (rval)
				printf("%d\tN\t%s\n", ncmds, linecopy);
			else
				printf("%d\tY\t%s\n", ncmds, linecopy);
		} else if (!strcmp(cmd, "DELETE")) {
			acl.permissions = WRITE;
			if (f_ops_delete(fs, filename, &acl))
				rval = OK;
			else
				rval = NOT_OK;
			if (rval != OK)
				printf("%d\tN\t%s\n", ncmds, linecopy);
			else
				printf("%d\tY\t%s\n", ncmds, linecopy);
		} else if (!strcmp(cmd, "ACL")) {
			acl.permissions = WRITE;
			rval = f_ops_acl_check(fs, filename, &acl);
			if (rval)
				printf("%d\tN\t%s\n", ncmds, linecopy);
			else
				printf("%d\tY\t%s\n", ncmds, linecopy);
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
				goto acl_loop;
			}
		} else if (!strcmp(cmd, "CREATE")) {
			acl.permissions = WRITE;
			rval = f_ops_acl_check(fs, filename, &acl);
			if (rval)
				printf("%d\tN\t%s\n", ncmds, linecopy);
			else
				printf("%d\tY\t%s\n", ncmds, linecopy);
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
	return ncmds > 0 ? OK : NOT_OK;

malformed_line:
	fprintf(stderr, "E: Malformed line: %s", line);
	fprintf(stderr, "E: Malformed operations section\n");
	fprintf(stderr, "E: Parsing operation section aborted\n");
	free(line);
	free(linecopy);
	return NOT_OK;
}



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
	rval = parse_file_operation_portion(&FS, stdin);
	if (rval)
		goto abort;
#ifdef _DEBUG
	ls(FS);
#endif
	f_ops_unmount(&FS);
	return OK;
abort:
	fprintf(stderr, "Simulation aborted\n");
	f_ops_unmount(&FS);
	return NOT_OK;
}
