CC = gcc
WARNINGS = -Wall -Wextra -Wshadow -Wstrict-prototypes -Wdouble-promotion \
		   -Wjump-misses-init -Wnull-dereference -Wrestrict -Wlogical-op \
		   -Wduplicated-branches -Wduplicated-cond
EXECUTABLE = crypto
FLAGS = `pkg-config --cflags json-glib-1.0 gtk+-3.0`
LIBS = -lcurl `pkg-config --libs json-glib-1.0 gtk+-3.0`

release:
	$(CC) *.c $(WARNINGS) $(FLAGS) -march=native -mtune=native -O2 -DNDEBUG \
		-o $(EXECUTABLE) $(LIBS)

debug:
	$(CC) *.c $(WARNINGS) $(FLAGS) -Og -g3 -o $(EXECUTABLE) $(LIBS)

install: 
	install $(EXECUTABLE) /usr/bin/
	install io.github.erikwallstrom.Crypto.desktop /usr/share/applications/
