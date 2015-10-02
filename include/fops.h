/* TODO: check this again */
#define FILENAME_LEN 256
#define USERNAME_LEN 16
#define GROUPNAME_LEN 16
#define COMPONENT_LEN 16
#define LINE_LEN  (USERNAME_LEN + 1 + GROUPNAME_LEN + 1 + FILENAME_LEN)

#define READ 0
#define WRITE 1
#define READ_WRITE 2
#define NO_PERMISSION 3


struct acl {
	int permissions;
	char user[USERNAME_LEN];
	char group[GROUPNAME_LEN];
	struct acl *next;
};


struct file {
	struct acl  *acls;
	char file_name[FILENAME_LEN];
	struct file *next;
};

struct file *fops_mount(struct file **fs);
struct file *fops_get_handle(struct file *fs, char *file_name);
struct file *fops_create(struct file **fs, char *file_name, char *user, char *group);
void *fops_update(struct file *file_handle, char *user, char *group, int permissions);
