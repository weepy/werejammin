#include <stdio.h>
#include <stdlib.h>
#include <list>

#include "portaudio.h"

using namespace std;

#define SAMPLE_RATE  (48000)
#define FRAMES_PER_BUFFER (120)
#define NUM_CHANNELS    (1)

#define PA_SAMPLE_TYPE  paInt16
typedef short SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"


struct Packet {
    int size=0;
    int timestamp=0;
    SAMPLE* samples=0;

    Packet(const SAMPLE *read, int _size, int _timestamp) {
        size = _size;
        samples = new SAMPLE[size];
        for( int i=0; i<size; i++ )
        {
            samples[i] = read[i];
        }
        timestamp = _timestamp;
    }
    ~Packet() {
        if(samples)
            delete samples;

        samples = 0;
    }
};

typedef struct
{
    int currentTimestamp = 0;
    int delayPackets = 10;
    list<Packet*> packets;
    bool insertPacket(Packet* p) {
        packets.push_back(p);
        return true;
    }
    Packet* getPacket(int timestamp) {
        
            
        while(true) {
            if(packets.size() == 0)
                return 0;

            Packet* p = packets.front();

            if(p->timestamp < timestamp) {
                packets.pop_front();
                delete p;
            }
            else if(p->timestamp == timestamp) {
                packets.pop_front();
                return p;
            }
            else if(p->timestamp > timestamp) {
                return 0;
            }

        }
    }
}
paData;


static int recordCallback( const void *inputBuffer,
                           void *outputBuffer,
                           unsigned long numSamples,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *userData )
{
    paData *data = (paData*)userData;

    //////////
    // Create and insert new packet from input buffer
    const SAMPLE *read = (const SAMPLE*)inputBuffer;
    Packet* packet = new Packet(read, numSamples, data->currentTimestamp);

    data->insertPacket(packet);
    
    ///////
    // Write into output buffer
    SAMPLE *write = (SAMPLE*)outputBuffer;
    Packet* output = data->getPacket(data->currentTimestamp - data->delayPackets);

    for( int i=0; i<numSamples; i++ ) {
        write[i] = output ? output->samples[i] : 0;
    }

    if(output)
        delete output;

    data->currentTimestamp += 1;

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
    paData data;

    printf("patest_record.c\n"); fflush(stdout);

  

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
    printf("\n=== Now recording!! Please speak into the microphone. ===\n"); fflush(stdout);

    while( true )
    {
        Pa_Sleep(1000);
        // printf("index = %d\n", data.frameIndex ); fflush(stdout);
    }

    done:
    Pa_Terminate();

    return err;
}
