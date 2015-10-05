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

static inline int get_parent(const char *filename, char *parent)
{
	int len;

	len = strlen(filename);
	for (; len >0 && filename[len] != '/'; len--)
		;

	if (len == 0)
		return -1;

	strncpy(parent, filename, len);
	parent[len] ='\0';
	return len;
}


static struct file *f_ops_get_handle(struct file *fs, char *filename)
{
	while (fs != NULL) {
		if (!strcmp(fs->filename, filename))
			break;
		fs = fs->next;
	}
	return fs;
}


static struct file *do_f_ops_mount(struct file **fs)
{
	struct file *file_handle;

	*fs = NULL;
	fs_is_mounted = 0;

	file_handle = f_ops_create(fs, "/home", "*", "*");
	if (!file_handle)
		return NULL;
	f_ops_update(*fs, "/home", "*", "*", READ);
	f_ops_create(fs, "/tmp", "*", "*");

	fs_is_mounted = 1;
#ifdef _DEBUG
	printf("filesystem mounted\n");
#endif
	return *fs;
}


static struct file *do_f_ops_create(struct file **fs, char *filename,
			    char *user, char *group)
{
	int rval;
	struct file *f;

	rval = f_ops_acl_check(*fs, filename, user, group, WRITE);
	if (rval)
		return NULL;

	if (f_ops_get_handle(*fs, filename)) {
		fprintf(stderr, "File: \"%s\" exists\n", filename);
		return NULL;
	}

	if ((f = calloc(1, sizeof(struct file))) == NULL) {
		perror("calloc");
		return NULL;
	}
	strcpy(f->filename, filename);
	if ((f->acls = calloc(1, sizeof(struct acl))) == NULL) {
	    perror("calloc");
	    return NULL;
	}
	f->acls->next = NULL;
	f->acls->permissions = READ_WRITE;
	strcpy(f->acls->user, user);
	strcpy(f->acls->group, group);

	
	f->next = *fs;
	*fs = f;

	printf("Created:<%s><%s>:<%s>\n", filename, user, group);
	return f;
}

static struct file *do_f_ops_update(struct file *fs, char *filename,
				    char *user, char *group, int permissions)
{
	struct acl *acl;

	struct file *file_handle; 

	if (f_ops_acl_check(fs, filename, user, group, WRITE))
		return NULL;

	file_handle = f_ops_get_handle(fs, filename);
	if (!file_handle)
		return NULL;

	acl = file_handle->acls;
	while ( acl != NULL) {
		if (!strcmp(acl->user, user) && !strcmp(acl->group, group))
		{
		    acl->permissions = permissions;
		    return file_handle;
		}
		acl = acl->next;
	}

	/* if file has no ACLs for current group and user, create them */
	if ((acl = calloc(1, sizeof(struct acl))) == NULL) {
	    perror("calloc");
	    return NULL;
	}
	acl->permissions = permissions;
	strcpy(acl->user, user);
	strcpy(acl->group, group);

	acl->next = file_handle->acls;
	file_handle->acls = acl;

	printf("Updated:<%s><%s>:<%s>\n", file_handle->filename, user, group);
	return file_handle;
}

static int  do_acl_check(struct file *fs, char *filename, char *user,
			 char *group, int permissions){

	int len;
	struct file *file_handle;
	char parent[FILENAME_LEN];

	if (!strcmp(filename, "/home") || !strcmp(filename, "/tmp"))
	    return 0;

	len = get_parent(filename, parent);
	if (len <= 0)
		return -1;

	printf("----%s:%s:%d\n", filename,parent, len);
	
//	file_handle = f_ops_get_handle(fs, parent);
//	if (file_handle == NULL)
//		return -1;
	//printf("#%s\n", file_handle->filename);
	return 0;
}


struct file *f_ops_mount(struct file **fs){
	return do_f_ops_mount(fs);
}


struct file *f_ops_create(struct file **fs, char *filename,
			    char *user, char *group)
{
	return do_f_ops_create(fs, filename, user, group);
}


struct file *f_ops_update(struct file *fs, char *filename, char *user,
			    char *group, int permissions)
{
	return do_f_ops_update(fs, filename, user, group, permissions);
}


int f_ops_acl_check(struct file *fs, char *filename, char *user,
		    char *group, int permissions){
	return do_acl_check(fs, filename, user, group, permissions);
}
