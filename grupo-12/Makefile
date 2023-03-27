all: sdstored sdstore

sdstored: sdstored.c auxServer.c
	gcc -o sdstored sdstored.c auxServer.c -I.

sdstore: sdstore.c auxClient.c
	gcc -o sdstore sdstore.c auxClient.c -I.

clean:
	rm sdstored sdstore log.bin log_limites.bin