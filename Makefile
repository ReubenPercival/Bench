CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
LDFLAGS ?= -lpthread
BIN     = bench
SRC     = bench.c

PREFIX  ?= /usr/local
BINDIR  = $(PREFIX)/bin

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) -o $(BIN) $(SRC) $(LDFLAGS)

install: $(BIN)
	install -d $(DESTDIR)$(BINDIR)
	install -m 755 $(BIN) $(DESTDIR)$(BINDIR)/$(BIN)

uninstall:
	rm -f $(DESTDIR)$(BINDIR)/$(BIN)

clean:
	rm -f $(BIN)

.PHONY: all install uninstall clean
