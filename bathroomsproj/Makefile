CC=gcc
_DEPS = cmdargs.h opreg.h
_ServerOBJ = q.o cmdargs.o opreg.o
_ClientOBJ = u.o cmdargs.o opreg.o
OBJDIR = obj
SRCDIR = src
ServerNAME = Q1
ClientNAME = U1
CFLAGS = -Wall -pthread -lrt #-lm

ServerOBJ = $(patsubst %,$(OBJDIR)/%,$(_ServerOBJ))
ClientOBJ = $(patsubst %,$(OBJDIR)/%,$(_ClientOBJ))
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	@mkdir -p $(OBJDIR)
	@$(CC) -c -o $@ $< $(CFLAGS)

all: $(ServerNAME) $(ClientNAME)
	@echo done

$(ServerNAME): $(ServerOBJ)
	@$(CC) -o $@ $^ $(CFLAGS)
	
$(ClientNAME): $(ClientOBJ)
	@$(CC) -o $@ $^ $(CFLAGS)


.PHONY: clean

clean: $(OBJDIR) $(ServerNAME) $(ClientNAME)
	@rm -f -r $^
