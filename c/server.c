#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"

#define PORT 3333
#define HOST "localhost"

#define CHECK(r, msg)                                       \
    if (r<0) {                                              \
        fprintf(stderr, "%s: %s\n", msg, uv_strerror(r));   \
        exit(1);                                            \
    }


static uv_loop_t *uv_loop;
static uv_udp_t   server;


// called after the data was sent
static void on_send(uv_udp_send_t* req, int status)
{
    free(req);
    if (status) {
        fprintf(stderr, "uv_udp_send_cb error: %s\n", uv_strerror(status));
    }
}

static void on_read(uv_udp_t *req, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags)
{
    (void) flags;
    if (nread < 0) {
        fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) req, NULL);
        free(buf->base);
        return;
    }

    if (nread > 0) {
        char sender[17] = { 0 };
        uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
        fprintf(stderr, "Recv from %s\n", sender);

        uv_udp_send_t* res = malloc(sizeof(uv_udp_send_t));
        uv_buf_t buff = uv_buf_init(buf->base, nread);
        uv_udp_send(res, req, &buff, 1, addr, on_send);
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

    (void)argc; (void) argv;
    uv_loop = uv_default_loop();

    status = uv_udp_init(uv_loop,&server);
    CHECK(status,"init");

    struct sockaddr_in addr;


    uv_ip4_addr(HOST, PORT, &addr);




    status = uv_udp_bind(&server, (const struct sockaddr*)&addr,UV_UDP_REUSEADDR);
    // CHECK(status,"bind");

    status = uv_udp_recv_start(&server, alloc_buffer, on_read);
    CHECK(status,"recv");
    


    printf("We're jammin on: UDP port: %d\n", PORT);
    
    uv_run(uv_loop, UV_RUN_DEFAULT);

    return 0;
}