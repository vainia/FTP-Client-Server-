CC=gcc -Wall -pedantic
SRC=Client/client
SRC2=Server/server
all: $(SRC).c
	$(CC) -o $(SRC) $(SRC).c
	$(CC) -o $(SRC2) $(SRC2).c
clean:
	rm $(SRC) $(SRC2)
