/* TODO: check this again */
#define FILENAME_LEN 256
#define COMPONENT_LEN 16

#define READ 0
#define WRITE 1
#define READ_WRITE 2
#define NO_PERM 3


struct acl {
	int permissions;
	char *user;
	char *group;
	struct acl *next;
};


struct file {
	struct acl  *acls;
	char filename[FILENAME_LEN];
	struct file *next;
};

struct file *f_ops_mount(struct file **fs);
struct file *f_ops_create(struct file **fs, char *filename, char *user, char *group);
struct file *f_ops_update(struct file **fs, char *filename, char *user, char *group, int permissions);