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


int fs_is_mounted;


/*
 * helper asserting validity of filename
 */
static int is_valid_name(const char *filename)
{
	int i;

	if (!strcmp(filename, "/home") || !strcmp(filename, "/tmp"))
		return 0;

	if (strncmp(filename, "/home", strlen("/home")))
		return 1;

	for (i = 0; i < strlen(filename) - 1; i++)
		if (filename[i] == '/' && filename[i+1] == '/')
			return -1;
	return 0;
}


/*
 * helper getting fullpath's parent path
 */
static int get_parent(const char *filename, char *parent)
{
	int len;

	len = strlen(filename);
	for (; len > 0 && filename[len] != '/'; len--)
		;

	if (len == 0)
		return -1;

	strncpy(parent, filename, len);
	parent[len] = '\0';

	return len;
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
#ifdef _DEBUG
	printf("checking::%s,%s,%s,%d\n", filename, user, group, permissions);
#endif
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
	while (acl != NULL) {
		if (!strcmp(acl->user, user) && !strcmp(acl->group, group)) {
			acl->permissions = permissions;
			return file_handle;
		}
		acl = acl->next;
	}

	/* if file has no ACLs for current group and user, create them */
	acl = calloc(1, sizeof(struct acl));
	if (acl) {
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

#ifdef _DEBUG
	printf("mounting\n");
#endif
	*fs = NULL;
	fs_is_mounted = 0;

	file_handle = f_ops_create(fs, "/home", "*", "*");
	if (!file_handle)
		return NULL;
	f_ops_update(fs, "/home", "*", "*", READ);
	f_ops_create(fs, "/tmp", "*", "*");

	fs_is_mounted = 1;

	return *fs;
}


/*
 * Note: force predecesors
 */
static struct file *do_f_ops_create(struct file **fs, char *filename,
				    char *user, char *group)
{
	int rval;
	struct file *f;

#ifdef _DEBUG
	printf("creating::%s,%s,%s\n", filename, user, group);
#endif
	if (is_valid_name(filename)) {
		fprintf(stderr, "Invalid filename: \"%s\"\n", filename);
		return NULL;
	}

	rval = do_f_ops_acl_check(fs, filename, user, group, WRITE);
	if (rval)
		return NULL;

	if (f_ops_get_handle(*fs, filename)) {
		fprintf(stderr, "File: \"%s\" exists\n", filename);
		return NULL;
	}


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
#ifdef _DEBUG
	printf("updating::%s,%s,%s,%d\n", filename, user, group, permissions);
#endif
	if (do_f_ops_acl_check(fs, filename, user, group, WRITE))
		return NULL;

	return do_f_ops_acl_set(fs, filename, user, group, permissions);
}



struct file *f_ops_mount(struct file **fs)
{
	return do_f_ops_mount(fs);
}


struct file *f_ops_create(struct file **fs, char *filename,
			    char *user, char *group)
{
	return do_f_ops_create(fs, filename, user, group);
}


struct file *f_ops_update(struct file **fs, char *filename, char *user,
			    char *group, int permissions)
{
	return do_f_ops_update(fs, filename, user, group, permissions);
}
