int get_cmd_user_group_filename(char *line, char **cmd, char **user,
				char **group, char **filename);
int get_user_group_perm(char *line, char **user,
			char **group, char **perm);

int get_user(char *line, char **user, char *delimiters);
int get_group(char *line, char **group, char *delimiters);
int get_filename(char *line, char **filename, char *delimiters);
int get_cmd(char *line, char **cmd, char *delimiters);

int tokenize(char *line, char ***args, char *delim);
