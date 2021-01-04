#include <stdio.h>
#include <stdlib.h>
#include "portaudio.h"

#include <opus/opus.h>

#include <time.h>
#include <sys/time.h>

#include <pthread.h>
pthread_t networking_thread;


#define SAMPLE_RATE  (48000)
#define FRAMES_PER_BUFFER (120)
#define NUM_CHANNELS    (1)
typedef short SAMPLE;

#define MAX_BUFFER_SIZE 1024

opus_int32 enc_bytes;
opus_int32 dec_bytes;


unsigned short captured[MAX_BUFFER_SIZE];
unsigned short decoded[MAX_BUFFER_SIZE];
unsigned char encoded[MAX_BUFFER_SIZE*2];


// initialize opus
OpusEncoder* enc;
OpusDecoder* dec;




int status;

#define PA_SAMPLE_TYPE  paInt16


typedef struct
{
    int currentTimestamp;
    int delayPackets;
}
paData;

static paData data;


static int recordCallback( const void *inputBuffer,
                           void *outputBuffer,
                           unsigned long numSamples,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paData *data = (paData*)userData;

    data->currentTimestamp+=1;
    //////////
    // Create and insert new packet from input buffer
    // send

    const SAMPLE *read = (const SAMPLE*)inputBuffer;


    SAMPLE *write = (SAMPLE*)outputBuffer;


    int enc_bytes = opus_encode(enc, read, numSamples, encoded, MAX_BUFFER_SIZE);
        
    int dec_bytes = opus_decode(dec, encoded, enc_bytes, (opus_int16*)decoded, numSamples, 0);

    if(dec_bytes != 0) {

    }
    for( int i=0; i<numSamples; i++ ) {
        write[i] = decoded[i];
    }

    return paContinue;
}



/*******************************************************************/
int main(void);
int main(void)
{
    PaStreamParameters  inputParameters,
                        outputParameters;
    PaStream*           stream;
    PaError             err = paNoError;
    
    int opusErr;
    // int err;

    enc = opus_encoder_create(SAMPLE_RATE, NUM_CHANNELS, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &opusErr);

    dec = opus_decoder_create(SAMPLE_RATE, NUM_CHANNELS, &opusErr);

    opus_encoder_ctl(enc, OPUS_SET_BITRATE(128000));
    opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(0));
    opus_encoder_ctl(enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));


    printf("patest_record.c\n"); fflush(stdout);
    
    data.currentTimestamp = 0;
    
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
              &data );
    
    if( err != paNoError ) goto done;

    err = Pa_StartStream( stream );
    if( err != paNoError ) goto done;
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); 
    fflush(stdout);

    while( 1 )
    {
        Pa_Sleep(1000);
        // printf("index = %d\n", data.frameIndex ); fflush(stdout);
    }

    done:
    Pa_Terminate();

    return err;
}
