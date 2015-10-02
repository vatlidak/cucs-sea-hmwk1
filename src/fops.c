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

struct file *fops_get_handle(struct file *fs, char *file_name)
{
	while (fs != NULL) {
		if (!strcmp(fs->file_name, file_name))
			break;
		fs = fs->next;
	}

	return fs;
}


/* insert at start */
struct file *fops_create(struct file **fs, char *file_name, char *user, char *group)
{
	struct file *f;

	if ((f = calloc(1, sizeof(struct file))) == NULL) {
		perror("calloc");
		return NULL;
	}
	strcpy(f->file_name, file_name);

	if ((f->acls = calloc(1, sizeof(struct acl))) == NULL) {
	    perror("calloc");
	    return NULL;
	}
	f->acls->permissions = READ_WRITE;
	strcpy(f->acls->user, user);
	strcpy(f->acls->group, group);

	f->next = *fs;
	*fs = f;

#ifdef _DEBUG
	printf("Creating file:<%s>.<%s>.<%s>\n", file_name, user, group);
#endif
	return f;
}

struct file *fops_mount(struct file **fs)
{
	*fs = NULL;
	return fops_create(fs, "/tmp", "*", "*");
}


void *fops_update(struct file *file_handle, char *user, char *group, int permissions)
{
	struct acl *acl;
	if ((acl = calloc(1, sizeof(struct acl))) == NULL) {
	    perror("calloc");
	    return NULL;
	}
	acl->permissions = permissions;
	strcpy(acl->user, user);
	strcpy(acl->group, group);

	acl->next = file_handle->acls;
	file_handle->acls = acl;

#ifdef _DEBUG
	printf("Updating file:<%s>.<%s>.<%s>\n", file_handle->file_name, user, group);
#endif
	return NULL;
}
