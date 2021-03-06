CC ?= clang
CXX ?= clang++
CFLAGS_DEBUG = -O0 -g -Wall -I/usr/local/include -I./libmpdclient-2.9/include
CFLAGS_OPT = -O3 -Wall -I/usr/local/include -I./libmpdclient-2.9/include -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard # -march=armv6zk
LINK ?= clang++
LDFLAGS = -L/usr/local/lib -L./libmpdclient-2.9/src/.libs -lsqlite3 -lmpdclient -static-libstdc++
MKDIR = mkdir
RM = rm
FIND = find
STRIP = strip

OBJDIR = .obj

SRC = bcm2835.c ini.c
SRCXX = main.cpp db.cpp reader.cpp controller.cpp mpclient.cpp INIReader.cpp config.cpp
INC = bcm2835.h ini.h
INCXX = db.h reader.h controller.h mpclient.h INIReader.h config.h
OBJ = $(patsubst %,$(OBJDIR)/%,$(SRC:.c=.c.o))
OBJXX = $(patsubst %,$(OBJDIR)/%,$(SRCXX:.cpp=.cpp.o))
BINARY = mpc-rfid

ifeq ($(DEBUG),1)
	CFLAGS := $(CFLAGS_DEBUG)
	DEBUG_INFO = "\(Debug\)"
	STRIP = ls -l
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
	$(QXX)$(CXX) $(CFLAGS) -o $@ -c $<

$(BINARY): $(OBJ) $(OBJXX)
	$(QLD)$(LINK) $(LDFLAGS) -o $(BINARY) $^
	@$(STRIP) $(BINARY)

clean:
	$(FIND) $(OBJDIR) -type f -exec rm -f {} \;
	$(RM) -f $(BINARY)

