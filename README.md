## load_add_server

Initialize a server and add it to the server list, saving its ID. Then, generate the 3 corresponding labels for the server and add them to the hash ring, in ascending order based on their hash value (using the server's hash function). Finally, after adding the new server, call the rebalance function, which will check if any objects need to be moved to the new server. Before closing the function, double-check that the server labels are in ascending order on the hash ring, and verify that each object is located on the nearest server in clockwise order.

## loader_remove_server

First, remove the labels corresponding to the server to be removed from the hash ring. Then, search for the desired server in the server list and move everything from it to the next immediate server (meaning, look for the next label in the hash ring for each label of the initial server and move to the corresponding server).

## loader_store + store

Initially, find the server where the objects need to be stored, and then use the store function to actually do this.

## loader_retrieve + retrieve

Search the server list for the server that might contain the object. Once found, call the retrieve function to return the object itself.

## free_load_balancer

First, free the entire server list, then free the hash ring, and finally, free the load balancer.

## which_server

Searches the entire server list for the server that saves the key and returns its position.

## find_next_server

Searches for the next server for the remove function.

## check_whole_balance

Iterates through each object on servers and checks if it is positioned where it should be. If not, it will be moved.

## rebalance

Checks which objects need to be moved to a newly added server.
