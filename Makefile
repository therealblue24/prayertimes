# Prayertimes Makefile
# Copyright (C) 2025 therealblue24

# set this to 1 for release mode
RELEASE = 0 
# set this to your favorite compiler
CC = clang
# binary directory
BINDIR = bin
# app name
APP = prayertimes
# lib name
LIBNAM = libprayertimes.a

CFLAGS = -std=c99 -Wall -Wextra -Isrc -Iinclude -g 
LDFLAGS = -lm

RELEASE ?= no
CODE_REVIEW ?= no
SANITIZERS ?= no

ifeq ($(RELEASE), yes)
	CFLAGS += -O2
endif

ifeq ($(CODE_REVIEW), yes)
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

ifeq ($(SANTIZERS), yes)
	CFLAGS += -fsanitize=undefined,address,leak
	LDFLAGS += -fsanitize=undefined,address,leak
endif

SRC = $(wildcard src/*.c)
LIBSRC = $(filter-out src/config.c, $(filter-out src/main.c, $(SRC)))
OBJ = $(SRC:.c=.o)
LIBOBJ = $(LIBSRC:.c=.o)

.PHONY: dirs build test link

all: dirs build link

dirs:
	@# Create bin dir
	@mkdir -p $(BINDIR)

%.o: %.c
	@echo "compiling $<"
	@$(CC) -o $@ -c $< $(CFLAGS)

build: dirs $(OBJ)

link: build
	@echo "linking library"
	@ar rcs $(BINDIR)/$(LIBNAM)	$(LIBOBJ)
	@echo "made lib $(LIBNAM)"
	@echo "linking $(APP)"
	@$(CC) -o $(BINDIR)/$(APP) $(LDFLAGS) $(OBJ)
	@echo "made $(APP)"

clean:
	@echo "cleaning"
	@rm -rf $(OBJ) $(BINDIR)
	@echo "cleaned"
