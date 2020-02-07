
#Target library

lib := libuthread.a
obj := queue.o

CC := gcc
CFLAGS := -Wall -Werror -g

ifneq ($(V),1)
Q = @
endif

libuthread.a: queue.o uthread.o context.o preempt.o
	@echo "AR libuthread.a"
	$(Q)ar rcs $@ $^

preempt.o: preempt.c preempt.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

queue.o: queue.c queue.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

context.o: context.c context.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

uthread.o: uthread.c uthread.h context.h queue.h
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

clean:
	@echo "CLEAN"
	$(Q)rm -f $(lib) *.o *.a *.out
