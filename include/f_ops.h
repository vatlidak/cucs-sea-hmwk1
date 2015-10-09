/*
 * Filename: include/f_ops.h
 *
 * Copyright (C) 2015 V. Atlidakis
 *
 * COMS W4187 Fall 2015, Columbia University
 */
#define FILENAME_LEN 256
#define COMPONENT_LEN 16

#define READ 1
#define WRITE 2
#define READ_WRITE 3
#define NO_PERM 0

#ifdef _DEBUG
#define DEBUG(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

#define OK 0
#define NOT_OK -1

int env_is_set;

struct acl {
	int permissions;
	char *user;
	char *group;
	struct acl *next;
};


struct file {
	struct acl  *acls;
	char filename[FILENAME_LEN];
	int children;
	struct file *next;
	struct file *parent;
};

struct file *f_ops_mount(struct file **fs);
struct file *f_ops_unmount(struct file **fs);
struct file *f_ops_create(struct file **fs, char *filename, struct acl *acl);
struct file *f_ops_update(struct file **fs, char *filename, struct acl *acl);
struct file *f_ops_delete(struct file **fs, char *filename, struct acl *acl);

int f_ops_acl_check(struct file **fs, char *filename, struct acl *acl);
