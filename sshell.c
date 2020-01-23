#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 1024
#define ARG_MAX 64
#define PATH_MAX 1024

int func_cd(char **args);
int func_pwd(char **args);

char *builtin_args[] = {
	 "cd",
	 "pwd"
};

int (*builtin_func[])(char **) = {
	&func_cd,
	&func_pwd
};

char **get_args(char *cmd)
{
	int arg_size = ARG_MAX;
	int pointer = 0;
	char **args = malloc(arg_size * sizeof(char*));
	char *arg;
	
	if (!args) {
	fprintf(stderr, "Allocation Error\n");
	exit(-1);
 	}

	arg = strtok(cmd, " ");
	while (arg != NULL) {
		args[pointer] = arg;
		pointer++;
		arg = strtok(NULL, " ");
	} 
	args[pointer] = NULL;
	
	return args;
}

int func_cd(char **args)
{
	if (chdir(args[1])) {
		printf("Error: no such directory\n");
		return 1;
	}
	return 0;
}

int func_pwd(char **args)	
{
	char cwd[PATH_MAX];	
	if (args[1] != NULL) {
		//printf("Too many arguments\n");
		//return 1;
	}	
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);

	return 0;
}


char *get_redir(char **args) 
{
	char *outfile;

	for (int i = 0; i < ARG_MAX; i++){
		if (args[i] == NULL) break;
		outfile = strchr(args[i], '>');
		if (outfile != NULL) {
			if (outfile++ == NULL) {
				outfile = args[i+1];
				i++;
			}
			else if (outfile != args[i]){			
				strncpy(args[i], args[i], outfile - args[i]);
				args[i][outfile - args[i]] = '\0';
				outfile++;
				break;
			}
					
		}
	}
	return outfile;
}

int execute_args(char **args)
{
	pid_t pid;
  	int status;
	char *outfile;
	int fd;

	if (args[0] == NULL)
		return 1;

	for (int i = 0; i < 2; i++) {
		if (strcmp(args[0], builtin_args[i]) == 0)
			return (*builtin_func[i])(args);
	}
	
	outfile = get_redir(args);

	pid = fork();
	if (pid == 0) {
		if (outfile != NULL) {
			fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
			if (fd < 0) {
				perror("Error: cannot open output file");
				return 1;
			}			
			dup2(fd, 1);
			close(fd); 
		}	
		if (execvp(args[0], args) == -1)
			exit(-1);
    
	} else if (pid > 0) {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	
	} else {
		perror("Unable to Fork");
		exit(-1);
	
	
	if (status != 0)
		return 1;
	
	return 0;
}

int main(void)
{
        char cmd[CMDLINE_MAX], temp_cmd[CMDLINE_MAX];

        while (1) {
                char *nl;
		char **args;
		int status;

                /* Print prompt */
                printf("sshell$ ");
                fflush(stdout);

                /* Get command line */
                fgets(cmd, CMDLINE_MAX, stdin);

                /* Print command line if stdin is not provided by terminal */
                if (!isatty(STDIN_FILENO)) {
                        printf("%s", cmd);
                        fflush(stdout);
                }

                /* Remove trailing newline from command line */
                nl = strchr(cmd, '\n');
                if (nl)
                        *nl = '\0';

                /* Builtin command */
                if (!strcmp(cmd, "exit")) {
                        fprintf(stderr, "Bye...\n");
                        printf("+ completed '%s' [0]\n", cmd);
						break;
                }

                /* Regular command */
		strcpy(temp_cmd, cmd);
		args = get_args(temp_cmd);
		status = execute_args(args);
		
		printf("+ completed '%s' [%d]\n", cmd, status);

        }

        return EXIT_SUCCESS;
}

