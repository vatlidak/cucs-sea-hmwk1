/*
 * Filename: src/main.c
 *
 * Description: Main parsing loop entry-points
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
static inline void ls(struct file *fs)
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


static inline int is_invalid_line(const char *line, int max_spaces)
{
	int i, spaces = 0;

	for (i = 0; i < ((int)strlen(line)) - 1; i++)
		if (line[i] == ' ')
			spaces++;
	return spaces > max_spaces ? NOT_OK : OK;
}


static inline void get_at_end_of_command(FILE *input_stream)
{
	size_t n = 12345;
	char *opsline;
	int len;

	while (1) {
		opsline = NULL;
		len = getline(&opsline, &n, input_stream);
		if (len == -1 || !strcmp(opsline, ".\n"))
			break;
		free(opsline);
	}
	free(opsline);
}


/*
 * parse_user_definition: parses the user definition portion of the  file
 */
static int parse_user_definition_portion(struct file **fs, FILE *input_stream)
{
	int len, nlines;
	size_t n = 12345;
	struct file *file_handle;
	struct acl acl_rw, acl_r;
	char *line, *_line, *user,  *group, *filename;

	nlines = 0;
	file_handle = NULL;
	while (1) {
		/* when line is NULL: getline allocates appropriate buffer */
		line = NULL;
		len = getline(&line, &n, input_stream);
		if (len == -1) {
			free(line);
			goto out;
		}

		_line = strdup(line);
		if (_line == NULL) {
			perror("calloc");
			goto out_free_line;
		}
		_line[strlen(line)-1] = '\0';
		nlines++;

		if (len == 2 && strncmp(line, ".\n", 2) == 0)
			goto out_free_line;
		if (is_invalid_line(_line, 1))
			goto error_free_line;
		len = get_user(line, &user, ".\n");
		if (len < 0)
			goto error_free_line;
		len = get_group(NULL, &group, "\n ");
		if (len < 0)
			goto error_free_line;
		len = get_filename(NULL, &filename, "\n");
		if (len < 0 || len > FILENAME_LEN)
			goto error_free_line;

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
			if (file_handle) {
				f_ops_update(fs, filename, &acl_r);
				printf("%d\tY\tOK\n", nlines);
			} else {
				printf("%d\tX\tE: "
				       "Failed to create: \"%s\"\n",
				       nlines, filename);
			}
		} else if (!len && file_handle) {
			if (f_ops_invalid_home_folder(fs, user)) {
				printf("%d\tX\tE: "
				       "Failed to update ACLs -- "
				       "no home folder for user \"%s\"\n",
				       nlines, user);
				goto cmd_loop;
			}
			if (f_ops_update(fs, file_handle->filename, &acl_rw))
				printf("%d\tY\tOK\n", nlines);
			else
				printf("%d\tX\tE: "
				       "Failed to update ACLs of: \"%s\"\n",
				       nlines, file_handle->filename);
		}
		goto cmd_loop;
error_free_line:
		printf("%d\tX\tE: Malformed line %s\n", nlines, _line);
cmd_loop:
		free(line);
		free(_line);
	}

out_free_line:
	free(line);
	free(_line);
out:
	return nlines > 1 ? OK : NOT_OK;
}


/*
 * parse_file_operation: parses the file operation portion of the  file
 */
static int parse_file_operation_portion(struct file **fs, FILE *input_stream)
{
	size_t n = 12345;
	int len, rval, ncmds;
	struct acl acl_, acl_w, acl_r;
	struct file *file_handle;
	char *cmdline, *_cmdline, *opsline;
	char *cmd, *user, *permissions, *group, *filename;

	ncmds = 1;
	while (1) {
		cmdline = NULL;
		len = getline(&cmdline, &n, input_stream);
		if (len == -1) {
			free(cmdline);
			goto out;
		}
		_cmdline = strdup(cmdline);
		if (_cmdline == NULL) {
			perror("calloc");
			goto out_free_cmd;
		}
		_cmdline[strlen(cmdline)-1] = '\0';
		if (is_invalid_line(_cmdline, 2))
			goto error_free_cmd;
		len = get_cmd(cmdline, &cmd, "\n ");
		if (len < 0)
			goto error_free_cmd;
		len = get_user(NULL, &user, "\n.");
		if (len < 0)
			goto error_free_cmd;
		len = get_group(NULL, &group, "\n ");
		if (len < 0)
			goto error_free_cmd;
		len = get_filename(NULL, &filename, "\n");
		if (len < 0 || len > FILENAME_LEN)
			goto error_free_cmd;
		DEBUG("cmd:%s, User:%s, Group:%s, Filename:%s\n",
		      cmd, user, group, filename);

		if (!strcmp(cmd, "READ")) {
			acl_r.user = user;
			acl_r.group = group;
			acl_r.permissions = READ;
			rval = f_ops_acl_check(fs, filename, &acl_r);
			if (rval) {
				printf("%d\tN\t%s\tE: ", ncmds, _cmdline);
				printf("Failed to read file\n");
			} else {
				printf("%d\tY\t%s\tOK\n", ncmds, _cmdline);
			}
			goto cmd_loop;
		} else if (!strcmp(cmd, "WRITE")) {
			acl_w.user = user;
			acl_w.group = group;
			acl_w.permissions = WRITE;
			if (f_ops_acl_check(fs, filename, &acl_w)) {
				printf("%d\tN\t%s\tE: ", ncmds, _cmdline);
				printf("Failed to write file\n");
			} else {
				printf("%d\tY\t%s\tOK\n", ncmds, _cmdline);
			}
			goto cmd_loop;
		} else if (!strcmp(cmd, "DELETE")) {
			acl_w.user = user;
			acl_w.group = group;
			acl_w.permissions = WRITE;
			if (f_ops_acl_check(fs, filename, &acl_w)) {
				printf("%d\tN\t%s\tE: ", ncmds, _cmdline);
				printf("Failed to delete file\n");
			} else {
				f_ops_delete(fs, filename, &acl_w);
				printf("%d\tY\t%s\tOK\n", ncmds, _cmdline);
			}
			goto cmd_loop;
		} else if (!strcmp(cmd, "CREATE")) {
			/*
			 * CREATE user.group filename
			 *	   user1.group1 permissions
			 *	   user2.group2 permissions
			 *	   ...
			 *	   userN.groupN permissions
			 *	   .
			 */
			file_handle = NULL;
create_loop:
			/* loop consuming lines following CREATE cmd */
			opsline = NULL;
			len = getline(&opsline, &n, input_stream);
			if (len == -1)
				goto out_free_ops_free_cmd;
			/* end of a CREATE cmd with NULL ACLs specified*/
			if (len == 2 && !strcmp(opsline, ".\n")
			    && !file_handle) {
				acl_.user = user;
				acl_.group = group;
				acl_.permissions = encode("inherit");
				file_handle = f_ops_create(fs, filename, &acl_);
				if (file_handle)
					printf("%d\tY\t%s\tOK\n",
					       ncmds, _cmdline);
				else
					printf("%d\tN\t%s\tE: "
					       "Failed to create file\n",
					       ncmds, _cmdline);
				free(opsline);
				goto cmd_loop;
			/* end of CREATE cmd with ACLs specified */
			} else if (len == 2 && !strcmp(opsline, ".\n")) {
				if (file_handle)
					printf("%d\tY\t%s\tOK\n",
					       ncmds, _cmdline);
				else
					printf("%d\tN\t%s\tE: "
					       "Failed to create file\n",
					       ncmds, _cmdline);
				free(opsline);
				goto cmd_loop;
			}
			/* if here, consume a "user.group permissions" line */
			len = get_user(opsline, &user, ".");
			if (len < 0)
				goto error_free_ops_free_cmd;
			len = get_group(NULL, &group, " ");
			if (len < 0)
				goto error_free_ops_free_cmd;
			len = get_perm(NULL, &permissions, "\n");
			if (len < 0)
				goto error_free_ops_free_cmd;
			acl_.user = user;
			acl_.group = group;
			acl_.permissions = encode(permissions);
			/* create or update file with ACLs read from stdin */
			if (!file_handle)
				file_handle = f_ops_create(fs, filename, &acl_);
			else
				f_ops_update(fs, filename, &acl_);
			free(opsline);
			if (file_handle) {
				printf("%d\tY\tOK\n", ncmds);
				goto create_loop;
			} else {
				/*
				 * creation failed the first time but we
				 * still need to consume lines until reaching
				 * the "." line.
				 */
				printf("%d\t%s\tN\tE: "
				       "Failed to create file\n",
				       ncmds, _cmdline);
				get_at_end_of_command(input_stream);
				goto cmd_loop;
			}
		/* end of CREATE cmd */
		} else if (!strcmp(cmd, "ACL")) {
			/*
			 * ACL user.group filename
			 *	   user1.group1 permissions
			 *	   user2.group2 permissions
			 *	   ...
			 *	   userN.groupN permissions
			 *	   .
			 */
			file_handle = NULL;
			/* loop consuming lines following ACL cmd */
acl_loop:
			opsline = NULL;
			len = getline(&opsline, &n, input_stream);
			if (len == -1)
				goto out_free_ops_free_cmd;
			/* end of a CREATE cmd with NULL ACLs specified*/
			if (len == 2 && !strcmp(opsline, ".\n")
			    && !file_handle) {
				acl_.user = user;
				acl_.group = group;
				printf("%d\tN\t%s\tE: ", ncmds, _cmdline);
				printf("Invalid (null) ACLs for file\n");
				free(opsline);
				goto cmd_loop;
			/* end of CREATE cmd with ACLs specified */
			} else if (len == 2 && !strcmp(opsline, ".\n")) {
				if (file_handle)
					printf("%d\tY\t%s\tOK\n",
					       ncmds, _cmdline);
				else
					printf("%d\tN\t%s\tE: "
					       "Failed to create file\n",
					       ncmds, _cmdline);
				free(opsline);
				goto cmd_loop;
			}
			/* if here, consume a "user.group permissions" line */
			len = get_user(opsline, &user, ".");
			if (len < 0)
				goto error_free_ops_free_cmd;
			len = get_group(NULL, &group, " ");
			if (len < 0)
				goto error_free_ops_free_cmd;
			len = get_perm(NULL, &permissions, "\n");
			if (len < 0)
				goto error_free_ops_free_cmd;
			acl_.user = user;
			acl_.group = group;
			acl_.permissions = encode(permissions);
			/* create or update file with ACLs read from stdin */
			if (!file_handle) {
				/*
				 * Remove the file (if it exists) so that new
				 * ACLs are created -- nasty hack!
				 */
				f_ops_delete(fs, filename, &acl_);
				file_handle = f_ops_create(fs, filename, &acl_);
			} else {
				f_ops_update(fs, filename, &acl_);
			}
			free(opsline);
			if (file_handle) {
				printf("%d\tY\t%s\tOK\n", ncmds, _cmdline);
				goto acl_loop;
			} else {
				/*
				 * acl failed the first time but we
				 * still need to consume lines until reaching
				 * the "." line.
				 */
				printf("%d\tN\t%s\tE: "
				       "Failed to create file\n",
				       ncmds, _cmdline);
				get_at_end_of_command(input_stream);
				goto cmd_loop;
			}
		}
		/* end of ACL command */
		printf("%d\tN\t%s\tE: You Shouldn't see this\n", ncmds, _cmdline);
error_free_ops_free_cmd:
		free(opsline);
error_free_cmd:
		printf("%d\tX\t%s\tE: Malformed line\n", ncmds, _cmdline);
cmd_loop:
		free(cmdline);
		free(_cmdline);
		ncmds++;
	/* end of while loop */
	}

out_free_ops_free_cmd:
	free(opsline);
out_free_cmd:
	free(cmdline);
	free(_cmdline);
out:
	return ncmds > 1 ? OK : NOT_OK;
}


int main(int argc, char **argv)
{
	int rval;
	struct file *FS;

	FS = f_ops_mount(&FS);
	printf("File system with read-only root \"/home\" folder\n");
	printf("User must first create his or her home folder\n");
	printf("eg.:vatlidak.phd /home/vatlidak\n");
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
