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
#include <libgen.h>

#include "parser.h"
#include "f_ops.h"


static inline int is_invalid_filename(const char *filename)
{
	int i;
	char *dup, *token;

	/* assert component length */
	dup = strdup(filename);
	token = strtok(dup, "/");
	if (strlen(token) > COMPONENT_LEN) {
		free(dup);
		goto error;
	}
	while ((token = strtok(NULL, "/")) != NULL)
		if (strlen(token) > COMPONENT_LEN) {
			free(dup);
			goto error;
		}
	free(dup);

	/* assert proper prefix of filename */
	if (filename[0] != '/')
		return NOT_OK;

	if (strncmp(filename, "/home", strlen("/home"))
	    && strncmp(filename, "/tmp", strlen("/tmp")))
		goto error;

	/* assert not more than one consecutive '/' */
	for (i = 0; i < (int)strlen(filename) - 1; i++)
		if (filename[i] == '/' && filename[i+1] == '/')
			goto error;
	return OK;
error:
	return NOT_OK;
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


static inline struct acl *copy_acls(struct acl **dst, struct acl *src)
{
	struct acl *temp;

	while (src != NULL) {
		temp = calloc(1, sizeof(struct acl));
		if (!temp) {
			perror("calloc");
			goto error;
		}
		temp->user = calloc(strlen(src->user) + 1, sizeof(char));
		temp->group = calloc(strlen(src->group) + 1, sizeof(char));
		if (!temp->user || !temp->group) {
			perror("calloc");
			goto error;
		}
		strcpy(temp->user, src->user);
		strcpy(temp->group, src->group);
		temp->permissions = src->permissions;
		temp->next = NULL;
		/* Append ACL from src (parent) to dst (child) */
		while (*dst)
			dst = &(*dst)->next;
		*dst = temp;
		src = src->next;
	}
	return *dst;
error:
	return NULL;
}


/*
 * Main method doing the ACL checks
 *
 * Needs work
 */
static int do_f_ops_acl_check(struct file **fs, char *filename,
			      struct acl *pacl)
{
	char *parent, *dup;
	struct file *file_handle;
	struct acl *ptemp;
	struct acl acl;

	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle)
		goto error;

	ptemp = file_handle->acls;
	if (!ptemp)
		goto error;

	/* recursively check READ permissions on all predecessors */
	dup = strdup(filename);
	parent = dirname(dup);
	if (!strncmp(parent, "/", strlen(parent)))
		goto no_predecessors_end_recursion;
	if (*parent == '.') {
		fprintf(stderr, "Can't parse parent of: \"%s\"\n", filename);
		free(dup);
		goto error;
	}
	acl.user = pacl->user;
	acl.group = pacl->group;
	acl.permissions = READ;
	if (do_f_ops_acl_check(fs, parent, &acl)) {
		fprintf(stderr, "Parent of: \"%s\" not READable\n", filename);
		free(dup);
		goto error;
	}
no_predecessors_end_recursion:
	free(dup);

	/* All right -- the actual ACLs check is performed here */
	while (ptemp != NULL) {
		if (!strcmp(ptemp->user, pacl->user) ||
		    !strcmp(ptemp->group, pacl->group) ||
		    !strcmp(ptemp->user, "*") ||
		    !strcmp(ptemp->group, "*")) {
			if ((ptemp->permissions & pacl->permissions) == 0)
				goto error;
			else
				return OK;
		}
		ptemp = ptemp->next;
	}
error:
	return NOT_OK;
}


/*
 * This method updates the ACLs of a file.
 *
 * Caller must have permissions to set acls and filename must
 * correspond to a valid file -- no checks here (only mem alloc).
 */
static struct file *do_f_ops_acl_set(struct file **fs, char *filename,
				     struct acl *pacl)
{
	struct acl **ppacl;
	struct file *file_handle;

	DEBUG("setting::%s,%s,%s,%d\n", filename, pacl->user, pacl->group,
	      pacl->permissions);

	file_handle = f_ops_get_handle(*fs, filename);
	if (pacl->permissions == INHERIT_FROM_PARENT) {
		if (!copy_acls(&file_handle->acls, file_handle->parent->acls))
			goto error;
		goto out;
	}

	ppacl = &file_handle->acls;
	while (*ppacl != NULL)
		ppacl = &((*ppacl)->next);
	*ppacl = calloc(1, sizeof(struct acl));
	if (!*ppacl) {
		perror("calloc");
		goto error;
	}
	(*ppacl)->permissions = pacl->permissions;
	(*ppacl)->user = calloc(strlen(pacl->user) + 1, sizeof(char));
	if (!(*ppacl)->user) {
		perror("calloc");
		goto error;
	}
	strcpy((*ppacl)->user, pacl->user);

	(*ppacl)->group = calloc(strlen(pacl->group) + 1, sizeof(char));
	if (!(*ppacl)->group) {
		perror("calloc");
		goto error;
	}
	strcpy((*ppacl)->group, pacl->group);
	(*ppacl)->next = NULL;
out:
	return file_handle;
error:
	return NULL;
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
		goto error;
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
		goto error;

	file_handle = calloc(1, sizeof(struct file));
	if (!file_handle) {
		perror("calloc");
		goto error;
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
		goto error;

	env_is_set = 1;

	return *fs;
error:
	return NULL;
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
 */
static struct file *do_f_ops_create(struct file **fs, char *filename,
				    struct acl *pacl)
{
	int rval;
	char *parent, *dup;
	char _filename[FILENAME_LEN];
	struct acl acl;
	struct file *file_handle;

	dup = strdup(filename);
	parent = dirname(dup);
	if (*parent == '.') {
		fprintf(stderr, "Can't parse parent of: \"%s\"\n", filename);
		goto error;
	}
	if (is_invalid_filename(filename)) {
		fprintf(stderr, "Invalid filename: \"%s\"\n", filename);
		goto error;
	}

	acl.user = pacl->user;
	acl.group = pacl->group;
	/*
	 * if user tries to create home folder, relax restrictions;
	 * but only for home folder. For any other creation WRITE
	 * persmission is required.
	 */
	acl.permissions = WRITE;
	memset(_filename, 0, FILENAME_LEN);
	sprintf(_filename, "/home/%s", acl.user);
	if (!strncmp(_filename, filename, strlen(filename)))
		acl.permissions = READ;
	rval = f_ops_acl_check(fs, parent, &acl);
	if (rval) {
		fprintf(stderr,
			"Parent of: \"%s\" isn't writable from user: \"%s\"\n",
			filename, acl.user);
		goto error;
	}

	if (f_ops_get_handle(*fs, filename)) {
		fprintf(stderr, "File: \"%s\" exists\n", filename);
		goto error;
	}
	/*
	 * Done with tests ;-)
	 */
	file_handle = calloc(1, sizeof(struct file));
	if (!file_handle) {
		perror("calloc");
		goto error;
	}
	strcpy(file_handle->filename, filename);
	file_handle->acls = NULL;
	file_handle->children = 0;
	file_handle->parent = f_ops_get_handle(*fs, parent);
	file_handle->parent->children++;
	file_handle->next = *fs;
	*fs = file_handle;
	free(dup);

	return do_f_ops_acl_set(fs, filename, pacl);
error:
	free(dup);
	return NULL;
}


/*
 * This method deletes an existing file.
 *
 * WRITE permission on the parent is first asserted and then
 * ACL structs are deleted along with the file structure.
 *
 */
static struct file *do_f_ops_delete(struct file **fs, char *filename,
				    struct acl *pacl)
{
	int rval;
	struct acl acl, *temp;
	struct file *file_handle, *prev;
	char *parent, *dup;
	char _filename[FILENAME_LEN];

	dup = strdup(filename);
	parent = dirname(dup);
	if (*parent == '.') {
		fprintf(stderr, "Can't parse parent of: \"%s\"\n", filename);
		goto error;
	}

	/* During unmounting disable checks to allow removing of everything*/
	if (!env_is_set)
		goto relax_checks;

	/* Otherwise, no-one removes /home and /tmp */
	if (!strcmp(filename, "/tmp") || !strcmp(filename, "/home")) {
		fprintf(stderr, "File \"%s\" cannot be removed\n", filename);
		goto error;
	}
	acl.user = pacl->user;
	acl.group = pacl->group;
	/*
	 * if user tries to remove his or her home folder, relax
	 * restrictions; but only for home folder. For any other creation
	 * WRITE persmission is required.
	 */
	acl.permissions = WRITE;
	memset(_filename, 0, FILENAME_LEN);
	sprintf(_filename, "/home/%s", acl.user);
	if (!strncmp(_filename, filename, strlen(filename)))
		acl.permissions = READ;
	rval = f_ops_acl_check(fs, parent, &acl);
	if (rval) {
		fprintf(stderr,
			"Parent of: \"%s\" isn't writable from user: \"%s\"\n",
			filename, acl.user);
		goto error;
	}

relax_checks:
	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle) {
		fprintf(stderr, "File: \"%s\" does not exist\n", filename);
		goto error;
	}

	if (file_handle->children)  {
		fprintf(stderr, "File: \"%s\" has children\n", filename);
		goto error;
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

	if (file_handle->parent)
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
	free(dup);
	return prev != NULL ? prev : *fs;
error:
	free(dup);
	return NULL;
}


/*
 * This method updates (sets) the ACLs of an existing file.
 *
 * Note that this method doesn't check for WRITE permissions
 * since in the parsing loop and update will always be following
 * a create (which asserts write permissions).
 */
static struct file *do_f_ops_update(struct file **fs, char *filename,
				    struct acl *pacl)
{
	struct file *file_handle;

	file_handle = f_ops_get_handle(*fs, filename);
	if (!file_handle) {
		fprintf(stderr, "File: \"%s\" does not exist\n", filename);
		goto error;
	}
	return do_f_ops_acl_set(fs, filename, pacl);
error:
	return NULL;
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


int f_ops_invalid_home_folder(struct file **fs, char *username)
{
	char _filename[FILENAME_LEN];

	DEBUG("checking homw folder of user:%s\n", username);

	memset(_filename, 0, FILENAME_LEN);
	sprintf(_filename, "/home/%s", username);
	if (f_ops_get_handle(*fs, _filename))
		return OK;

	return NOT_OK;
}
