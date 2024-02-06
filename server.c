#include "package.h"

// function declarations
void creation_of_socket();
void initializing_the_server_address();
void binding_the_server();
void listen_to_server();
int accept_the_client();
void *receive_message_from_client(void *);
int get_the_index_of_target_fd(int);
void handle_client_data(char *, int);
int check_if_same_user_exists(char *);
int check_that_particular_FD_in_array(int);
void gather_all_user_name(char *);
int get_the_fd_using_to_user_name(char *);
void concatenate_message(char *, char *, char *, char *);
void handle_info_tag();
void *handle_info_tag_in_thread();

// declaring the structure for clients data
typedef struct
{
    int user_id;
    int dest_sock_fd;
    int is_exists;
    int is_signedIn_helper;
    char user_name[USER_NAME_LIMIT + 1];
} Clients;

Clients client_data[LIMIT_OF_NODES];

struct sockaddr_in server_addr;
struct sockaddr_in client_address;

// declare variable for socket file decsriptor
int main_socket_server;
int helper_fd_received = 0;

int main()
{
    // create a socket
    creation_of_socket();

    // Initialize the server address
    initializing_the_server_address();

    // Bind the server
    binding_the_server();

    // Listen to the server
    listen_to_server();

    // create the thread for info tag
    handle_info_tag();

    // send and receive message from clients
    while (1 < 2)
    {
        int accepted_fd = accept_the_client();
        if (accepted_fd > 0)
        {
            printf("New connection established: %d\n", accepted_fd);
            pthread_t pth_client;
            pthread_create(&pth_client, NULL, (void *)receive_message_from_client, &accepted_fd);
            sleep(0.2);
        }
    }
}

// handle info tag in thread
void handle_info_tag()
{
    pthread_t info_thread;
    pthread_create(&info_thread, NULL, (void *)handle_info_tag_in_thread, NULL);
    sleep(0.2);
}

// Handle info tag to send message for clients
void *handle_info_tag_in_thread()
{
    while (1)
    {
        char info_message[BUFFER];
        memset(info_message, 0, BUFFER);
        fgets(info_message, BUFFER, stdin);
        info_message[strlen(info_message) - 1] = '\0';

        char *info_tag_message = fetch_data_from_tags(info_message, INFO_START, INFO_END);
        if (info_tag_message != NULL)
        {
            int i;
            while (i < LIMIT_OF_NODES)
            {
                if (client_data[i].is_exists)
                {
                    char send_buffer_info_message[BUFFER];
                    memset(send_buffer_info_message, 0, BUFFER);
                    snprintf(send_buffer_info_message, BUFFER, "%s : %s", "Message from server:", info_tag_message);

                    send(client_data[i].dest_sock_fd, send_buffer_info_message, strlen(send_buffer_info_message), 0);
                }

                i++;
            }
        }
    }
}

// receive message from the respective clients
void *receive_message_from_client(void *args)
{

    int accepted_fd = *(int *)args;
    int i = 0;
    while (i < LIMIT_OF_NODES)
    {
        if (client_data[i].dest_sock_fd == 0)
        {
            if (!check_that_particular_FD_in_array(accepted_fd))
            {
                client_data[i].dest_sock_fd = accepted_fd;
                break;
            }
        }
        i++;
    }
    while (1)
    {

        // Receive the message from server
        char receive_buffer[BUFFER];
        memset(receive_buffer, 0, BUFFER);
        ssize_t received_length = recv(accepted_fd, receive_buffer, BUFFER, 0);
        if (received_length > 0)
        {
            // get the index of fd
            int index = get_the_index_of_target_fd(accepted_fd);

            // Handle data which was received from the client
            handle_client_data(receive_buffer, index);
        }
    }
}

// check that FD is there is in the array
int check_that_particular_FD_in_array(int clientFD)
{
    for (int i = 0; i < LIMIT_OF_NODES; i++)
    {
        if (client_data[i].dest_sock_fd == clientFD)
        {
            return 1;
        }
    }
    return 0;
}

// Get the index for which client connected.
int get_the_index_of_target_fd(int accepted_client_fd)
{
    for (int i = 0; i < LIMIT_OF_NODES; i++)
    {
        if (client_data[i].dest_sock_fd == accepted_client_fd)
        {
            return i;
        }
    }
    return 0;
}

// creating the socket
void creation_of_socket()
{
    main_socket_server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (main_socket_server == -1)
    {
        perror("Error : Socket Initializaton Failed\n");
        exit(-1);
    }
}

// Initialize the server address
void initializing_the_server_address()
{
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NO);
    server_addr.sin_addr.s_addr = INADDR_ANY;
}

// Binding the server
void binding_the_server()
{
    if ((bind(main_socket_server, (struct sockaddr *)&server_addr, sizeof(server_addr))) < 0)
    {
        perror("Error: Cannot bind to the server\n");
        exit(-1);
    }
}

// listen to the server
void listen_to_server()
{
    if (listen(main_socket_server, LIMIT_OF_NODES) == -1)
    {
        perror("Error : Listening Failed\n");
    }
    else
    {
        printf("Listening on port : %d \n", PORT_NO);
    }
}

// accept the client
int accept_the_client()
{
    socklen_t client_length = sizeof(client_address);
    int accept_fd = accept(main_socket_server, (struct sockaddr *)&client_address, &client_length);
    if (accept_fd == -1)
    {
        perror("Error while accepting the client\n");
        exit(-1);
    }
    return accept_fd;
}

// Handling the message which was sent from the client
void handle_client_data(char *message, int pos)
{

    // check whether the message received is from Helper node

    if (strcmp(message, HELPER_MESSAGE) == 0)
    {
        helper_fd_received = client_data[pos].dest_sock_fd;
        for (int i = 0; i < LIMIT_OF_NODES; i++)
        {
            client_data[i].is_signedIn_helper = 1;
            return;
        }
    }

    // check for login tag
    char *user_name_of_client = fetch_data_from_tags(message, LOGIN_START, LOGIN_END);
    if (user_name_of_client != NULL)
    {
        if ((check_if_same_user_exists(user_name_of_client)) == 0)
        {
            printf("User Successfully Logged In : %s\n", user_name_of_client);
            client_data[pos].is_exists = 1;
            strcat(client_data[pos].user_name, user_name_of_client);
            // send success message to the client
            send(client_data[pos].dest_sock_fd, SUCCESS, strlen(SUCCESS), 0);
        }
        else
        {
            // send Failure Message to the client
            send(client_data[pos].dest_sock_fd, FAILURE, strlen(FAILURE), 0);
        }
    }

    // Handle Login list
    if (strncmp(message, LOGINLIST_OPEN, strlen(LOGINLIST_OPEN)) == 0)
    {
        char all_user_names[BUFFER];
        gather_all_user_name(all_user_names);
        send(client_data[pos].dest_sock_fd, all_user_names, strlen(all_user_names), 0);
    }

    // Handle Logout
    if (strncmp(message, LOGOUT, strlen(LOGOUT)) == 0)
    {
        printf("User Logged out Successfully : %s\n", client_data[pos].user_name);

        // remove files starting with that user
        char user_data[BUFFER];
        memset(user_data, 0, BUFFER);
        snprintf(user_data, BUFFER, "rm -rf %s*.txt", client_data[pos].user_name);
        system(user_data);

        client_data[pos].is_exists = 0;
        strcpy(client_data[pos].user_name, "");
        close(client_data[pos].dest_sock_fd);
    }

    // Handle Message Data
    char *to_user_name = fetch_data_from_tags(message, TO_START, TO_END);
    char *from_user_name = fetch_data_from_tags(message, FROM_START, FROM_END);
    char *body_message = fetch_data_from_tags(message, BODY_START, BODY_END);

    if (to_user_name != NULL && body_message != NULL)
    {
        int destination_fd = get_the_fd_using_to_user_name(to_user_name);
        if (destination_fd > 0)
        {
            char send_message_body[BUFFER];
            memset(send_message_body, 0, BUFFER);
            concatenate_message(send_message_body, from_user_name, body_message, to_user_name);
            send(destination_fd, send_message_body, strlen(send_message_body), 0);
        }
        else
        {
            printf("Destination User Not there to send message\n");
        }
    }
}

// concatenate message
void concatenate_message(char *send_message_body, char *from_user, char *body_message, char *to_user)
{
    // check if helpernode is signed in
    if (helper_fd_received)
    {
        send(helper_fd_received, body_message, strlen(body_message), 0); // send to helpernode
        // now receive message
        int no_of_characters_received = recv(helper_fd_received, body_message, strlen(body_message), 0);
    }

    // save the message to file
    char user_data[BUFFER];
    memset(user_data, 0, BUFFER);
    snprintf(user_data, BUFFER, "%s%s.txt", from_user, to_user);
    FILE *fp = fopen(user_data, "a");
    fprintf(fp, "%s\n", body_message);
    fclose(fp);

    strcat(send_message_body, from_user);
    strcat(send_message_body, ">>= ");
    strcat(send_message_body, body_message);
}

// get the FD
int get_the_fd_using_to_user_name(char *to_user)
{
    int i = 0;
    while (i < LIMIT_OF_NODES)
    {
        if (strncmp(to_user, client_data[i].user_name, USER_NAME_LIMIT) == 0)
        {
            return client_data[i].dest_sock_fd;
        }
        i++;
    }
    return 0;
}

// Gather all usernames
void gather_all_user_name(char *all_user_names)
{

    memset(all_user_names, 0, BUFFER);
    strcat(all_user_names, "LOGGED IN USERS IN THE SERVER ARE:\n");
    for (int i = 0; i < LIMIT_OF_NODES; i++)
    {
        if (client_data[i].is_exists == 1)
        {
            strcat(all_user_names, client_data[i].user_name);
            strcat(all_user_names, " ");
        }
    }
}

// check if same user exists
int check_if_same_user_exists(char *username)
{
    int i = 0;
    while (i < LIMIT_OF_NODES)
    {
        if (client_data[i].dest_sock_fd && client_data[i].is_exists)
        {
            if (strcmp(username, client_data[i].user_name) == 0)
            {
                return 1;
            }
        }

        i++;
    }
    return 0;
}