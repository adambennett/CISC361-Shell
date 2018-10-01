# specify all source files here
SRCS = sh.c main.c builtins.c environ.c plumbing.c execute.c alias.c wildcard.c util.c

# specify target here (name of executable)
TARG = mysh

# specify compiler, compile flags, and needed libs
CC = gcc
OPTS = -g -Wall
LIBS = -lm

# this translates .c files in src list to .o’s
OBJS = $(SRCS:.c=.o)

# all is not really needed, but is used to generate the target
all: $(TARG)

# this generates the target executable
$(TARG): $(OBJS)
	$(CC) -o $(TARG) $(OBJS) $(LIBS)

# this is a generic rule for .o files
%.o: %.c
	$(CC) $(OPTS) -c $< -o $@

# and finally, a clean line
clean:
	rm -f $(OBJS) $(TARG)
