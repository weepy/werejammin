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



#define PORT 2222
#define HOST "127.0.0.1"


#define REMOTE_PORT 2222
#define REMOTE_HOST "127.0.0.1"




// static uv_udp_t* udp_socket = NULL;
static uv_loop_t *uv_loop;
static uv_udp_t   sock;
struct sockaddr_in remote_addr;


// called after the data was sent
static void on_send(uv_udp_send_t* req, int status)
{
    free(req);
    if (status) {
        fprintf(stderr, "uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
}


void timer_cb (uv_timer_t* timer, int status) {

    const char* msg = "hello";
    // clock_t cpu_time = clock();

    int64_t now = getTimestamp();
    
	// fprintf(stderr, "sending %s %lld \n", msg, (long long) now);

    uv_udp_send_t* res = malloc(sizeof(uv_udp_send_t));
    uv_buf_t buff = uv_buf_init((char*) &now, sizeof(int64_t));
    uv_udp_send(res, &sock, &buff, 1, (const struct sockaddr*) &remote_addr, on_send);
    

}


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
        
        fprintf(stderr, "Recv from %s: %lld took %d \n", sender, (long long) packetTime, diff);
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

    printf("we're jammin\n");

    uv_loop = uv_default_loop();

    uv_timer_t timer;
	uv_timer_init(uv_loop, &timer);
	uv_timer_start(&timer, (uv_timer_cb) &timer_cb, 0, 25);

    uv_ip4_addr(REMOTE_HOST, REMOTE_PORT, &remote_addr);

    
    status = uv_udp_init(uv_loop,&sock);
    CHECK(status,"init");


    struct sockaddr_in addr;

    uv_ip4_addr(HOST, PORT, &addr);

    status = uv_udp_bind(&sock, (const struct sockaddr*)&addr, UV_UDP_REUSEADDR);
    CHECK(status,"bind");

    status = uv_udp_recv_start(&sock, alloc_buffer, on_read);
    CHECK(status,"recv");
    


    printf("client jammin on: UDP port: %d\n", PORT);
    
    uv_run(uv_loop, UV_RUN_DEFAULT);

    return 0;
}