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


/*
 * helper asserting validity of filename
 */
static int is_invalid_name(const char *filename)
{
	int i;

	if (strncmp(filename, "/home", strlen("/home"))
	    && strncmp(filename, "/tmp", strlen("/tmp")))
		return 1;

	for (i = 0; i < strlen(filename) - 1; i++)
		if (filename[i] == '/' && filename[i+1] == '/')
			return 1;
	return 0;
}


/*
 * helper getting fullpath's parent path
 */
static int get_parent(const char *filename, char *parent)
{
	int len;

	len = strlen(filename);
//	printf("%d\n", len);
	for (; len > 0 && filename[len] != '/'; len--)
		;

//	printf("%d\n", len);
	if (len == 0)
		return 1;

	strncpy(parent, filename, len);
	parent[len] = '\0';

	return 0;
}


/*
 * helper getting a file_handle on a filename
 */
static struct file *f_ops_get_handle(struct file *fs, char *filename)
{
	while (fs != NULL) {
		if (!strcmp(fs->filename, filename))
			break;
		fs = fs->next;
	}
	return fs;
}


/*
 * Main method doinf the ACL checks
 */
static int do_f_ops_acl_check(struct file **fs, char *filename, char *user,
			      char *group, int permissions)
{
	struct file *file_handle;
	
#ifdef _DEBUG
	printf("checking::%s,%s,%s,%d\n", filename, user, group, permissions);
#endif
	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle)
		return 1;

	return 0;

}


/*
 * Caller must have permissions to set acls
 */
static struct file *do_f_ops_acl_set(struct file **fs, char *filename,
				     char *user, char *group, int permissions)
{
	struct acl *acl;
	struct file *file_handle;

#ifdef _DEBUG
	printf("setting::%s,%s,%s,%d\n", filename, user, group, permissions);
#endif
	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle)
		return NULL;

	acl = file_handle->acls;
	
	acl = calloc(1, sizeof(struct acl));
	if (!acl) {
		perror("calloc");
		return NULL;
	}
	acl->permissions = permissions;

	acl->user = calloc(strlen(user) + 1, sizeof(char));
	if (!acl->user) {
		perror("calloc");
		return NULL;
	}
	strcpy(acl->user, user);

	acl->group = calloc(strlen(group) + 1, sizeof(char));
	if (!acl->group) {
		perror("calloc");
		return NULL;
	}
	strcpy(acl->group, group);

	acl->next = file_handle->acls;
	file_handle->acls = acl;

	return file_handle;
}


/*
 *
 */
static struct file *do_f_ops_mount(struct file **fs)
{
	struct file *file_handle;

	*fs = NULL;

	file_handle = calloc(1, sizeof(struct file));
	if (!file_handle) {
		perror("calloc");
		return NULL;
	}
	strcpy(file_handle->filename, "/home");
	file_handle->acls = NULL;

	file_handle->next = *fs;
	*fs = file_handle;

	if (!do_f_ops_acl_set(fs, file_handle->filename, "*", "*", NO_PERM))
		return NULL;
	if (!do_f_ops_acl_set(fs, file_handle->filename, "*", "*", READ_WRITE))
		return NULL;

	file_handle = calloc(1, sizeof(struct file));
	if (!file_handle) {
		perror("calloc");
		return NULL;
	}
	strcpy(file_handle->filename, "/tmp");
	file_handle->acls = NULL;

	file_handle->next = *fs;
	*fs = file_handle;

	if (!do_f_ops_acl_set(fs, file_handle->filename, "*", "*", NO_PERM))
		return NULL;
	if (!do_f_ops_acl_set(fs, file_handle->filename, "*", "*", READ_WRITE))
		return NULL;

	return *fs;
}


/*
 * Note: force predecesors
 * .assert valid name
 * .assert parent exists
 * .check ACLs on parent
 * .assert file does not exits
 */
static struct file *do_f_ops_create(struct file **fs, char *filename,
				    char *user, char *group)
{
	int rval;
	struct file *f;
	char parent[FILENAME_LEN];

	if (is_invalid_name(filename)) {
		fprintf(stderr, "Invalid filename: \"%s\"\n", filename);
		return NULL;
	}
	rval = get_parent(filename, parent);
	if (rval)
		return NULL;

	rval = do_f_ops_acl_check(fs, parent, user, group, WRITE);
	if (rval)
		return NULL;

	if (f_ops_get_handle(*fs, filename)) {
		fprintf(stderr, "File: \"%s\" exists\n", filename);
		return NULL;
	}
	/*
	 * Done with test ;-)
	 */

	f = calloc(1, sizeof(struct file));
	if (!f) {
		perror("calloc");
		return NULL;
	}
	strcpy(f->filename, filename);
	f->acls = NULL;

	f->next = *fs;
	*fs = f;

	if (!do_f_ops_acl_set(fs, filename, "*", "*", NO_PERM))
		return NULL;

	return do_f_ops_acl_set(fs, filename, user, group, READ_WRITE);
}


static struct file *do_f_ops_update(struct file **fs, char *filename,
				    char *user, char *group, int permissions)
{
	if (do_f_ops_acl_check(fs, filename, user, group, WRITE))
		return NULL;

	return do_f_ops_acl_set(fs, filename, user, group, permissions);
}



struct file *f_ops_mount(struct file **fs)
{
	struct file *file_handle;

	file_handle = do_f_ops_mount(fs);
#ifdef _DEBUG
	if (!file_handle)
		printf("E: mounting\n");
	else
		printf("mounting\n");
#endif
	return file_handle;
}


struct file *f_ops_create(struct file **fs, char *filename,
			    char *user, char *group)
{
	struct file *file_handle;

	file_handle =  do_f_ops_create(fs, filename, user, group);
#ifdef _DEBUG
	if (!file_handle)
		printf("E:creating::%s,%s,%s\n", filename, user, group);
	else
		printf("creating::%s,%s,%s\n", filename, user, group);
#endif
	return file_handle;
}


struct file *f_ops_update(struct file **fs, char *filename, char *user,
			    char *group, int permissions)
{
	struct file *file_handle;

	file_handle = do_f_ops_update(fs, filename, user, group, permissions);
#ifdef _DEBUG
	if (!file_handle)
		printf("E: updating::%s,%s,%s,%d\n", filename, user,
		       group, permissions);
	else
		
		printf("updating::%s,%s,%s,%d\n", filename, user,
		       group, permissions);
#endif
	return file_handle;
}
