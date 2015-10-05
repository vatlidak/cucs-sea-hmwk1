/* TODO: check this again */
#define FILENAME_LEN 16
#define USERNAME_LEN 16
#define GROUPNAME_LEN 16
#define COMPONENT_LEN 16
#define LINE_LEN  (USERNAME_LEN + 1 + GROUPNAME_LEN + 1 + FILENAME_LEN)

int get_user(char *line, char **user, char *delimiters);
int get_group(char *line, char **group, char *delimiters);
int get_filename(char *line, char **filename, char *delimiters);

int tokenize(char *line, char ***args, char *delim);
