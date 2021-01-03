#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

#include <iostream>

#include "uv.h"
#include <time.h>
#include <sys/time.h>

#include <pthread.h>
pthread_t networking_thread;

using namespace std;

#define SAMPLE_RATE  (48000)
#define FRAMES_PER_BUFFER (120)
#define NUM_CHANNELS    (1)
typedef short SAMPLE;


uint64_t getTimestamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}


#define CHECK(r, msg)                                       \
    if (r<0) {                                              \
        cerr << msg << ": " << uv_strerror(r) << endl;      \
        exit(1);                                            \
    }




// static uv_udp_t* udp_socket = NULL;
static uv_loop_t *uv_loop;
static uv_udp_t   sock;
struct sockaddr_in addr;


#define PORT 3333
#define HOST "127.0.0.1"

int status;

#include "packet.hpp"

#define PA_SAMPLE_TYPE  paInt16


typedef struct
{
    int currentTimestamp;
    int delayPackets;
    PacketBuffer* localBuffer;
    PacketBuffer* recvBuffer;
}
paData;

static paData userData;

// called after the data was sent
static void on_send(uv_udp_send_t* req, int status)
{
    free(req);
    if (status) {
        cerr<< "uv_udp_send_cb error: " << uv_strerror(status) << endl;
    }
}
static int recordCallback( const void *inputBuffer,
                           void *outputBuffer,
                           unsigned long numSamples,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *_userData )
{
    paData *userdata = (paData*)_userData;

    //////////
    // Create and insert new packet from input buffer
    // send

    const SAMPLE *read = (const SAMPLE*)inputBuffer;
    Packet* packet = new Packet(read, (int)numSamples, userdata->currentTimestamp);
    

    uv_udp_send_t* res = new uv_udp_send_t;
    uv_buf_t buff = uv_buf_init((char*) packet, packet->numBytes );
    uv_udp_send(res, &sock, &buff, 1, (const struct sockaddr*) &addr, on_send);
    

    userdata->localBuffer->push(packet);
    
    
    ///////
    // Write into output buffer
    //


    SAMPLE *write = (SAMPLE*)outputBuffer;
//    Packet* outputPacket = data->localBuffer->pull( data->currentTimestamp - data->delayPackets);
//
//    if(outputPacket){
//        for( int i=0; i<numSamples; i++ ) {
//            write[i] = outputPacket->samples[i];
//        }
//
//        delete outputPacket;
//    }
//
    Packet* remoteOutputPacket = userdata->recvBuffer->pull( userdata->currentTimestamp - 20);
    if(remoteOutputPacket){
        for( int i=0; i<numSamples; i++ ) {
            write[i] = remoteOutputPacket->samples[i];
        }

        delete remoteOutputPacket;
    }
    

    userdata->currentTimestamp += 1;

    return paContinue;
}


/*********** NETWORKING ***********/







void timer_cb (uv_timer_t* timer, int status) {

    // const char* msg = "hello";
    // clock_t cpu_time = clock();

    // int64_t now = getTimestamp();
    
    // fprintf(stderr, "sending %s %lld \n", msg, (long long) now);

    // uv_udp_send_t* res = malloc(sizeof(uv_udp_send_t));
    // uv_buf_t buff = uv_buf_init((char*) &now, sizeof(int64_t));
    // uv_udp_send(res, &sock, &buff, 1, (const struct sockaddr*) &addr, on_send);
    

}


static void on_read(uv_udp_t *req_sock, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags)
{
    (void)flags;
    if (nread < 0) {
        cerr<< "Read error "<< uv_err_name(nread)<<endl;
        uv_close((uv_handle_t*) req_sock, NULL);
        free(buf->base);
        return;
    }

    if (nread > 0) {
        char sender[17] = { 0 };
        uv_ip4_name((const struct sockaddr_in*) addr, sender, 16);
        
        Packet* b = (Packet*) buf->base;
        Packet* packet = new Packet(b->samples,b->numSamples,b->timestamp);
        
        int diff = (int)(userData.currentTimestamp- packet->timestamp);
        
        cerr << "Recv from " << sender << ": " << "took " << diff << "\n";
        userData.recvBuffer->push(packet);

    }

    //free(buf->base);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)handle;
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}


void *setupNetworking (void *x_void_ptr) {
    uv_loop = uv_default_loop();
    uv_ip4_addr(HOST, PORT, &addr);
    status = uv_udp_init(uv_loop,&sock);
    CHECK(status,"init");
    
    
    // struct sockaddr_in addr;
    // uv_ip4_addr(RECEIVE_HOST, RECEIVE_PORT, &addr);
    status = uv_udp_bind(&sock, (const struct sockaddr*)&addr, UV_UDP_REUSEADDR);
    // sCHECK(status,"bind");
    status = uv_udp_recv_start(&sock, alloc_buffer, on_read);
    CHECK(status,"recv");
    
    uv_run(uv_loop, UV_RUN_DEFAULT);
    return NULL;
}





/*******************************************************************/

int main(void)
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    

    printf("patest_record.c\n"); fflush(stdout);


    pthread_create(&networking_thread, NULL, setupNetworking, NULL);
    //setupNetworking();


    userData.currentTimestamp = 0;
    userData.recvBuffer = new PacketBuffer();
    userData.localBuffer = new PacketBuffer();
    userData.delayPackets = 10;

    err = Pa_Initialize();
    if( err != paNoError ) goto done;

    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto done;
    }
    inputParameters.channelCount = NUM_CHANNELS;                    /* stereo input */
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;

    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto done;
    }
    outputParameters.channelCount = NUM_CHANNELS;                     /* stereo output */
    outputParameters.sampleFormat =  PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;


    /* Record some audio. -------------------------------------------- */
    err = Pa_OpenStream(
              &stream,
              &inputParameters,
              &outputParameters,                  /* &outputParameters, */
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              recordCallback,
              &userData );
    
    if( err != paNoError ) goto done;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n");
    fflush(stdout);

    while( 1 )
    {
        Pa_Sleep(1000);
        // printf("index = %d\n", userData.frameIndex ); fflush(stdout);
    }

    done:
    Pa_Terminate();

    return err;
}
