CC = clang
CFLAGS = -std=c23 -O0 -g -Wall -Wextra -Wunused -Wunreachable-code
LDFLAGS = -lm
SRC = $(wildcard src/**/*.c) $(wildcard src/*.c) $(wildcard src/**/**/*.c) $(wildcard src/**/**/**/*.c)
OBJ = $(SRC:.c=.o)
BINDIR = bin
BIN = prayertimes

.PHONY: all

all: dirs make

run: $(BINDIR)/$(BIN) 
	@$(BINDIR)/$(BIN)

$(BINDIR): dirs

dirs:
	@mkdir -p ./$(BINDIR)
	@echo "made dir $(BINDIR)"

make: $(OBJ) dirs
	@$(CC) $(OBJ) $(LDFLAGS) -o $(BINDIR)/$(BIN)
	@echo "made exe $(LIB_BIN)"

%.o: %.c
	@echo "compiled $<"
	@$(CC) -o $@ -c $< $(CFLAGS)
clean:
	@rm -rf $(OBJ) $(BINDIR)
	@echo "cleaned"
