CC = gcc
WARNINGS = -Wall -Wextra -Wshadow -Wstrict-prototypes -Wdouble-promotion \
		   -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op \
		   -Wduplicated-branches -Wduplicated-cond
EXECUTABLE = -o crypto
FLAGS = `pkg-config --cflags json-glib-1.0 gtk+-3.0`
LIBS = -lcurl `pkg-config --libs json-glib-1.0 gtk+-3.0`

debug:
	$(CC) *.c $(WARNINGS) $(FLAGS) -Og -g3 $(EXECUTABLE) $(LIBS)
release:
	$(CC) *.c $(WARNINGS) $(FLAGS) -march=native -mtune=native -O2 -DNDEBUG \
		$(EXECUTABLE) $(LIBS)
