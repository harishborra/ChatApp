#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <semaphore.h>

#define PORT_NO 8040
#define IP_ADDRESS "127.0.0.1"
#define LIMIT_OF_NODES 7
#define USER_NAME_LIMIT 8
#define BUFFER 3072
#define VOWEL_SIZE 5
#define POSITIVE_VAL 1
#define VOWELS_STRING "aeiou"
#define LOGIN_START "<LOGIN>"
#define LOGIN_END "</LOGIN>"
#define SUCCESS "Login success"
#define FAILURE "Login Failure"
#define LOGOUT "<LOGOUT>"
#define LOGINLIST_OPEN "<LOGIN_LIST>"
#define LOGINLIST_CLOSE "</LOGIN_LIST>"
#define TO_START "<TO>"
#define TO_END "</TO>"
#define INFO_START "<INFO>"
#define INFO_END "</INFO>"
#define HELPER_MESSAGE "HELPER NODE CONNECTED"
#define FROM_START "<FROM>"
#define FROM_END "</FROM>"
#define BODY_START "<BODY>"
#define BODY_END "</BODY>"

// Fetch the data between Tags
char *fetch_data_from_tags(char *input, char *start_tag, char *end_tag)
{
    if (POSITIVE_VAL)
    {
        int start_tag_len = strlen(start_tag);
        int end_tag_len = strlen(end_tag);

        int flag_start = (start_tag_len > 0) ? 1 : 0;
        int flag_end = (start_tag_len > 0) ? 1 : 0;

        if (flag_start && flag_end)
        {

            char *start_tag_position = strstr(input, start_tag);
            if (start_tag_position == NULL)
            {
                return NULL;
            }

            char *end_tag_position = strstr(start_tag_position + strlen(start_tag), end_tag);
            if (end_tag_position == NULL)
            {
                return NULL;
            }

            int extract_string_length = end_tag_position - (start_tag_position + strlen(start_tag));
            char *extracted_string = malloc(extract_string_length + 1);
            if (extracted_string == NULL)
            {
                return NULL;
            }

            strncpy(extracted_string, start_tag_position + strlen(start_tag), extract_string_length);
            extracted_string[extract_string_length] = '\0';

            return extracted_string;
        }
    }
}
