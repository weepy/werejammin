#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include "json11.hpp"
#include <opus/opus.h>

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

OpusEncoder *encoder;
OpusDecoder *decoder;

#define CHECK(r, msg)                                       \
    if (r<0) {                                              \
        cerr << msg << ": " << uv_strerror(r) << endl;      \
        exit(1);                                            \
    }


uv_loop_t *uv_loop;
uv_udp_t   sock;
struct sockaddr_in send_addr;
struct sockaddr_in recv_addr;

int PORT = 3333;
string HOST = "localhost";


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

#define MAX_BUFFER_SIZE 1024
char encoding_buffer[MAX_BUFFER_SIZE]; // MAX
char decoding_buffer[MAX_BUFFER_SIZE]; // MAX

static int recordCallback( const void *inputBuffer,
                           void *outputBuffer,
                           unsigned long numSamples,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *_userData )
{
    paData *userdata = (paData*)_userData;
    
    int num = int(numSamples);
    //////////
    // Create and insert new packet from input buffer
    // send

    const char *read = (const char*)inputBuffer;

    Packet* localPacket = Packet::create(1, read, num*sizeof(SAMPLE), num, userdata->currentTimestamp);
    
    userdata->localBuffer->push(localPacket);
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
    
    opus_int32 enc_bytes = opus_encode(encoder, (opus_int16*)read, num, (unsigned char*) encoding_buffer, MAX_BUFFER_SIZE);

    
//    Packet* packetToSend = Packet::create(2, encoding_buffer, enc_bytes, num, userdata->currentTimestamp);
//
//    uv_udp_send_t* res = new uv_udp_send_t;
//    uv_buf_t buff = uv_buf_init((char*) packetToSend, packetToSend->numBytes );
//    uv_udp_send(res, &sock, &buff, 1, (const struct sockaddr*) &send_addr, on_send);
//
    
//    int time = userdata->currentTimestamp - 20;
//    Packet* recvPacket = packetToSend;//userdata->recvBuffer->pull( time );
    int dec_bytes;
    
//    if(recvPacket){
        
//        if(recvPacket->type == 2) {
            
            dec_bytes = opus_decode(decoder,
                (const unsigned char*) encoding_buffer, //(&recvPacket->data),
                enc_bytes,//recvPacket->dataLength(),
                (opus_int16 *)decoding_buffer,
                num, //recvPacket->numSamples,
                0);


                // printf("%i %i\n", dec_bytes, num);

//        }
//        else {
//            // NO PACKET
////            samplesToPlay
//
//            cerr<< "unknown packet type: " << recvPacket->type << endl;
//            for( int i=0; i<numSamples; i++ )
//               write[i] = 0;
//        }
//
//
//        Packet::destroy(recvPacket);
//    }
//    else {
//        cerr << "missing packet @" << time << endl;
//        dec_bytes = opus_decode(decoder, NULL, 0, (opus_int16*) decoding_buffer, BUFFER_SIZE, 0);
//    }

    for( int i=0; i<numSamples; i++ ) {
        write[i] = decoding_buffer[i];
    }
//
//    if(packetToSend)
//        Packet::destroy(packetToSend);
    
    userdata->currentTimestamp += 1;

    return paContinue;
}


/*********** NETWORKING ***********/

float smoothedLatency = 0;
float maxLatency = 0;
float smoothing = 0.9;

#include <math.h>

void timer_cb (uv_timer_t* timer, int status) {

    // DO PERIODIC STUFF
    
    cerr << "mean: " << floor(smoothedLatency*100)/100.0 ;
    if(maxLatency > smoothedLatency) {
        cerr << " (" << maxLatency << ")";
        maxLatency = 0;
    }
    cerr << endl;
}


static void on_read(uv_udp_t *req_sock, ssize_t nread, const uv_buf_t *buf,
             const struct sockaddr *addr, unsigned flags)
{
    (void)flags;
    if (nread < 0) {
        cerr<< "Read error "<< uv_err_name((int)nread)<<endl;
        uv_close((uv_handle_t*) req_sock, NULL);
        free(buf->base);
        return;
    }

    if (nread > 0) {
        
        Packet* packet = (Packet*) buf->base;
        
        int diff = (int)(userData.currentTimestamp - packet->timestamp);
        
        smoothedLatency = diff * (1.-smoothing) + smoothedLatency * smoothing;

        if(diff > maxLatency) {
            maxLatency = diff;
        }

        userData.recvBuffer->push(packet);

    }


}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)handle;
    buf->base = new char[suggested_size];
    buf->len = suggested_size;
}


void *setupUV (void *x_void_ptr) {
    
    struct hostent *ghbn=gethostbyname(HOST.c_str());//change the domain name
    if (!ghbn) {
        printf("couldn't look up ");
     }
    
    char * ipaddr = inet_ntoa(*(struct in_addr *)ghbn->h_addr);

    printf("Host Name->%s\n", ghbn->h_addr);
    printf("IP ADDRESS->%s\n",ipaddr );

    
    

    
    
    uv_loop = uv_default_loop();
    uv_ip4_addr(ipaddr, PORT, &send_addr);
    status = uv_udp_init(uv_loop,&sock);
    CHECK(status,"init");
    
    uv_timer_t timer;
    uv_timer_init(uv_loop, &timer);
    uv_timer_start(&timer, (uv_timer_cb) &timer_cb, 25, 1000);
    
    // struct sockaddr_in addr;
    
    uv_ip4_addr("0.0.0.0", 0, &recv_addr);
    status = uv_udp_bind(&sock, (const struct sockaddr*)&recv_addr, UV_UDP_REUSEADDR);
    CHECK(status,"bind");
    
    
    // sCHECK(status,"bind");
    status = uv_udp_recv_start(&sock, alloc_buffer, on_read);
    CHECK(status,"recv");
    
    printf("I hope you like Jammin too \n");
    
    uv_run(uv_loop, UV_RUN_DEFAULT);
    return NULL;
}



/*******************************************************************/
int main(int argc, char* argv[])
{
    
    
    if(argc > 1)
        HOST = argv[1];
    if(argc > 2)
        PORT = atoi(argv[2]);
    
    encoder = opus_encoder_create(SAMPLE_RATE, NUM_CHANNELS, OPUS_APPLICATION_AUDIO, &status);
    
    // opus_encoder_ctl(encoder, OPUS_SET_BITRATE(64000));
    // opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(10));
    // opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));


    
//    OPUS_SIGNAL_MUSIC
//    OPUS_FRAMESIZE_2_5_MS
//    OPUS_SET_COMPLEXITY
//    OPUS_SET_BITRATE
//    OPUS_SET_VBR
    
    //
    decoder = opus_decoder_create(SAMPLE_RATE, NUM_CHANNELS, &status);
    
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    
    printf("patest_record.c\n");
    
    pthread_create(&networking_thread, NULL, setupUV, NULL);
    

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
