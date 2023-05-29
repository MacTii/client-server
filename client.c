/*  Make the necessary includes and set up the variables.  */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define EXIT_STATUS_CONNECTION_FAILED 1
#define FAIL -1
#define EXIT_STATUS_SUCCESS 0

/* struct to root request */
struct root_request_t {
    uint32_t message_type;
    uint32_t request_id;
    uint64_t number;
} __attribute__((__packed__));

/* struct to time request */
struct time_request_t {
    uint32_t message_type;
    uint32_t request_id;
} __attribute__((__packed__));

/* struct to check size of time */
struct time_response_t {
    uint32_t message_type;
    uint32_t request_id;
    uint32_t time_size;
} __attribute__((__packed__));

int main()
{
    int sockfd;
    socklen_t len;
    struct sockaddr_in address;
    int result;

    /*  Create a socket for the client.  */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /*  Name the socket, as agreed with the server.  */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(9734);
    len = sizeof(address);

    /*  Now connect our socket to the server's socket.  */
    result = connect(sockfd, (struct sockaddr *) &address, len);

    if (result == FAIL)
    {
        perror("Connection failed");
        exit(EXIT_STATUS_CONNECTION_FAILED);
    }

    int type;
    do
    {
        printf("Enter the type of message: ");
        if(scanf("%d", &type)!=1 || (type != 1 && type != 2))
            printf("Wrong type of message!\n");
    } while(type != 1 && type != 2);

    if (type == 1)
    {
        double number;
        printf("Enter the number: ");
        scanf("%lf", &number);

        /* copying to proper fields in root structure*/
        struct root_request_t message;
        memcpy(&message.message_type, &type, sizeof(uint32_t));
        memcpy(&message.request_id, &sockfd, sizeof(uint32_t));
        memcpy(&message.number, &number, sizeof(double));

        // uint8_t *ptr = (uint8_t *) &message;
        // for(int i=0; i<sizeof(message.message_type); i++)
        //     printf("%x ", *(ptr+i));
        // printf("\n");

        /* send root structure to server */
        write(sockfd, &message, sizeof(struct root_request_t));

        /* read message about number from server */
        struct root_request_t response = {0};
        read(sockfd, &response, sizeof(struct root_request_t));

        /* copying to field */
        double result_root = 0;
        memcpy(&result_root, &response.number, sizeof(double));
        printf("The root of the given number is: %lf\n", result_root);
    }
    else if (type == 2)
    {
        /* filling the time structure */
        struct time_request_t message = {0};
        memcpy(&message.message_type, &type, sizeof(uint32_t));
        memcpy(&message.request_id, &sockfd, sizeof(uint32_t));

        // uint8_t *ptr = (uint8_t *) &message;
        // for(int i=0; i<sizeof(message.message_type); i++)
        //     printf("%x ", *(ptr+i));
        // printf("\n");

        /* send message about type and id to server socket */
        write(sockfd, &message, sizeof(struct time_request_t));

        /* read a message(struct) from server */
        struct time_response_t response = {0};
        read(sockfd, &response, sizeof(struct time_response_t));

        // uint8_t *ptr = (uint8_t *) &response;
        // for(int i=0; i<sizeof(response.message_type); i++)
        //     printf("%x ", *(ptr+i));
        // printf("\n");

        /* time */
        time_t rawtime;
        uint8_t *rawtime_ptr = (uint8_t *) &rawtime;

        if (response.time_size != sizeof(time_t))
        {
            printf("Time size not compatible with the time_t structure\n");
            close(sockfd);
            return FAIL;
        }
        read(sockfd, &rawtime, sizeof(time_t));

        // for(int i=0; i<sizeof(response.time_size); i++)
        //     printf("%x ", *(rawtime_ptr+i));
        // printf("\n");

        memcpy(&rawtime, rawtime_ptr, response.time_size);
        struct tm *timeinfo = localtime(&rawtime);
        printf ("Time is: %s\n", asctime(timeinfo));
    }

    close (sockfd);
    exit (EXIT_STATUS_SUCCESS);
}
