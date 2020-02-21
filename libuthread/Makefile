# Target library
REMOVE := libuthread.a sem.o tps.o

C := gcc
FLAGS := -Wall -Werror

ifneq ($(V),1)
Q = @
endif

libuthread.a: queue.o thread.o sem.o tps.o
	@echo "AR	libuthread.a"
	$(Q)ar rcs $@ $^

sem.o: sem.c sem.h
	@echo "CC	$@"
	$(Q)$(C) $(FLAGS) -c -o $@ $<

tps.o: tps.c tps.h
	@echo "CC	$@"
	$(Q)$(C) $(FLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	$(Q)rm -f $(REMOVE)