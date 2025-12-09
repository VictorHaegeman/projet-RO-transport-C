CC = gcc
CFLAGS = -Wall -Wextra -g

COMMON_SRC = src/problem.c src/io.c src/utils.c \
             src/nord_ouest.c src/balas_hammer.c \
             src/potentiel.c src/marche_pied.c \
             src/base_affiche.c

TRANSPORT_SRC = src/main.c $(COMMON_SRC)
COMPLEXITE_SRC = src/complexite.c $(COMMON_SRC)

TRANSPORT_OBJ = $(TRANSPORT_SRC:.c=.o)
COMPLEXITE_OBJ = $(COMPLEXITE_SRC:.c=.o)

transport: $(TRANSPORT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TRANSPORT_OBJ)

complexite: $(COMPLEXITE_OBJ)
	$(CC) $(CFLAGS) -o $@ $(COMPLEXITE_OBJ)

clean:
	rm -f $(TRANSPORT_OBJ) $(COMPLEXITE_OBJ) transport complexite
