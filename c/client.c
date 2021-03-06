#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include <time.h>
#include <sys/time.h>



uint64_t getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}


#define CHECK(r, msg)                                       \
    if (r<0) {                                              \
        fprintf(stderr, "%s: %s\n", msg, uv_strerror(r));   \
        exit(1);                                            \
    }



// #define PORT 3333
// #define HOST "b33p.live"


static int PORT = 3333;
static char* HOST = "localhost";

static uv_loop_t *uv_loop;
static uv_udp_t   sock;
struct sockaddr_in send_addr;

// static uv_udp_t   recv_sock;
static struct sockaddr_in recv_addr;


// called after the data was sent
static void on_send(uv_udp_send_t* req, int status)
{
//    fprintf(stderr, "1 %i\n", (status));
    free(req);
    if (status) {
        fprintf(stderr, "uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
}


void timer_cb (uv_timer_t* timer, int status) {

    
    int64_t now = getTimestamp();
    
//	fprintf(stderr, "sending %lld %i %i %i \n", (long long) now, send_addr.sin_family, send_addr.sin_port, send_addr.sin_addr.s_addr );

    // uv_ip4_addr(SEND_HOST, SEND_PORT, &send_addr);    

    uv_udp_send_t* res = malloc(sizeof(uv_udp_send_t));
    uv_buf_t buff = uv_buf_init((char *) &now, sizeof(now));
    uv_udp_send(res, &sock, &buff, 1, (const struct sockaddr*) &send_addr, on_send);    

}


int numResponses = 0 ;

static void on_read(uv_udp_t *req_sock, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags)
{
    (void)flags;
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) req_sock, NULL);
        free(buf->base);
        return;
    }

    if (nread > 0) {
        char sender[17] = { 0 };
        uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);

        int64_t packetTime = *((int64_t*) buf->base);

        int diff = (int)(getTimestamp()- packetTime);
        

        if(++numResponses%1000==0) {
            fprintf(stderr, "Recv from %s: %lld took %dus \n", sender, (long long) packetTime, diff);
        }
    }

    free(buf->base);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)handle;
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}




int main(int argc,char *argv[]) {
    int status=0;

    // atoi(getenv("PORT"))

    if(argc > 1) {
        PORT = atoi(argv[1]);
    }
    if(argc > 2) {
        HOST = argv[2];
    }

    struct hostent *ghbn=gethostbyname(HOST);//change the domain name
    if (!ghbn) {
        printf("couldn't look up ");
    }

    char * ipaddr = inet_ntoa(*(struct in_addr *)ghbn->h_addr);

    printf("Host Name->%s\n", ghbn->h_addr);
    printf("IP ADDRESS->%s\n",ipaddr );
    
    uv_loop = uv_default_loop();

    uv_timer_t timer;
	uv_timer_init(uv_loop, &timer);
	uv_timer_start(&timer, (uv_timer_cb) &timer_cb, 25, 1);

    
    status = uv_udp_init(uv_loop,&sock);
    CHECK(status,"init");
    
    uv_ip4_addr(ipaddr, PORT, &send_addr);

    uv_ip4_addr("0.0.0.0", 0, &recv_addr);
    status = uv_udp_bind(&sock, (const struct sockaddr*)&recv_addr, UV_UDP_REUSEADDR);
    CHECK(status,"bind");

    status = uv_udp_recv_start(&sock, alloc_buffer, on_read);
    CHECK(status,"recv");

    printf("we're jammin at \n");

    
    uv_run(uv_loop, UV_RUN_DEFAULT);

    return 0;
}