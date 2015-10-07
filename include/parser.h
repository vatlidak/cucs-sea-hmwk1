int get_user(char *line, char **user, char *delimiters);
int get_group(char *line, char **group, char *delimiters);
int get_filename(char *line, char **filename, char *delimiters);
int get_cmd(char *line, char **cmd, char *delimiters);
int get_perm(char *line, char **perm, char *delimiters);

int tokenize(char *line, char ***args, char *delim);
