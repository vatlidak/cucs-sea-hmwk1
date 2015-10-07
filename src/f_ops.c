/*
 * Filename: src/f_ops.c
 *
 * Description: Implements the file system operations
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

	return 0;
}


static inline struct file *f_ops_get_handle(struct file *fs, char *filename)
{
	while (fs != NULL) {
		if (!strcmp(fs->filename, filename))
			break;
		fs = fs->next;
	}
	return fs;
}


static inline struct file *f_ops_get_handle_prev(struct file *fs,
						 char *filename)
{
	while (fs != NULL) {
		if (fs->next)
			if (!strcmp(fs->next->filename, filename))
				break;
		fs = fs->next;
	}
	return fs;
}


/*
 * Main method doing the ACL checks
 *
 * Needs work
 */
static int do_f_ops_acl_check(struct file **fs, char *filename,
			      struct acl *pacl)
{
	struct file *file_handle;
	struct acl *ptemp;

	if (!env_is_set)
		return 0;

	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle)
		return 1;

	ptemp = file_handle->acls;
	if (!ptemp)
		return 1;

	while (ptemp != NULL) {
		if (!strcmp(ptemp->user, "*") ||
		    !strcmp(ptemp->group, "*"))
			return (ptemp->permissions & pacl->permissions) == 0;
		if (!strcmp(ptemp->user, pacl->user) ||
		    !strcmp(ptemp->group, pacl->group))
			return (ptemp->permissions & pacl->permissions) == 0;
		ptemp = ptemp->next;
	}
	return 1;
}


/*
 * This method updates the ACLs of a file; and if respective
 * node does not exist, it is created.
 *
 * Caller must have permissions to set acls -- no check here.
 */
static struct file *do_f_ops_acl_set(struct file **fs, char *filename,
				     struct acl *pacl)
{
	struct acl **ppacl;
	struct file *file_handle;

#ifdef _DEBUG
	printf("setting::%s,%s,%s,%d\n", filename, pacl->user,
	       pacl->group, pacl->permissions);
#endif
	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle)
		return NULL;

	ppacl = &file_handle->acls;

	while (*ppacl != NULL)
		ppacl = &((*ppacl)->next);

	*ppacl = calloc(1, sizeof(struct acl));
	if (!*ppacl) {
		perror("calloc");
		return NULL;
	}
	(*ppacl)->permissions = pacl->permissions;

	(*ppacl)->user = calloc(strlen(pacl->user) + 1, sizeof(char));
	if (!(*ppacl)->user) {
		perror("calloc");
		return NULL;
	}
	strcpy((*ppacl)->user, pacl->user);

	(*ppacl)->group = calloc(strlen(pacl->group) + 1, sizeof(char));
	if (!(*ppacl)->group) {
		perror("calloc");
		return NULL;
	}
	strcpy((*ppacl)->group, pacl->group);

	(*ppacl)->next = NULL;

	return file_handle;
}


/*
 * This method mounts (initializes) the filesystem, basically by
 * creating /tmp and /home.
 */
static struct file *do_f_ops_mount(struct file **fs)
{
	struct acl acl;
	struct file *file_handle;

	env_is_set = 0;
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

	acl.user = "*";
	acl.group = "*";
	acl.permissions = READ_WRITE;
	if (!do_f_ops_acl_set(fs, file_handle->filename, &acl))
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

	acl.user = "*";
	acl.group = "*";
	acl.permissions = READ_WRITE;
	if (!do_f_ops_acl_set(fs, file_handle->filename, &acl))
		return NULL;

	return *fs;
}


/*
 * This method creates a file in the file system.
 *
 * There is handful of checks before creation of a file:
 * - valid filename
 * - exisiting predecessors
 * - WRITE permision on parent
 * - not existing file
 *
 * If tests succeed file structure is creates and then ACLs are
 * set (i.e., created as will).
 *
 * TODO: write on all predecessors?
 */
static struct file *do_f_ops_create(struct file **fs, char *filename,
				    struct acl *pacl)
{
	int rval;
	struct acl acl;
	struct file *file_handle;
	char parent[FILENAME_LEN];

	if (is_invalid_name(filename)) {
		fprintf(stderr, "Invalid filename: \"%s\"\n", filename);
		return NULL;
	}
	rval = get_parent(filename, parent);
	if (rval)
		return NULL;

	acl.user = pacl->user;
	acl.group = pacl->group;
	acl.permissions = pacl->permissions;
	rval = do_f_ops_acl_check(fs, parent, &acl);
	if (rval)
		return NULL;

	if (f_ops_get_handle(*fs, filename)) {
		fprintf(stderr, "File: \"%s\" exists\n", filename);
		return NULL;
	}
	/*
	 * Done with tests ;-)
	 */

	file_handle = calloc(1, sizeof(struct file));
	if (!file_handle) {
		perror("calloc");
		return NULL;
	}
	strcpy(file_handle->filename, filename);
	file_handle->acls = NULL;

	file_handle->next = *fs;
	*fs = file_handle;

	return do_f_ops_acl_set(fs, filename, pacl);
}


/*
 * This method updates (sets) the ACLs of an existing file.
 *
 * Before setting the ACLs WRITE permission is asserted.
 */
static struct file *do_f_ops_update(struct file **fs, char *filename,
				    struct acl *pacl)
{
	if (do_f_ops_acl_check(fs, filename, pacl))
		return NULL;

	return do_f_ops_acl_set(fs, filename, pacl);
}


/*
 * This method deletes an existing file.
 *
 * WRITE permission on the parent is first asserted and then
 * ACL structs are deleted along with the file structure.
 *
 * TODO: prevent deletion of parent of any folder
 */
static struct file *do_f_ops_delete(struct file **fs, char *filename,
				    struct acl *pacl)
{
	int rval;
	struct acl acl, *temp;
	struct file *file_handle, *prev;
	char parent[FILENAME_LEN];

	/* During unmounting disable checks to allow easier removing */
	if (!env_is_set)
		goto no_checks;

	rval = get_parent(filename, parent);
	if (rval)
		return NULL;
	printf("getparent:%s\n", filename);

	acl.user = pacl->user;
	acl.group = pacl->group;
	acl.permissions = WRITE;
	rval = do_f_ops_acl_check(fs, parent, &acl);
	if (rval)
		return NULL;
no_checks:
	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle) {
		fprintf(stderr, "File: \"%s\" does not exist\n", filename);
		return NULL;
	}

	/* find previous node and shortcut current */
	prev = *fs;
	while (prev) {
		if (prev->next)
			if (!strcmp(prev->next->filename, filename))
			    break;
		prev = prev->next;

	}

	/* if no previous node found, current is at the beginning */
	if (prev)
		prev->next = file_handle->next;
	else
		*fs = file_handle->next;


	/* free memory */
	while (file_handle->acls) {
		temp = file_handle->acls;
		free(temp->user);
		free(temp->group);
		free(temp);
		file_handle->acls = file_handle->acls->next;
	}
	free(file_handle);

	return prev != NULL ? prev : *fs;
}


/*
 * This method mounts (initializes) the filesystem, basically by
 * creating /tmp and /home.
 */
static struct file *do_f_ops_unmount(struct file **fs)
{
	struct file *file_handle;
	struct acl acl;

	memset((char *)&acl, 0, sizeof(struct acl));
	env_is_set = 0;

	file_handle = *fs;
	while (file_handle) {
		f_ops_delete(fs, file_handle->filename, &acl);
		file_handle = file_handle->next;
	}

	return *fs;
}


/*
 * Exported interface
 *
 * mount: Initializes the file system (created /home and /tmp)
 * unmount: Deallocated the structs kept in mem.
 * create: Creates a file and adds it in the file system
 * update: Updates the acls of a file
 * check: Check (compares) mathing acl permissions on a file
 * delete: Delete a file from the file system
 */
struct file *f_ops_mount(struct file **fs)
{
	DEBUG("mounting\n");

	return do_f_ops_mount(fs);
}


struct file *f_ops_unmount(struct file **fs)
{
	DEBUG("unmounting\n");

	return do_f_ops_unmount(fs);
}


struct file *f_ops_create(struct file **fs, char *filename,
			  struct acl *pacl)
{
	DEBUG("creating::%s,%s,%s,%d\n", filename, pacl->user,
	      pacl->group, pacl->permissions);

	return  do_f_ops_create(fs, filename, pacl);
}


struct file *f_ops_update(struct file **fs, char *filename,
			  struct acl *pacl)
{
	DEBUG("updating::%s,%s,%s,%d\n", filename, pacl->user,
	      pacl->group, pacl->permissions);

	return do_f_ops_update(fs, filename, pacl);
}


struct file *f_ops_delete(struct file **fs, char *filename,
			  struct acl *pacl)
{
	DEBUG("deleting::%s,%s,%s,%d\n", filename, pacl->user,
	      pacl->group, pacl->permissions);

	return do_f_ops_delete(fs, filename, pacl);
}


int f_ops_acl_check(struct file **fs, char *filename, struct acl *pacl)
{

	DEBUG("checking::%s,%s,%s,%d\n", filename, pacl->user,
	      pacl->group, pacl->permissions);

	return do_f_ops_acl_check(fs, filename, pacl);
}
