//
//  packet.h
//  jammin
//
//  Created by Jonah Fox on 01/01/2021.
//  Copyright © 2021 Jonah Fox. All rights reserved.
//

#ifndef packet_h
#define packet_h

#define BUFFER_SIZE 100

class Packet {
public:
    char type[4]="PCM";// {'P','C','M',' '}; // 'PCM '
    int numBytes=0;
    int timestamp=-1;
    int numSamples=0; //
    SAMPLE* samples;
    Packet(const SAMPLE *read, int _numSamples, int _timestamp) {
        numSamples = _numSamples;
        numBytes = numSamples*sizeof(SAMPLE)+4*sizeof(int);
        samples = new SAMPLE[numSamples];
        
        for( int i=0; i<numSamples; i++ ) {
            samples[i] = read ? read[i] : 0;
        }
        timestamp = _timestamp;
    };
    float rms() {
        float sum=0;
        for( int i=0; i<numSamples; i++ ) {
            sum += samples[i]*samples[i];
        }
        return (sum/32768.0/numSamples);// dont bother with sqrt
    }
    
    ~Packet() {
        if(samples)
            delete samples;
        samples = NULL;
    }
};



// 250ms of audio
class PacketBuffer {
public:
    Packet* packets[BUFFER_SIZE];
    PacketBuffer() {
        for(int i=0; i<BUFFER_SIZE; i++) {
            packets[i] = 0;
        }
    }


    
    void push(Packet* p) {
        int index = (p->timestamp+BUFFER_SIZE)%BUFFER_SIZE;
        if(packets[index]) {
            delete packets[index];
        }
        packets[index] = p;
    }
    
    Packet* pull(int timestamp) {       
        int index = (timestamp+BUFFER_SIZE)%BUFFER_SIZE;
        Packet* p = packets[index];
        if(!p) {
            return 0;
        }
        if(p->timestamp != timestamp) {
            fprintf(stderr, "wrong timestamp %lld != %lld\n", (long long) p->timestamp, (long long)  timestamp );
            packets[index] = 0;
            delete p;
            return 0;
        }
        
        packets[index] = 0;
        return p;
    }
    
};






#endif /* packet_h */
