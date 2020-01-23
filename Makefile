sshell: sshell.c
	gcc -g -Wall -Wextra -o sshell sshell.c


.PHONY : clean
clean:
	-rm sshell


