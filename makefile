CC = gcc

CFLAGS = -std=c99 -Wall -Wextra -pedantic
GTK_CFLAGS = `pkg-config --cflags gtk+-3.0`
GTK_LIB = `pkg-config --libs gtk+-3.0`

SRC_DIR = src
BLD_DIR = build
INC_DIR = include

SRC = $(shell find $(SRC_DIR) -type f -name *.c)
OBJ = $(patsubst $(SRC_DIR)/%,$(BLD_DIR)/%,$(SRC:.c=.o))
EXEC = galendae

.PHONY: debug  
debug: CFLAGS+=-g -o0
debug: $(EXEC)

.PHONY: release
release: CFLAGS+=-o3
release: $(EXEC)

$(EXEC): $(OBJ)
	@echo "Linking..."
	$(CC) $^ -o $(EXEC) $(GTK_LIB)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BLD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -I $(INC_DIR) -c -o $@ $<

.PHONY: clean
clean:
	@echo "Cleaning..."; 
	$(RM) -r $(BLD_DIR) $(EXEC)
