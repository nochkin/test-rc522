CC = clang
CXX = clang++
CFLAGS_DEBUG = -O0 -g -Wall -I.
CFLAGS_OPT = -O3 -Wall -I. -march=armv6zk -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard
LINK = clang++
LDFLAGS = -lsqlite3
MKDIR = mkdir
RM = rm
FIND = find

OBJDIR = .obj

SRC = bcm2835.c rc522.c rfid.c
SRCXX = main.cpp db.cpp
INC = bcm2835.h rc522.h rfid.h
INCXX = db.h
OBJ = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.c.o))
OBJXX = $(patsubst %,$(OBJDIR)/%,$(SRCXX:.cpp=.cpp.o))
BINARY = rc522-mpc

ifeq ($(DEBUG),1)
	CFLAGS := $(CFLAGS_DEBUG)
	DEBUG_INFO = "\(Debug\)"
else
	CFLAGS := $(CFLAGS_OPT)
	DEBUG_INFO = ""
endif

# Be silent per default, but 'make V=1' will show all compiler calls.
ifneq ($(V),1)
Q = @echo "[C  ] $< $(DEBUG_INFO)" &&
QXX = @echo "[C++] $< $(DEBUG_INFO)" &&
QLD = @echo "[Lnk] $(BINARY) $(DEBUG_INFO)" &&
endif

all: $(OBJDIR) $(BINARY)

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

$(OBJDIR)/%.c.o: %.c $(INC)
	$(Q)$(CC) $(CFLAGS) -o $@ -c $<

$(OBJDIR)/%.cpp.o: %.cpp $(INCXX)
	$(QXX)$(CC) $(CFLAGS) -o $@ -c $<

$(BINARY): $(OBJ) $(OBJXX)
	$(QLD)$(LINK) $(LDFLAGS) -o $(BINARY) $^

clean:
	$(FIND) $(OBJDIR) -type f -exec rm -f {} \;
	$(RM) -f $(BINARY)

