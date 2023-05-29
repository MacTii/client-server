#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define TYPE_SIZE 4
#define ID_SIZE 4
#define NUMBER_SIZE 8

#define EXIT_STATUS_SUCCESS 0
#define CONNECTION_FAILED -1

struct root_request_t {
    uint32_t message_type;
    uint32_t request_id;
    uint64_t number;
} __attribute__((__packed__));

struct time_request_t {
    uint32_t message_type;
    uint32_t request_id;
    uint32_t time_size;
    time_t time;
} __attribute__((__packed__));

int main ()
{
    /* Initialization of server socket and client socket*/
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(9734);
    server_len = sizeof(server_address);

    if (bind(server_sockfd, (struct sockaddr *) &server_address, server_len) != 0)
    {
        printf("Failed to connect ip %u to the socket\n", server_address.sin_addr.s_addr);
        close(server_sockfd);
        return CONNECTION_FAILED;
    }

    /*  Create a connection queue, ignore child exit details and wait for clients.  */
    listen(server_sockfd, 5);

    signal(SIGCHLD, SIG_IGN);

    while (1)
    {
        printf ("Server waiting...\n");

        /*  Accept connection.  */
        client_len = sizeof(client_address);
        client_sockfd = accept (server_sockfd, (struct sockaddr *) &client_address, &client_len);

        /*  Fork to create a process for this client and perform a test to see
			whether we're the parent or the child.  */
        if (fork () == 0)
        {
            /* read request type from client */
            uint8_t request_type[TYPE_SIZE] = {0};
            read(client_sockfd, &request_type, TYPE_SIZE);
            //printf("%d %d %d %d",request_type[0], request_type[1],request_type[2], request_type[3]);

            /* read request id from client */
            uint8_t request_id[ID_SIZE] = {0};
            read(client_sockfd, &request_id, ID_SIZE);
            
            sleep(1);

            if (*(uint32_t *) request_type == 1)
            {
                /*reading number to table (bytes)*/
                uint8_t number[NUMBER_SIZE] = {0};
                read(client_sockfd, &number, NUMBER_SIZE);

                /* copying number to struct */
                double double_number;
                memcpy(&double_number, number, NUMBER_SIZE);
                double root = sqrt(double_number);

                /* copying and sending to client */
                struct root_request_t server_response;
                memcpy(&server_response.message_type, &request_type, TYPE_SIZE);
                memcpy(&server_response.request_id, &request_id, ID_SIZE);
                memcpy(&server_response.number, &root, NUMBER_SIZE);

                // uint8_t *ptr = (uint8_t *) &server_response;
                // for(int i=0; i<sizeof(server_response.message_type); i++)
                //     printf("%x ", *(ptr+i));
                // printf("\n");

                write(client_sockfd, &server_response, sizeof(struct root_request_t));

                close(client_sockfd);
                exit(EXIT_STATUS_SUCCESS);
            }
            else if (*(uint32_t *) request_type == 2)
            {
                time_t rawtime;
                time(&rawtime);

                uint32_t time_size = sizeof(time_t);

                struct time_request_t server_response;
                memcpy(&server_response.message_type, &request_type, TYPE_SIZE);
                memcpy(&server_response.request_id, &request_id, ID_SIZE);
                memcpy(&server_response.time_size, &time_size, sizeof(uint32_t));
                memcpy(&server_response.time, &rawtime, time_size);

                // uint8_t *ptr = (uint8_t *) &server_response;
                // for(int i=0; i<sizeof(server_response.message_type); i++)
                //     printf("%x ", *(ptr+i));
                // printf("\n");

                write(client_sockfd, &server_response, sizeof(struct time_request_t));

                close(client_sockfd);
                exit(EXIT_STATUS_SUCCESS);
            }
        }
        else
        {
            close(client_sockfd);
        }
    }
}
