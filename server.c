/* Copyright 2023 Bogdan-Vasile Petrea */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "LinkedList.h"
#include "Hashtable.h"
#include "load_balancer.h"

#define MAX_STRING_SIZE	256
#define HMAX 1500

server_memory* init_server_memory()
{
	server_memory *server;
	server = ht_create(HMAX, hash_function_key, compare_function_strings);
	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	ht_put(server, key, strlen(key) + 1, value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
	return ht_get(server, key);
}

void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server, key);
}

void free_server_memory(server_memory *server) {
	ht_free(server);
}
