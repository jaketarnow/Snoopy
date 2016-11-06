#define _GNU_SOURCE     /* for strcasestr() */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "utils.h"

#define SSDP_MULTICAST_ADDRESS      "239.255.255.250"
#define SSDP_MULTICAST_PORT         1900
#define MAX_NUM_HOSTS               25

#define MAX_BUFFER_LEN              8192


/* Globals */
int opt_source_port = 0;
int opt_verbose = FALSE;
int opt_dns_lookup = FALSE;

/* Functions */
char **discover_hosts (struct str_vector *vector);
char **dns_lookup(char *ip_addr, char *hostname, int hostname_size);
char **scanUPNP (int argc, char *argv[]);
int parse_cmd_opts (int argc, char *argv[]);


//int main () {
//    char *array[MAX_NUM_HOSTS];
//    array[0] = "-r";
//    scanUPNP(1, array);
//    return 1;
//}

char **scanUPNP (int argc, char *argv[]) {
    char **ret;
    struct str_vector my_vector;
    
    parse_cmd_opts(argc, argv);
    
    str_vector_init(&my_vector);
    
    ret = discover_hosts(&my_vector);
    
    printf("\nHost Discovery Complete\n\n");
    
    //output of host array is wrong...
//    int i = 0;
//    for (; i < (MAX_NUM_HOSTS * 2); i++) {
//        //printf("%d:\t%s\n", i, ret[i]);
//    }
    
    str_vector_free(&my_vector);
    
    return(ret);
}


/*
 * *******************************************************************
 * Function: discover_hosts()
 * Purpose: Discover UPnP hosts on a network
 * Returns: 0 on success, -1 otherwise
 * *******************************************************************
 */
char **discover_hosts (struct str_vector *vector) {
    int ret, sock, bytes_in, done = FALSE;
    
    /* need to set malloc for char * array */
    
    char **hostArray = (char **)malloc(2 * sizeof(char *) * MAX_NUM_HOSTS);
    // char *hostArray[MAX_NUM_HOSTS];
    
    int addHost = 0;
    
    unsigned int host_sock_len;
    struct sockaddr_in src_sock, dest_sock, host_sock;
    char buffer[MAX_BUFFER_LEN];
    char host[NI_MAXHOST];
    char *ssdp_discover_string =
    "M-SEARCH * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1900\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "MX: 10\r\n"
    "ST: ssdp:all\r\n"
    "\r\n";
    char *url_start, *host_start, *host_end;
    fd_set read_fds;
    struct timeval timeout;
    
    /* Get a socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket()");
        exit(1);
    }
    
    /* Bind client-side (source) port to socket */
    memset(&src_sock, 0, sizeof(src_sock));
    src_sock.sin_family = AF_INET;
    src_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    src_sock.sin_port = htons(opt_source_port);
    
    if ( (bind(sock, (struct sockaddr *)&src_sock,
               sizeof(src_sock))) == -1 ) {
        perror("bind()");
        exit(1);
    }
    else if ( (opt_verbose == TRUE) && (opt_source_port != 0) )
        printf("[Client bound to port %d]\n\n", opt_source_port);
    
    /* Prepare destination info */
    memset(&dest_sock, 0, sizeof(dest_sock));
    dest_sock.sin_family = AF_INET;
    dest_sock.sin_port = htons(SSDP_MULTICAST_PORT);
    inet_pton(AF_INET, SSDP_MULTICAST_ADDRESS, &dest_sock.sin_addr);
    
    /* Send SSDP request */
    if ( (ret = sendto(sock, ssdp_discover_string,
                       strlen(ssdp_discover_string), 0,
                       (struct sockaddr*) &dest_sock,
                       sizeof(dest_sock))) == -1) {
        perror("sendto()");
        exit(1);
    } else if (ret != strlen(ssdp_discover_string)) {
        fprintf(stderr, "sendto(): only sent %d of %d bytes\n",
                ret, (int)strlen(ssdp_discover_string));
        exit(1);
    } else if ( opt_verbose == TRUE ) {
        hostArray[addHost++] = ssdp_discover_string;
        //printf("%s\n", ssdp_discover_string);
    }
    
    /* Get SSDP response */
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    printf("\n");
    
    /* Loop through SSDP discovery request responses */
    do {
        if (select(sock+1, &read_fds, NULL, NULL, &timeout) == -1) {
            perror("select()");
            exit(1);
        }
        
        if (FD_ISSET(sock, &read_fds)) {
            host_sock_len = sizeof(host_sock);
            if ((bytes_in = recvfrom(sock, buffer, sizeof(buffer), 0,
                                     &host_sock, &host_sock_len)) == -1) {
                perror("recvfrom()");
                exit(1);
            }
            
            buffer[bytes_in] = '\0';
            
            if (strncmp(buffer, "HTTP/1.1 200 OK", 12) == 0) {
                if ( opt_verbose == TRUE ) {
                    printf("\n%s", buffer);
                }
                
                /* Skip ahead to url in SSDP response */
                if ( (url_start = strcasestr(buffer, "LOCATION:")) != NULL ) {
                    /* Get hostname/IP address in SSDP response */
                    if ( (host_start = strstr(url_start, "http://")) != NULL ) {
                        host_start += 7;    /* Move past "http://" */
                        
                        if ( (host_end = strstr(host_start, ":")) != NULL ) {
                            strncpy(host, host_start, host_end - host_start);
                            host[host_end - host_start] = '\0';
                            
                            /* Add host to vector if we haven't done so already */
                            if ( str_vector_search(vector, host) == FALSE ) {
                                str_vector_add(vector, host);
                                //                                printf("%s", host);
                                hostArray[addHost++] = host;
                                printf("%d: \t%s\n", addHost, host);
                                
                                /* Are we doing lookups? */
                                //                                 if ( opt_dns_lookup == TRUE ) {
                                //                                     char name[NI_MAXHOST];
                                //                                     name[NI_MAXHOST - 1] = '\0';
                                
                                //                                     if (dns_lookup(host, name, NI_MAXHOST)) {
                                // //                                        printf("\t%s", name);
                                //                                         hostArray[addHost++] = name;
                                //                                         printf("%d\t: %s\n", addHost, name);
                                //                                     }
                                //                                 }
                            }
                        }
                    }
                }
            } else {
                fprintf(stderr, "[Unexpected SSDP response]\n");
                if ( opt_verbose == TRUE ) {
                    printf("%s\n\n", buffer);
                }
            }
        }
        else {
            /* select() timed out, so we're done */
            done = TRUE;
        }
        
    } while ( done == FALSE && addHost < MAX_NUM_HOSTS );
    
    if ( close(sock) == -1 )
        perror("close()");
    //output of host array is wrong... need to double check pointers
    return hostArray;
}


/*
 * *******************************************************************
 * Function: dns_lookup()
 * Purpose: Given an IP address in *ip_addr, return its hostname in
 *           *hostname, not to exeed hostname_size
 * Returns: 0 on success
 * *******************************************************************
 */
char **dns_lookup(char *ip_addr, char *hostname, int hostname_size) {
    char **hostArray = (char **)malloc(sizeof(char *) + MAX_NUM_HOSTS);
    struct sockaddr_in sa;
    
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &sa.sin_addr);
    
    if ( (**hostArray = getnameinfo((struct sockaddr *)&sa, sizeof(sa),
                                    hostname, hostname_size, NULL, 0, 0)) != 0 ) {
        fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(**hostArray));
    }
    
    return(hostArray);
}


/*
 * *******************************************************************
 * Function: parse_cmd_opts()
 * Purpose: Given argc and argv[], parse the command line options.
 * Returns: 0 on success, exits on failure
 * *******************************************************************
 */
int parse_cmd_opts (int argc, char *argv[]) {
    int cmdopt;
    
    while ( (cmdopt = getopt(argc, argv, "p:rv")) != -1 ) {
        switch (cmdopt) {
            case 'p':
                opt_source_port = atoi(optarg);
                break;
            case 'r':
                opt_dns_lookup = TRUE;
                break;
            case 'v':
                opt_verbose = TRUE;
                break;
            default:
                printf("\nUsage: %s [OPTION]...", argv[0]);
                printf("\nDiscover and list UPnP devices on the network.\n\n");
                printf("Available options:\n\n");
                printf("  -p [port]\tSpecify client-side (source) UDP port to bind to\n");
                printf("  -r\t\tDo reverse DNS lookups\n");
                printf("  -v\t\tProvide verbose information\n");
                printf("\n");
                exit(EXIT_FAILURE);
        }
    }
    
    return(0);
}
