CC=clang
BIN=gamerd
CSTD=c99
FRAMEWORK=-framework IOKit -framework ApplicationServices
SOURCE=gamerd.m

all:
	$(CC) -std=$(CSTD) -o $(BIN) $(SOURCE) $(FRAMEWORK)
