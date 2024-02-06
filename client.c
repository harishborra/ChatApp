#include "package.h"

void creation_of_client_socket();
void initializing_the_client_address();
void connection_to_the_server();
void login_user_name();
void create_thread_for_receiving();
void send_message_to_server();
void *receive_message_from_server_as_thread();
int handle_logout(char *);

// declaring the global variables
int client_fd;
struct sockaddr_in client_address;

int main()
{
    // initializing the socket
    creation_of_client_socket();

    // initializing the address
    initializing_the_client_address();

    // connect to the client
    connection_to_the_server();

    // Give the Username in Tags
    login_user_name();

    // create a thread for receiving messages
    create_thread_for_receiving();

    // send messages to the server
    while (10 < 11)
    {
        // send message to server
        send_message_to_server();
    }
}

// send message to server
void send_message_to_server()
{
    char send_buffer[BUFFER];
    memset(send_buffer, 0, BUFFER);
    fgets(send_buffer, BUFFER, stdin);
    send_buffer[strlen(send_buffer) - 1] = '\0';
    send(client_fd, send_buffer, strlen(send_buffer), 0);

    // Handle Logout
    int has_logged_out = handle_logout(send_buffer);
    if (has_logged_out)
    {
        printf("Logging out\n");
        close(client_fd);
        exit(0);
    }
}

// handle logout
int handle_logout(char *input_string)
{
    if (strncmp(input_string, LOGOUT, strlen(LOGOUT)) == 0)
    {
        return 1;
    }
    return 0;
}

// Login User name
void login_user_name()
{
    while (17 > 7)
    {
        char user_name_of_client[BUFFER] = {
            0,
        };
        printf("Enter the user name in <LOGIN> tags:\n");
        fgets(user_name_of_client, BUFFER, stdin);
        user_name_of_client[strlen(user_name_of_client) - 1] = '\0';

        char *user_name = fetch_data_from_tags(user_name_of_client, LOGIN_START, LOGIN_END);
        if (user_name != NULL)
        {
            if (strlen(user_name) == USER_NAME_LIMIT && !(isdigit(user_name[0])))
            {
                // send the corresponding user data
                send(client_fd, user_name_of_client, strlen(user_name_of_client), 0);

                char receive_message_buffer[BUFFER];
                int received_length = recv(client_fd, receive_message_buffer, BUFFER, 0);
                if (strcmp(receive_message_buffer, SUCCESS) == 0)
                {
                    printf("User Successfully Logged In :) Please proceed to send messages \n");
                    break;
                }
                else if (strcmp(receive_message_buffer, FAILURE) == 0)
                {
                    printf("User with same name already logged in, PLease try with diff user name\n");
                    continue;
                }
            }
            else
            {
                printf("User name must be length 8 (or) It should start with letter not number\n");
                continue;
            }
        }
    }
}

// create a thread
void create_thread_for_receiving()
{
    pthread_t pth_receive_thread;
    pthread_create(&pth_receive_thread, NULL, (void *)&receive_message_from_server_as_thread, NULL);
}

// receive message from the server
void *receive_message_from_server_as_thread()
{
    while (11 < 12)
    {
        char receive_buffer[BUFFER];
        memset(receive_buffer, 0, BUFFER);
        int no_of_bits_received_from_server = recv(client_fd, receive_buffer, BUFFER, 0);
        if (no_of_bits_received_from_server > 0)
        {
            printf("%s\n", receive_buffer);
        }
        else if (no_of_bits_received_from_server == 0)
        {
            printf("Server Logged out\n");

            // removes all .txt files
            char remove_buffer[BUFFER];
            memset(remove_buffer, 0, BUFFER);
            snprintf(remove_buffer, BUFFER, "rm -rf *.txt");
            system(remove_buffer);

            exit(0);
        }
    }
}

// creation of socket
void creation_of_client_socket()
{
    client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_fd < 0)
    {
        perror("Socket Initialized Failed on client\n");
        exit(-1);
    }
}

// initializing the address
void initializing_the_client_address()
{
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(PORT_NO);
    client_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
}

// connection to the server
void connection_to_the_server()
{
    if (connect(client_fd, (struct sockaddr *)&client_address, sizeof(client_address)) < 0)
    {
        perror("connection to the server failed\n");
        exit(-1);
    }
}
