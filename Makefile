CC = clang
CFLAGS = -std=c99 -O0 -g
LDFLAGS = -lm

ifeq ($(CODEREVIEW), yes)
	# Enable EVERYTHING.
	CFLAGS += -Weverything
	# We are using C
	CFLAGS += -Wno-unsafe-buffer-usage
	# We don't care about trailing ;
	CFLAGS += -Wno-extra-semi-stmt
	# We are using C99
	CFLAGS += -Wno-declaration-after-statement
	# I honestly don't care right know
	CFLAGS += -Wno-padded
	# I don't care
	CFLAGS += -Wno-date-time
	# I don't care
	CFLAGS += -Wno-float-equal
endif

ifeq ($(SANITIZERS), yes)
	CFLAGS += -fsanitize=undefined,address,leak
	LDFLAGS += -fsanitize=undefined,address,leak
endif

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
