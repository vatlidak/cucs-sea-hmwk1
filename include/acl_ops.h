int acl_ops_read(struct file **fs, char *filename, char *user, char *group);
int acl_ops_write(struct file **fs, char *filename, char *user, char *group);
int acl_ops_create(struct file **fs, char *filename, char *user, char *group);
int acl_ops_delete(struct file **fs, char *filename, char *user, char *group);
int acl_ops_acl(struct file **fs, char *filename, char *user, char *group);
