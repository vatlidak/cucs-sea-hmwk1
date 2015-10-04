/* TODO: check this again */
//#define FILENAME_LEN 256
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
	char filename[FILENAME_LEN];
	struct file *next;
};

struct file *f_ops_mount(struct file **fs);
struct file *f_ops_create(struct file **fs, char *filename, char *user, char *group);
struct file *f_ops_update(struct file *fs, char *filename, char *user, char *group, int permissions);
int f_ops_acl_check(struct file *fs, char *filename, char *user, char *group, int permissions);
