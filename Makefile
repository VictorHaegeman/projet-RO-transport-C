CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = src/main.c src/problem.c src/io.c src/utils.c \
      src/nord_ouest.c src/balas_hammer.c \
      src/potentiel.c src/marche_pied.c \
      src/base_affiche.c

OBJ = $(SRC:.c=.o)

transport: $(OBJ)
	$(CC) $(CFLAGS) -o transport $(OBJ)

clean:
	rm -f $(OBJ) transport
