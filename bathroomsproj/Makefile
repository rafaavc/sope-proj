CC=gcc
_DEPS = cmdargs.h
_ServerOBJ = Q1.o cmdargs.o
_ClientOBJ = U1.o cmdargs.o
OBJDIR = obj
SRCDIR = src
ServerNAME = Q1
ClientNAME = U1
CFLAGS = -Wall -pthread #-lm

ServerOBJ = $(patsubst %,$(OBJDIR)/%,$(_ServerOBJ))
ClientOBJ = $(patsubst %,$(OBJDIR)/%,$(_ClientOBJ))
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $< $(CFLAGS)

all: $(ServerNAME) $(ClientNAME)

$(ServerNAME): $(ServerOBJ)
	@$(CC) -o $@ $^ $(CFLAGS)
	
$(ClientNAME): $(ClientOBJ)
	@$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean

clean: $(OBJDIR) $(ServerNAME) $(ClientNAME)
	@rm -f -r $^
