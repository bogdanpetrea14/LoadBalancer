/* Copyright 2023 Boggdan-Vasile Petrea */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "load_balancer.h"
#include "server.h"
#include "Hashtable.h"
#include "LinkedList.h"

#define SERVER_INIT 999
#define COPY 100000
#define NO 3

struct load_balancer {
    server_memory *server[SERVER_INIT];
    int size_server;
    unsigned int *hashring;
    int hashring_size;
};

unsigned int hash_function_servers(void *a)
{
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a)
{
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

load_balancer *init_load_balancer()
{
    load_balancer *lb = (load_balancer *)malloc(sizeof(load_balancer));
    DIE(!lb, "Cannot alloc memory for the load balancer");

    lb->size_server = 0;
    lb->hashring_size = 0;
    lb->hashring = malloc(NO * SERVER_INIT * sizeof(unsigned int));
    DIE(!lb->hashring, "Cannot alloc memory for the hashring");
    return lb;
}

int which_server(load_balancer *main, char *key)
{
    int pos = 0;
    if (hash_function_key(key) >
        hash_function_servers(&main->hashring[main->hashring_size - 1])) {
        pos = 0;
        for (int j = 0; j < main->size_server; j++) {
            if (main->server[j]->id_server == (int)main->hashring[pos] % COPY)
                return j;
        }
    }
    for (int i = 0; i < main->hashring_size; i++) {
        if (hash_function_key(key) <
            hash_function_servers(&main->hashring[i])) {
            pos = i;
            for (int j = 0; j < main->size_server; j++) {
                if (main->server[j]->id_server ==
                    (int)main->hashring[pos] % COPY)
                    return j;
            }
        }
    }
    return 0;
}

int find_next_server(load_balancer *main, int server_id)
{
    int which = 0;
    for (int i = 0; i < main->hashring_size; i++) {
        if (main->hashring[i] == (unsigned int)server_id) {
            if (i + 3 < main->hashring_size) {
                if ((int)main->hashring[i + 1] % COPY != server_id % COPY) {
                    which = i + 1;
                    return which;
                }
                if ((int)main->hashring[i + 2] % COPY != server_id % COPY) {
                    which = i + 2;
                    return which;
                } else {
                    which = i + 3;
                    return which;
                }
            }
            if (i + 2 < main->hashring_size) {
                if ((int)main->hashring[i + 1] % COPY != server_id % COPY) {
                    which = i + 1;
                    return which;
                }
                if ((int)main->hashring[i + 2] % COPY != server_id % COPY) {
                    which = i + 2;
                    return which;
                } else {
                    which = 0;
                    return which;
                }
            }
            if (i + 1 < main->hashring_size) {
                if ((int)main->hashring[i + 1] % COPY != server_id % COPY) {
                    which = i + 1;
                    return which;
                }
                if ((int)main->hashring[0] % COPY != server_id % COPY) {
                    which = 0;
                    return which;
                } else {
                    which = 1;
                    return which;
                }
            }
            if (i == main->hashring_size - 1) {
                if ((int)main->hashring[0] % COPY != server_id % COPY) {
                    which = 0;
                    return which;
                }
                if ((int)main->hashring[1] % COPY != server_id % COPY) {
                    which = 1;
                    return which;
                } else {
                    which = 2;
                    return which;
                }
            }
        }
    }
    return 0;
}

void keep_server_sortated(load_balancer *main, unsigned int eticheta, int pos)
{
    for (int i = main->hashring_size; i > pos; i--)
        main->hashring[i] = main->hashring[i - 1];
    main->hashring[pos] = eticheta;
}

void verify_order(load_balancer *main)
{
    int size = main->hashring_size;
    unsigned int aux;
    for (int i = 0; i < size - 1; i++) {
        for (int j = i + 1; j < size; j++) {
            if (hash_function_servers(&main->hashring[i]) >
                hash_function_servers(&main->hashring[j])) {
                aux = main->hashring[i];
                main->hashring[i] = main->hashring[j];
                main->hashring[j] = aux;
            }
        }
    }
}

void rem_serv_hash(load_balancer *main, int pos)
{
    for (int i = pos; i < main->hashring_size; i++)
        main->hashring[i] = main->hashring[i + 1];
}

void rem_serv(load_balancer *main, int pos)
{
    for (int i = pos; i < main->size_server; i++)
        main->server[i] = main->server[i + 1];
}

void check_whole_balance(load_balancer *main)
{
    for (int i = 0; i < main->size_server; i++) {
        for (unsigned int j = 0; j < main->server[i]->hmax; j++) {
            ll_node_t *cur = main->server[i]->buckets[j]->head;

            while (cur) {
                char *ckey = ((struct info *)cur->data)->key;
                char *cvalue = ((struct info *)cur->data)->value;
                int what = which_server(main, ckey);
                if (what != i) {
                    server_store(main->server[what], ckey, cvalue);
                }
                cur = cur->next;
            }
        }
    }
}

void rebalance(load_balancer *main, int server_id)
{
    int next = find_next_server(main, server_id);
    int poz_from_move = 0, id;
    for (int i = 0; i < main->size_server; i++) {
        if (server_id % COPY == main->server[i]->id_server) {
            id = main->server[i]->id_server;
        }
        if ((int)main->hashring[next] % COPY == main->server[i]->id_server) {
            poz_from_move = i;  // de unde mutam
        }
    }
    for (unsigned int i = 0; i < main->server[poz_from_move]->hmax; i++) {
        ll_node_t *cur = main->server[poz_from_move]->buckets[i]->head;
        ll_node_t *node;
        while (cur) {
            node = cur;
            cur = cur->next;
            char *ckey = ((struct info *)node->data)->key;
            char *cvalue = ((struct info *)node->data)->value;
            if (hash_function_key(&ckey) < hash_function_servers(&id)) {
                int which = which_server(main, ckey);
                server_store(main->server[which], ckey, cvalue);
            }
        }
    }
}

void loader_add_server(load_balancer *main, int server_id)
{
    server_memory *server = init_server_memory();
    int index = main->size_server;  // adaug un server pe ult poz din ring
    main->server[index] = server;
    main->server[index]->id_server = server_id;
    main->size_server++;
    for (int i = 0; i < NO; i++) {
        int eticheta = i * COPY + server_id;
        unsigned int hash = hash_function_servers(&eticheta);

        if (main->hashring_size == 0) {
            main->hashring[main->hashring_size] = eticheta;
            main->hashring_size++;
            continue;
        }

        if (hash_function_servers(&main->hashring[main->hashring_size - 1]) <
            hash && main->hashring_size) {
            main->hashring[main->hashring_size] = eticheta;
            main->hashring_size++;
            continue;
        }

        for (int j = 0 ; j < main->hashring_size; j++) {
            if (hash_function_servers(&main->hashring[j]) > hash) {
                keep_server_sortated(main, eticheta, j);
                main->hashring_size++;
                break;
            }
        }
        rebalance(main, eticheta);
    }
    check_whole_balance(main);
}

void loader_remove_server(load_balancer *main, int server_id)
{
    int id;
    for (int k = 0; k < NO; k++) {
        for (int i = 0; i < main->hashring_size; i++) {
            id = main->hashring[i] % COPY;
            if (id == server_id % COPY) {
                rem_serv_hash(main, i);
                main->hashring_size--;
            }
        }
        for (int i = 0; i < main->size_server; i++) {
            int ID = main->server[i]->id_server;
            if (ID == server_id) {
                for (unsigned int j = 0; j < main->server[i]->hmax; j++) {
                    ll_node_t *cur = main->server[i]->buckets[j]->head;

                    while (main->server[i]->buckets[j]->size) {
                        char *ckey = ((struct info *)cur->data)->key;
                        char *cvalue = ((struct info *)cur->data)->value;
                        int what = which_server(main, ckey);
                        server_store(main->server[what], ckey, cvalue);
                        server_remove(main->server[i], ckey);
                    }
                }
                free_server_memory(main->server[i]);
                rem_serv(main, i);
                main->size_server--;
                break;
            }
        }
    }
}

void store(load_balancer *main, char *key,
           char *value, int *server_id, int which)
{
    int id = main->hashring[which] % COPY;
    for (int j = 0; j < main->size_server; j++) {
        if (id == main->server[j]->id_server) {
            server_store(main->server[j], key, value);
            *server_id = id;
            return;
        }
    }
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
    int which;
    if (hash_function_servers(&main->hashring[main->hashring_size - 1]) <
        hash_function_key(key)) {
        which = 0;
        store(main, key, value, server_id, 0);
        return;
    }
    for (int i = 0; i < main->hashring_size; i++) {
        if (hash_function_key(key) <
            hash_function_servers(&main->hashring[i])) {
            which = i;
            store(main, key, value, server_id, which);
            return;
        }
    }
}

char* retrieve(load_balancer *main, char *key, int *server_id, int which)
{
    int id = main->hashring[which] % COPY;
    for (int j = 0; j < main->size_server; j++) {
        if (id == main->server[j]->id_server) {
            *server_id = id;
            return server_retrieve(main->server[j], key);
        }
    }
    return NULL;
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
    int which;
    if (hash_function_servers(&main->hashring[main->hashring_size - 1]) <
        hash_function_key(key)) {
        which = 0;
        return retrieve(main, key, server_id, which);
    }
    for (int i = 0; i < main->hashring_size; i++) {
        if (hash_function_key(key) <
            hash_function_servers(&main->hashring[i])) {
            which = i;
            return retrieve(main, key, server_id, which);
        }
    }
    return NULL;
}

void free_load_balancer(load_balancer *main)
{
    for (int i = 0; i < main->size_server; i++) {
        free_server_memory(main->server[i]);
    }
    free(main->hashring);
    free(main);
}
