//
// Created by Jacob Tarnow on 9/27/16.
//
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
/*
 * Makes available the type in_port_t and type in_addr_t as defined in netinet/in.h
 * The sockaddr_in structure is used to store addresses for the Internet protocol family.
 * Values of this type must be cast to struct sockaddr for use with the socket interfaces
 * */
#include <netdb.h>
#include <stdbool.h>
/*
 * This header defines the hostnet structure that includes the following memebers
 * char *h_name which is the official name of the host
 * char **h_aliases which is a pointer to an array of pointers to alternative host names, terminated by a null pointed
 * int h_addretype which is the address type
 * int h_length which is the length, in bytes, of the address
 * char **h_addr_list which is a pointer to an array of pointer to network addresses (in network byte order)
 */

#include "utils.h"

#define SSDP_MULTICAST_ADDRESS  "239.255.255.250"
#define SSDP_MULTICAST_PORT     1900
#define MAX_BUFFER_LEN          8192

/* Global Vars */
int opt_source_port = 0;
int opt_verbose = false;
int opt_dns_lookup = false;

/* Functions */
int discover_hosts(struct str_vector *vector);
int dns_lookup(char *ip_addr, char *hostname, int hostname_size);
int parse_cmd_opts(int argc, char *argv[]);

int main(int argc, char *argv[]) {
    int ret;
    struct str_vector my_vector;
    parse_cmd_opts(argc, argv);
    str_vector_init(&my_vector);
    ret = discover_hosts(&my_vector);
    str_vector_free(&my_vector);
    return ret;
}

/*
 * Discover UPnP hosts via multicast
 * http://www.upnp-hacks.org/upnp.html
 * http://upnp.org/specs/arch/UPnP-arch-DeviceArchitecture-v1.1.pdf
 * Bonjour and other UDP
 */
int discover_hosts(struct str_vector *vector) {
    int ret, sock, bytes_in, done = false;
    unsigned int host_sock_len;
    struct sockaddr_in src_sock, dest_sock, host_sock;
    char buffer[MAX_BUFFER_LEN];
    //NETDB defined the constant for choosing reasonable size buffers
    char host[NI_MAXHOST];
    //check Port 1900 for UDP SSDP and Bonjour, 554 for Streaming Services
    //simple service discovery protocol to retrieve via HTTPU -> HTTP over UDP
    char *ssdp_discover_string =
            "M-SEARCH * HTTP/1.1\r\n"
            "HOST: 239.255.255.250:1900\r\n"
            "MAN: \"ssdp:discover\"\r\n"
            "MX: 10\r\n"
            "ST: ssdp:all\r\n"
            "\r\n";

    //declare pointers for IP URL and host specs
    char *url_start, *host_start, *host_end;
    fd_set read_fds; // from the netdb lib
    struct timeval timeout;

    // Get a socket for multicasting
    if ((sock == socket(PF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket()");
        return -1;
    }

    // Bind to the client source port to socket (piped)
    memset(&src_sock, 0, sizeof(src_sock)); //set memory to address of socket
    src_sock.sin_family = AF_INET;
    src_sock.sin_addr.s_addr = htonl(INADDR_ANY);
    src_sock.sin_port = htons(opt_source_port);

    if ((bind(sock, (struct sockaddr *)&src_sock, sizeof(src_sock))) == -1) {
        perror("bind()");
        return -1;
    } else if ((opt_verbose == true) && (opt_source_port != 0)) {
        printf("[Client bound to port %d]\n\n", opt_source_port);
    }

    //Prepare the destination info
    memset(&dest_sock, 0, sizeof(dest_sock)); //set memory to address of socket
    dest_sock.sin_family = AF_INET;
    dest_sock.sin_port = htons(SSDP_MULTICAST_PORT);
    inet_pton(AF_INET, SSDP_MULTICAST_ADDRESS, &dest_sock.sin_addr);

    //Send SSDP request
    if ((ret = sendto(sock, ssdp_discover_string, strlen(ssdp_discover_string), 0,
                      (struct sockaddr*) &dest_sock, sizeof(dest_sock))) == -1) {
        perror("sendto()");
        return -1;
    } else if (ret != strlen(ssdp_discover_string)) {
        printf(stderr, "sendto(): only sent %d of %d bytes\n", ret, (int)strlen(ssdp_discover_string));
        return -1;
    } else if(opt_verbose == true) {
        printf("%s\n", ssdp_discover_string);
    }

    //Get SSDP response --> make into new function
    FD_ZERO(&read_fds);
    FD_SET(sock, &read_fds);
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    //Loop through SSDP discovery -- request and response protocols
    do {
        if (select(sock + 1, &read_fds, NULL, NULL, &timeout) == -1) {
            perror("select()");
            return -1;
        }
        if (FD_ISSET(sock, &read_fds)) {
            host_sock_len = sizeof(host_sock);
            if ((bytes_in = recvfrom(sock, buffer, sizeof(buffer), 0, &host_sock, &host_sock_len)) == -1) {
                perror("recvfrom()");
                return -1;
            } buffer[bytes_in] = '\0'; // set buffer to accept 0 more bytes
            //if HTTP Response is a 200 OK
            if (strncmp(buffer, "HTTP/1.1 200 OK", 12) == 0) {
                if (opt_verbose == true) {
                    printf("\n%s", buffer);
                }
                // skip to solely parse SSDSP response, not all of header
                if ((url_start = strcasestr(buffer, "LOCATION:")) != NULL) {
                    if ((host_start = strstr(url_start, "http://")) != NULL) {
                        host_start += 7;
                        if ((host_end = strstr(host_start, ":")) != NULL) {
                            strncpy(host, host_start, host_end - host_start);
                            host[host_end - host_start] = '\0';

                            if (str_vector_search(vector, host) == false) {
                                str_vector_add(vector, host);
                                printf("%s", host);

                                if (opt_dns_lookup == true) {
                                    char name[NI_MAXHOST];
                                    name[0] = '\0';

                                    if (dns_lookup(host, name, NI_MAXHOST) == 0) {
                                        printf("\t%s", name);
                                    }
                                }
                                printf("\n");
                            }
                        }
                    }
                }
            } else {
                fprintf(stderr, "[Unexpected SSDP response]\n");
                if (opt_verbose == true) {
                    printf("%s\n\n", buffer);
                }
            }
        } else {
            done = true;
        }
    } while (done == false);

    if (close(sock) == -1) {
        perror("close()");
    }
    return 0;
}

int dns_lookup(char *ip_addr, char *host_name, int hostname_size) {
    int ret;
    struct sockaddr_in structsockaddr_in;
    memset(&structsockaddr_in, 0, sizeof(structsockaddr_in));
    structsockaddr_in.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &structsockaddr_in);

    if ((ret = getnameinfo((struct sockaddr *)&structsockaddr_in, sizeof(structsockaddr_in),
    host_name, hostname_size, NULL, 0, 0))) {
        fprintf(stderr, "getnameinfo(): %s\n", gai_strerror(ret));
    }
    return ret;
}

int parse_cmd_opts(int argc, char *argv[]) {
    int cmdopt;

    while ((cmdopt = getopt(argc, argv, "p:rv")) != -1) {
        switch (cmdopt) {
            case 'p':
                opt_source_port = atoi(optarg);
                break;
            case 'r':
                opt_dns_lookup = true;
                break;
            case 'v':
                opt_verbose = true;
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
    return 0;
}