#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define CMDLINE_MAX 1024
#define ARG_MAX 128
#define PATH_MAX 1024

int func_cd(char **args, int position);
int func_pwd(char **args, int position);
int execute_args(char **args, int position, char *stack);

//System Buildin Command
char *builtin_args[] = {
	 "cd",
	 "pwd"
};


//System Buildin Functions
int (*builtin_func[])(char **, int) = {
	&func_cd,
	&func_pwd
};

//To truncate the whole string into pieces
char **get_args(char *cmd)
{
	int arg_size = ARG_MAX;
	int pointer = 0;
	char **args = malloc(arg_size * sizeof(char*));
	char *arg;
	
	arg = strtok(cmd, " ");
	while (arg != NULL) {
		args[pointer] = arg;
		pointer++;
		arg = strtok(NULL, " ");
	} 
	args[pointer] = NULL;
	
	return args;
}

//Implement cd function
int func_cd(char **args, int position)
{
	if (chdir(args[position + 1])) {
		fprintf(stderr, "Error: no such directory\n");
		return 1;
	}
	return 0;
}

//Implement pwd function
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

//Implement pushd function
int func_pushd(char **args, int position, char *stack) 
{
	char cwd[PATH_MAX];	
	int status;

	getcwd(cwd, sizeof(cwd));
	status = func_cd(args, position);
	if (status == 0) strcpy(stack, cwd);

	return status;
}

//Implement popd function
int func_popd(char *stack) 
{		
	if (chdir(stack)) {
		fprintf(stderr, "Error: no such directory\n");
		stack = NULL;		
		return 1;
	}
	stack = NULL;		
	return 0;
	
}

//Implement dirs function
int func_dirs(char *stack) 
{
	char cwd[PATH_MAX];	
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);	
	if (stack != NULL)	printf("%s\n", stack);	
	return 0;
}	

//To find any redirection signal
char *get_redir(char **args, int position, int *bit) 
{
	char *outfile;
	int redir = -1;
	int sign_bit = 1;
	for (int i = position; i < ARG_MAX; i++){
		if (args[i] == NULL) {
			redir = i;			
			break;
		}
		//Find the next signal
		outfile = strchr(args[i], '>');
		if (outfile != NULL) {
			if (outfile[1] == '&') {
			sign_bit++;
			outfile++;
			}
			//To determine wether there is whitespace between or after the signal
			if ((outfile[1] == '\0' && outfile == args[i] && sign_bit == 1) || (outfile[1] == '\0' && outfile == (args[i] + 1) && sign_bit == 2)) {
				redir = i - 1;							
				i++;
				outfile = args[i];		
			}
			else if ((outfile[1] == '\0' && outfile != args[i] && sign_bit == 1) || (outfile[1] == '\0' && outfile != (args[i] + 1) && sign_bit == 2)){			
				strncpy(args[i], args[i], outfile - args[i] - sign_bit + 1);
				args[i][outfile - args[i]] = '\0';
				redir = i;									
				i++;
				outfile = args[i];

			}

			else if ((outfile[1] != '\0' && outfile != args[i] && sign_bit == 1) || (outfile[1] != '\0' && outfile != (args[i] + 1) && sign_bit == 2)){			
				strncpy(args[i], args[i], outfile - args[i] - sign_bit + 1);
				args[i][outfile - args[i]] = '\0';
				outfile++;
				redir = i;									

			}
			else { //next_ins[1] != '\0' && next_ins == args[i]
				outfile++;
				redir = i - 1;
			}
		break;			
		}
	}
	for (int i = position; i <= redir; i++)   args[i+65] = args[i];
	*bit = sign_bit;	
	args[redir+1] = NULL;
	return outfile;
}

//Truncate the arguments to find the inputs and ouputs for pipe();
int truncate_args(char **args, char *stack)
{	
	int status = 0;
	char* next_ins = 0;
	int last_ins = 0;
	//To find the signal of pipe()
	for (int i = 0; i < ARG_MAX; i++){
		if (args[i] == NULL) break;
		next_ins = strchr(args[i], '|');
		if (next_ins != NULL) {
			//To nelegect any unecessary whitespaces
			if (next_ins[1] == '\0' && next_ins == args[i]) {
				i++;
				status += execute_args(args, last_ins, stack);
				last_ins = i;		
			}
			else if (next_ins[1] == '\0' && next_ins != args[i]){			
				strncpy(args[i], args[i], next_ins - args[i]);
				args[i][next_ins - args[i]] = '\0';
				next_ins++;
				status += execute_args(args, last_ins, stack);
				last_ins = i + 1;

			}

			else if (next_ins[1] != '\0' && next_ins != args[i]){			
				strncpy(args[i], args[i], next_ins - args[i]);
				args[i][next_ins - args[i]] = '\0';
				next_ins++;
				status += execute_args(args, last_ins, stack);
				args[i] = next_ins;				
				last_ins = i;

			}
			else { //next_ins[1] != '\0' && next_ins == args[i]
				args[i]++;
				status += execute_args(args, i, stack);
			}		
		}
	}
					status += execute_args(args, last_ins, stack);
	return status;
}


//Main part of execution, to exe the argument and find any special cases
int execute_args(char **args, int position, char *stack)
{
	pid_t pid;
  	int status;
	int sign_bit;
	char *outfile;

	//System builtin functions
	if (args[position] == NULL)
		return 1;
	for (int i = 0; i < 2; i++) {
		if (strcmp(args[position], builtin_args[i]) == 0)
			return (*builtin_func[i])(args, position);
	}
	
	if (strcmp(args[position], "pushd") == 0) return func_pushd(args, position, stack);
	if (strcmp(args[position], "popd") == 0) return func_popd(stack);
	if (strcmp(args[position], "dirs") == 0) return func_dirs(stack);	
	
	outfile = get_redir(args, position, &sign_bit);
	
	//main part of fork()
	pid = fork();
	if (pid == 0) {
		if (outfile != NULL) {
			int fd = open(outfile, O_RDWR | O_CREAT | O_TRUNC, 0644);
			if (sign_bit == 1) dup2(fd, 1);
			else dup2(fd, 2);
			close(fd); 
		}	

		if (execvp(args[65], args) == -1)
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
		char stack[CMDLINE_MAX];
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
		status = truncate_args(args, stack);
		
		fprintf(stderr, "+ completed '%s' [%d]\n", cmd, status);

        }

        return EXIT_SUCCESS;
}

