CC = clang
CFLAGS = -O0 -g -Wall
MKDIR = mkdir
RM = rm
FIND = find

OBJDIR = .obj

SRC = bcm2835.c main.c rc522.c rfid.c db.c
INC = bcm2835.h rc522.h rfid.h db.h
OBJ = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.o))
BINARY = rc522-mpc

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q := @
endif

all: $(OBJDIR) $(BINARY)

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.o: %.c $(INC)
	$(CC) $(CFLAGS) -o $@ -c $<

$(BINARY): $(OBJ)
	$(CC) -lsqlite3 $(CFLAGS) -o $(BINARY) $^

clean:
	$(FIND) $(OBJDIR) -type f -rm {} \;
	$(RM) -rf $(BINARY)

