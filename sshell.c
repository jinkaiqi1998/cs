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
#define PUNCH " \t\r\n\a"

int func_cd(char **args, int position);
int func_pwd(char **args, int position);
int execute_args(char **args, int position);

char *builtin_args[] = {
	 "cd",
	 "pwd"
};

int (*builtin_func[])(char **, int) = {
	&func_cd,
	&func_pwd
};

char **get_args(char *cmd)
{
	int arg_size = ARG_MAX;
	int pointer = 0;
	char **args = malloc(arg_size * sizeof(char*));
	char *arg;
	
	arg = strtok(cmd, PUNCH);
	while (arg != NULL) {
		args[pointer] = arg;
		pointer++;
		arg = strtok(NULL, PUNCH);
	} 
	args[pointer] = NULL;
	
	return args;
}

int func_cd(char **args, int position)
{
	if (chdir(args[position + 1])) {
		printf("Error: no such directory\n");
		return 1;
	}
	return 0;
}

int func_pwd(char **args, int position)	
{
	char cwd[PATH_MAX];	
	if (args[position + 1] != NULL) {
		//printf("Too many arguments\n");
		//return 1;
	}	
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);

	return 0;
}


char *get_redir(char **args, int position) 
{
	char *outfile;

	for (int i = position; i < ARG_MAX; i++){
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


int truncate_args(char **args, int position)
{	
	int status = 0;
	char* next_ins = 0;
	int last_ins = 0;
	for (int i = postition; i < ARG_MAX; i++){
		if (args[i] == NULL) break;
		next_ins = strchr(args[i], '|');
		if (next_ins != NULL) {
			if (next_ins[1] == '\0' && next_ins == args[i]) {
				i++;
				//printf("ONLY |\n");
				status += execute_args(args, last_ins);
				last_ins = i;		
			}
			else if (next_ins[1] == '\0' && next_ins != args[i]){			
				strncpy(args[i], args[i], next_ins - args[i]);
				args[i][next_ins - args[i]] = '\0';
				next_ins++;
				//printf("End with | \n");		
				status += execute_args(args, last_ins);
				last_ins = i + 1;

			}

			else if (next_ins[1] != '\0' && next_ins != args[i]){			
				strncpy(args[i], args[i], next_ins - args[i]);
				args[i][next_ins - args[i]] = '\0';
				next_ins++;
				//printf("Middle with | \n");		
				status += execute_args(args, last_ins);
				args[i] = next_ins;				
				last_ins = i;

			}
			else { //next_ins[1] != '\0' && next_ins == args[i]
				args[i]++;
				//printf("Start with |\n");
				status += execute_args(args, i);
			}		
		}
	}
					status += execute_args(args, last_ins);
	return status;
}





int execute_args(char **args, int position)
{
	pid_t pid;
  	int status;
	char *outfile;


	if (args[position] == NULL)
		return 1;
	//printf("Execute: %s\n", args[position]);
	for (int i = 0; i < 2; i++) {
		if (strcmp(args[position], builtin_args[i]) == 0)
			return (*builtin_func[i])(args, position);
	}
	
	outfile = get_redir(args, position);

	pid = fork();
	if (pid == 0) {
		if (outfile != NULL) {
			int fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
			dup2(fd, 1);
			close(fd); 
		}	

		if (execvp(args[position], args) == -1)
			exit(-1);
    
	} else if (pid > 0) {
		do {
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	
	} else exit(-1);
	
	
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
                        fprintf(stderr, "Bye...\n+ completed '%s' [0]\n",cmd);
						break;
                }

                /* Regular command */
		strcpy(temp_cmd, cmd);
		args = get_args(temp_cmd);
		status = truncate_args(args, 0);
		
		printf("+ completed '%s' [%d]\n", cmd, status);

        }

        return EXIT_SUCCESS;
}

