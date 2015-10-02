/* TODO: check this again */
#define FILENAME_LEN 256
#define USERNAME_LEN 16
#define GROUPNAME_LEN 16
#define COMPONENT_LEN 16
#define LINE_LEN  (USERNAME_LEN + 1 + GROUPNAME_LEN + 1 + FILENAME_LEN)

int get_user(const char *line, char *user);
int get_group(const char *line, char *group);
int get_file_name(const char *line, char *file_name);
