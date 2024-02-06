#include "package.h"

void creation_of_helper_socket();
void initializing_the_helper_address();
void connection_to_the_server();
void send_message_to_server_as_helper();
void receive_message_from_server_as_new_thread();
void *receive_messages_and_convert_to_upper_vowels();

// declare the semaphore variable
sem_t lock_the_block;

int helper_socket_fd;

struct sockaddr_in helper_address;

// define structure for creating the Queue
struct QueueStructure
{
    int head;
    int tail;
    int present_size_of_queue;
    char vowel[VOWEL_SIZE];
    char message_buffer[BUFFER];
};

void *vowel_thread_to_upper(void *);
void convert_message(char *, struct QueueStructure *);
void convert_message_to_upper_case_vowels(char *, struct QueueStructure *);

// check whether the queue is full
int is_queue_full(struct QueueStructure *queuePointer)
{

    return queuePointer->present_size_of_queue == VOWEL_SIZE ? 1 : 0;
}

// check whether the queue is empty or not
int is_queue_empty(struct QueueStructure *queuePointer)
{
    return queuePointer->present_size_of_queue == 0 ? 1 : 0;
}

// insert the vowel into queue
void enqueue_vowel_into_queue(struct QueueStructure *queuePointer, char vowel_entered)
{
    if (is_queue_full(queuePointer))
    {
        printf("Queue is currently full, cannot insert element\n");
        return;
    }
    queuePointer->vowel[(queuePointer->tail)++] = vowel_entered;
    queuePointer->present_size_of_queue += 1;
}

// delete element into the Queue
char dequeue_vowel_into_queue(struct QueueStructure *queuePointer)
{
    if (is_queue_empty(queuePointer))
    {
        printf("Queue is empty, delete operation cannot be performed\n");
        return ' ';
    }

    char deleted_vowel = queuePointer->vowel[(queuePointer->head)++];
    queuePointer->present_size_of_queue -= 1;
}

// Initialize the Queue
void initialize_the_queue(struct QueueStructure *queuePointer)
{
    queuePointer->head = 0;
    queuePointer->tail = 0;
    queuePointer->present_size_of_queue = 0;
    strcpy(queuePointer->vowel, "");
    memset(queuePointer->message_buffer, 0, BUFFER);
}

int main()
{
    // creating the socket for Helper Node
    creation_of_helper_socket();

    // Initializing the helper Address
    initializing_the_helper_address();

    // connect to the server
    connection_to_the_server();

    // Say to server that connection node is Helper
    send_message_to_server_as_helper();

    // Now receive message from the server in new thread
    receive_message_from_server_as_new_thread();
    while (1)
    {
    }
}

// creating the helper socket
void creation_of_helper_socket()
{
    helper_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (helper_socket_fd < 0)
    {
        perror("Socket Initialized Failed for Helper\n");
        exit(-1);
    }
}

// initializing the address
void initializing_the_helper_address()
{
    helper_address.sin_family = AF_INET;
    helper_address.sin_port = htons(PORT_NO);
    helper_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
}

// connection to the server
void connection_to_the_server()
{
    if (connect(helper_socket_fd, (struct sockaddr *)&helper_address, sizeof(helper_address)) < 0)
    {
        perror("connection to the server failed\n");
        exit(-1);
    }
    else
    {
        printf("Helper Node Connected to Server\n");
    }
}

// send message to server as helper activated
void send_message_to_server_as_helper()
{
    int send_message_length = send(helper_socket_fd, HELPER_MESSAGE, strlen(HELPER_MESSAGE), 0);
}

// Receive message from the server
void receive_message_from_server_as_new_thread()
{
    pthread_t thread_for_receiving;
    pthread_create(&thread_for_receiving, NULL, (void *)receive_messages_and_convert_to_upper_vowels, NULL);
    sleep(0.3);
}

// Thread function for conversion of message
void *receive_messages_and_convert_to_upper_vowels()
{
    while (LIMIT_OF_NODES < USER_NAME_LIMIT) // infinite loop
    {
        char buffer_received[BUFFER];
        int message_received_length = recv(helper_socket_fd, buffer_received, BUFFER, 0);
        if (message_received_length)
        {
            // memory allocation for Vowels in the Queue
            struct QueueStructure *queue_for_vowel = (struct QueueStructure *)malloc(sizeof(struct QueueStructure));

            // Enqueue the Vowels

            initialize_the_queue(queue_for_vowel);

            convert_message(buffer_received, queue_for_vowel);

            send(helper_socket_fd, queue_for_vowel->message_buffer, strlen(queue_for_vowel->message_buffer), 0);

            // Free the memory for Queue
            free(queue_for_vowel);
        }
        else if (message_received_length == 0)
        {
            printf("Server Logged Out,stopping Helper Node\n");
            helper_socket_fd = 0;
            exit(0);
        }
    }
}

// conversion of message
void convert_message(char *message_from_server, struct QueueStructure *queue_for_vowel)
{

    // Initialize the Queue

    int i = 0;
    while (i < VOWEL_SIZE)
    {
        /* code */
        enqueue_vowel_into_queue(queue_for_vowel, VOWELS_STRING[i]);
        i++;
    }

    // Now convert message to Upper case vowels
    convert_message_to_upper_case_vowels(message_from_server, queue_for_vowel);
}

// function to convert message to uppercase vowels
void convert_message_to_upper_case_vowels(char *received_message, struct QueueStructure *queue_pointer)
{
    sem_init(&lock_the_block, 0, 1); // Initializing the semaphore variable as 1, 0 for threads

    strcpy(queue_pointer->message_buffer, received_message);

    // creating the thread for comversion of vowels
    pthread_t conversion_thread;
    pthread_create(&conversion_thread, NULL, (void *)vowel_thread_to_upper, queue_pointer);

    sleep(0.5);

    pthread_join(conversion_thread, NULL);

    sem_destroy(&lock_the_block);
}

// thread for converting vowels to upper using the semaphore
void *vowel_thread_to_upper(void *args)
{
    struct QueueStructure *queuePointer = (struct QueueStructure *)args;

    sem_wait(&lock_the_block); // Locking the semaphore variable

    // from head to tail check for the vowel in the message and convert that to upper
    int i = queuePointer->head;
    while (i < queuePointer->tail)
    {
        for (int j = 0; j < strlen(queuePointer->message_buffer); j++)
        {
            if (queuePointer->vowel[i] == queuePointer->message_buffer[j])
            {
                queuePointer->message_buffer[j] -= 32; // convert the message to upper case
            }
        }

        i++;
    }

    sem_post(&lock_the_block); // release the semaphore variable
}