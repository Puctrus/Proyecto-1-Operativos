/*
	C socket server example, handles multiple clients using threads
*/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h>   //inet_addr
#include <unistd.h>      //write
#include <pthread.h>     //for threading , link with lpthread
#include <json-c/json.h> //JSON

//the thread function
void *connection_handler(void *);

int main(int argc, char *argv[])
{

    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in server, client;

    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    //Bind
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        puts("Connection accepted");

        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;

        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( sniffer_thread , NULL);
        puts("Handler assigned");
    }

    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }

    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    struct json_object *parsed_json;
    struct json_object *name;
    struct json_object *age;
    struct json_object *friends;
    struct json_object *friend;
    size_t n_friends;

    size_t i;
    //Get the socket descriptor
    int sock = *(int *)socket_desc;
    int read_size;
    char *message;
    char client_message[2000];

    //Send some messages to the client
    message = " Greetings! I am your connection handler\n Now type something and i shall repeat what you type \n";
    write(sock, message, strlen(message));

    // message = "Now type something and i shall repeat what you type \n";
    // write(sock , message , strlen(message));

    //Receive a message from client

    while ((read_size = recv(sock, client_message, 2000, 0)) > 0)
    {
        

        //if(IsJsonString(client_message)
        parsed_json = json_tokener_parse(client_message);
        json_object_object_get_ex(parsed_json, "name", &name);
        json_object_object_get_ex(parsed_json, "age", &age);
        json_object_object_get_ex(parsed_json, "friends", &friends);

        printf("Name: %s\n", json_object_get_string(name));
        printf("Age: %d\n", json_object_get_int(age));

        n_friends = json_object_array_length(friends);
        printf("Found %lu friends\n", n_friends);

        for (i = 0; i < n_friends; i++)
        {
            friend = json_object_array_get_idx(friends, i);
            printf("%lu. %s\n", i + 1, json_object_get_string(friend));
        }

        //Send the message back to client
        puts("EL CLIENTE ENVIO :");
        puts(client_message);
        write(sock, client_message, strlen(client_message));
        memset(client_message, 0, sizeof(client_message));
    }

    if (read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if (read_size == -1)
    {
        perror("recv failed");
    }

    //Free the socket pointer
    free(socket_desc);

    return 0;
}