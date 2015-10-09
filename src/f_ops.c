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



static int is_invalid_filename(const char *filename)
{
	int i;

	/* TODO: check components length */
	if (filename[0] != '/')
		return NOT_OK;

	if (strncmp(filename, "/home", strlen("/home"))
	    && strncmp(filename, "/tmp", strlen("/tmp")))
		return NOT_OK;

	for (i = 0; i < (int)strlen(filename) - 1; i++)
		if (filename[i] == '/' && filename[i+1] == '/')
			return NOT_OK;
	return OK;
}


static int get_parent_name(const char *filename, char *parent)
{
	int len;

	len = (int)strlen(filename);
	for (; len > 0 && filename[len] != '/'; len--)
		;

	if (len == 0)
		return NOT_OK;

	strncpy(parent, filename, len);
	parent[len] = '\0';

	return OK;
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

	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle)
		return NOT_OK;

	ptemp = file_handle->acls;
	if (!ptemp)
		return NOT_OK;

	while (ptemp != NULL) {
		if (!strcmp(ptemp->user, "*") || !strcmp(ptemp->group, "*")) {
			if ((ptemp->permissions & pacl->permissions) == 0)
				return NOT_OK;
			else
				return OK;
		}
		if (!strcmp(ptemp->user, pacl->user) ||
		    !strcmp(ptemp->group, pacl->group)) {
			if ((ptemp->permissions & pacl->permissions) == 0)
				return NOT_OK;
			else
				return OK;
		}
		ptemp = ptemp->next;
	}
	return NOT_OK;
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

	DEBUG("setting::%s,%s,%s,%d\n", filename, pacl->user, pacl->group,
	      pacl->permissions);

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
	file_handle->parent = NULL;
	file_handle->children = 0;
	
	file_handle->next = *fs;
	*fs = file_handle;

	acl.user = "*";
	acl.group = "*";
	acl.permissions = READ;
	if (!do_f_ops_acl_set(fs, file_handle->filename, &acl))
		return NULL;

	file_handle = calloc(1, sizeof(struct file));
	if (!file_handle) {
		perror("calloc");
		return NULL;
	}
	strcpy(file_handle->filename, "/tmp");
	file_handle->acls = NULL;
	file_handle->parent = NULL;
	file_handle->children = 0;

	file_handle->next = *fs;
	*fs = file_handle;

	acl.user = "*";
	acl.group = "*";
	acl.permissions = READ_WRITE;
	if (!do_f_ops_acl_set(fs, file_handle->filename, &acl))
		return NULL;

	env_is_set = 1;

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

	if (is_invalid_filename(filename)) {
		fprintf(stderr, "Invalid filename: \"%s\"\n", filename);
		return NULL;
	}
	rval = get_parent_name(filename, parent);
	if (rval) {
		fprintf(stderr, "Can't parse parent name of: \"%s\"\n",
			filename);
		return NULL;
	}

	acl.user = pacl->user;
	acl.group = pacl->group;
	printf("<%s> - <%s>:%d\n", filename+6, pacl->user,
	       strncmp(filename+6, pacl->user, strlen(pacl->user)));
	if (!strncmp(filename+6, pacl->user, strlen(pacl->user)))
		acl.permissions = READ;
	else
		acl.permissions = WRITE;
	rval = do_f_ops_acl_check(fs, parent, &acl);
	if (rval) {
		fprintf(stderr, "Parent of: \"%s\" isn't writable\n", filename);
		return NULL;
	}

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
	file_handle->children = 0;
	file_handle->parent = f_ops_get_handle(*fs, parent);
	file_handle->parent->children++;
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

	/* During unmounting disable checks to allow removing of everything*/
	if (!env_is_set)
		goto no_checks;

	/* Otherwise, no-one removes /home and /tmp */
	if (!strcmp(filename, "/tmp") || !strcmp(filename, "/home")) {
		fprintf(stderr, "File \"%s\" cannot be removed\n", filename);
		return NULL;
	}

	rval = get_parent_name(filename, parent);
	if (rval) {
		fprintf(stderr, "Can't parse parent name of: \"%s\"\n",
			filename);
		return NULL;
	}

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

	if (file_handle->children)  {
		fprintf(stderr, "File: \"%s\" haschildren\n", filename);
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


	/*TODO: REMOVE THIS GETTER */
	if(file_handle->parent)
		file_handle->parent->children--;

	/* free memory */
	while (file_handle->acls) {
		temp = file_handle->acls;
		file_handle->acls = file_handle->acls->next;
		free(temp->user);
		free(temp->group);
		free(temp);
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
	struct file *file_handle, *temp;
	struct acl acl;

	memset((char *)&acl, 0, sizeof(struct acl));
	env_is_set = 0;

	file_handle = *fs;
	while (file_handle) {

		temp = file_handle->next;
		f_ops_delete(fs, file_handle->filename, &acl);
		file_handle = temp;

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
	DEBUG("deleting::%s\n", filename);

	return do_f_ops_delete(fs, filename, pacl);
}


int f_ops_acl_check(struct file **fs, char *filename, struct acl *pacl)
{

	DEBUG("checking::%s,%s,%s,%d\n", filename, pacl->user,
	      pacl->group, pacl->permissions);

	return do_f_ops_acl_check(fs, filename, pacl);
}
