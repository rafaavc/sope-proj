CC=gcc
_DEPS = logger.h
_OBJ = simpledu.o logger.o 
OBJDIR = obj
SRCDIR = src
PROGNAME = simpledu

OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $<

$(PROGNAME): $(OBJ)
	$(CC) -o $@ $^

.PHONY: clean

clean: $(OBJ)
	rm $(OBJ) $(PROGNAME)
