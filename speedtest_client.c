
#define _GNU_SOURCE
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

#define DL_MIB 4

#define min(x, y)   ((x)>(y)?(y):(x))

int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        fprintf(stderr, "Usage: %s server [test size in MiB]\n", argv[0]);
        return 1;
    }
    struct hostent *ents = gethostbyname(argv[1]);
    if(!ents)
    {
        fprintf(stderr, "Failed to resolve server\n");
        return 1;
    }

    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_port = htons(9999),
        .sin_addr = *(struct in_addr*)ents->h_addr,
    };

    int dev_null = open("/dev/null", O_WRONLY);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1)
    {
        fprintf(stderr, "socket() failed: %s (%d)\n", strerror(errno), errno);
        return 1;
    }

    int window_size = 1024 * 1024; // 1mib;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char*)&window_size, sizeof(window_size));
    setsockopt(fd, SOL_SOCKET, TCP_WINDOW_CLAMP, (char*)&window_size, sizeof(window_size));

    if(connect(fd, (const struct sockaddr*)&server, sizeof(server)) == -1)
    {
        fprintf(stderr, "connect() failed:  (%d)\n", errno);
        return 1;
    }

    struct timespec ts1, ts2;
    unsigned int dl_mib = DL_MIB;
    if(argc == 3)
        dl_mib = atoi(argv[2]);
    unsigned int dl_count = dl_mib * 1024 * 1024; // 4MiB
    printf("Size: %fMiB\n", (float)dl_count / (1024.0f * 1024.0f));
    //fprintf(stderr, "dl_count: %u\n", dl_count);
    char buffer[1024 * 1024];

    sleep(1);
    clock_gettime(CLOCK_MONOTONIC, &ts1);
    while(dl_count > 0)
    {
        int result = recv(fd, buffer, min(sizeof(buffer), dl_count), 0);
    //    fprintf(stderr, ". %d\n", result);
        if(result == -1)
        {
            fprintf(stderr, "recv() failed: (%d)\n", errno);
            return 1;
        }
        dl_count -= result;
    }
    clock_gettime(CLOCK_MONOTONIC, &ts2);
    close(fd);

    ts2.tv_sec -= ts1.tv_sec;
    ts2.tv_nsec -= ts1.tv_nsec;
    if(ts2.tv_nsec < 0)
    {
        ts2.tv_sec--;
        ts2.tv_nsec += 1000000000;
    }
    double dl_time = (double)ts2.tv_sec + ((double)(ts2.tv_nsec / 1000000) / 1000.0f);
    printf("Time: %fs\n", dl_time);
    
    double speed = (double)(dl_mib * 8) / dl_time;
    printf("Speed: %.3fmbps\n", speed);
}