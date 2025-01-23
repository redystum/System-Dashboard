# Libraries to include (if any)
LIBS=#-lm -pthread

# Compiler flags
CFLAGS=-Wall -Wextra -ggdb -std=c11 -pedantic -D_POSIX_C_SOURCE=200809L -Werror=vla #-pg

# Linker flags
LDFLAGS=#-pg

# Indentation flags
IFLAGS=-linux -brs -brf -br

# Name of the executable
PROGRAM=sysDash

# Prefix for the gengetopt file (if gengetopt is used)
PROGRAM_OPT=args

# Automatically find all source files in the current directory and /controllers
SRCS := $(wildcard *.c) $(wildcard controllers/*.c) $(PROGRAM_OPT).c
OBJS := $(filter-out $(PROGRAM_OPT).o, $(SRCS:.c=.o)) $(PROGRAM_OPT).o

# Clean and all are not files
.PHONY: clean all docs indent debugon optimize raspberrypi

all: $(PROGRAM)

# Ensure args.c and args.h are generated before building
$(PROGRAM_OPT).c $(PROGRAM_OPT).h: $(PROGRAM_OPT).ggo
	gengetopt < $(PROGRAM_OPT).ggo --file-name=$(PROGRAM_OPT)

# Activate DEBUG, defining the SHOW_DEBUG macro
debugon: CFLAGS += -D DEBUG_ENABLED -g
debugon: $(PROGRAM)

# Activate optimization (-O...)
OPTIMIZE_FLAGS=-O2
optimize: CFLAGS += $(OPTIMIZE_FLAGS)
optimize: LDFLAGS += $(OPTIMIZE_FLAGS)
optimize: $(PROGRAM)

# Build the executable
$(PROGRAM): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

# Rule to compile .c files into .o files
%.o: %.c $(PROGRAM_OPT).h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(PROGRAM) $(PROGRAM_OPT).h $(PROGRAM_OPT).c

docs: Doxyfile
	doxygen Doxyfile

Doxyfile:
	doxygen -g Doxyfile

depend:
	$(CC) -MM $(SRCS)

indent:
	indent $(IFLAGS) $(SRCS) *.h

pmccabe:
	pmccabe -v $(SRCS)

cppcheck:
	cppcheck --enable=all --verbose $(SRCS) *.h

run: all
	./$(PROGRAM)

valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(PROGRAM)

raspberrypi: CFLAGS += -D RASPBERRYPI
raspberrypi: $(PROGRAM)
