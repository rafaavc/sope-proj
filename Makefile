CC=gcc
_DEPS = logger.h
_OBJ = simpledu.o logger.o 
OBJDIR = obj
SRCDIR = src

OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
DEPS = $(patsubst %,$(SRCDIR)/%,$(_DEPS))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $<

simpledu: $(OBJ)
	$(CC) -o $@ $^
