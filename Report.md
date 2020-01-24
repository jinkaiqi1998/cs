# EEC 150 Project Report
 - Kaiqi Jin (914037402)
 - Zeyu Bai  (914237257)

# Project Requirement:
> In this project, we are required to write codes to implement the sshell function that can simulate the functions of shell in Unix system. Through this function, it should be able to read the inputs from users and then execute them under 'sshell'.

>Below is an example:
 - ```
    sshell$ date
    2020/1/23/ Thur 21:03:23 PDT
    sshell$
   ``` 
 # Our Ideas to Implement the Functions Step by Step:
 * Before reading and getting the inputs, we analyze the string first. First make strings become two-dimension arrays. The every char* in the array express the contents of any two adjacent spaces. And a two-dimension arrays is a independent argument.
 
 * After it, we truncate every argument until we find one or more 'I'. And then we execute all contents separately. And we will check if there is any redirection when we are executing. If there exist any redirection. we will truncate again.
 
 * The purpose of it is to make sure we separate the contents of outputs and the object of outputs. Finally, we use ``` fork()``` and ```execvp()```to compile.


# Some Explanations of Code:
 - We used the below function to call two different functions with different names but have the same references at the same time. And it allows us to define several similar functions without switching.
```c
int (*builtin_func[])(char **, int) = {
    &func_cd,
    &func_pwd
};

```

* We used ```if()``` statements to determine both ```pipe()``` and redirection at the same time. Part of codes are shown below:

```c
if (next_ins[1] == '\0' && next_ins == args[i]){
    i++;
    status += execute_args(args, last_ins, stack);
    last_ins = i;
}
```
```c
else if (next_ins[i] == '\0' && next_ins != args[i]){
    strncpy(args[i], args[i], next_ind - args[i]);
    args[i][next_ins - args[i]] = '\0';
    next_ins++;
    status += execute_args(args, last_ins, stack);
    last_ins = i + 1;
}
```


# The Difficults We Have:


> In order to determind the position of redirection, we wrote a function ```char *get_redir(char **args, int position, int *bit)```. First, we tried to use recursion to achieve this function becasue it is easier. But recursion couldn't return the values we need. Therefore, we chose to write four different ```If()``` statements as shown below:

```c
if ((outfile[1] == '\0' && outfile == args[i] && sign_bit ==1) || (outfile[1] == '\0' && outfile == args[i + 1] && sign_bit == 2))
```
```c
else if ((outfile[1] == '\0' && outfile == args[i] && sign_bit ==1) || (outfile[1] == '\0' && outfile != args[i + 1] && sign_bit == 2))
```
```c
else if ((outfile[1] != '\0' && outfile != args[i] && sign_bit ==1) || (outfile[1] != '\0' && outfile != args[i + 1] && sign_bit == 2))
```



# Resource & Reference:






 - Search the ways how to use API at the [source]:
 (http://www.cplusplus.com/)
 
 - Search for how to use ```pipe()```at the [source]:
 (https://linux.die.net/man/2/pipe) and (https://www.geeksforgeeks.org/pipe-system-call/)
 
 - Sometimes discuss with classmates about the ideas to write this project.