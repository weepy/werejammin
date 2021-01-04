//
//  packet.h
//  jammin
//
//  Created by Jonah Fox on 01/01/2021.
//  Copyright © 2021 Jonah Fox. All rights reserved.
//

#ifndef packet_h
#define packet_h

#define BUFFER_SIZE 1000


class Packet {
public:
    int type;
    int numBytes=0;
    long long timestamp=-1;
    int numSamples;
    char data;
    
//    static Packet* create(const char* _data, int numBytes) {
//        char* data = (char*) malloc(numBytes);
//
//        for( int i=0; i<numBytes; i++ ) {
//            data[i] = _data[i];
//        }
//
//        return (Packet*)data;
//    }
    
//    static Packet* createPcm(const SAMPLE* _samples, int numSamples, long long timestamp) {
//        int numBytes = numSamples*sizeof(SAMPLE) + 3*sizeof(int)+sizeof(long long);
//
//        Packet* packet = (Packet*) malloc(numBytes);
//
//        packet->type = 1;
//        packet->numBytes = numBytes;
//        packet->numSamples = numSamples;
//        packet->timestamp = timestamp;
//
//        SAMPLE* samples = (SAMPLE*) (&packet->data);
//
//        for( int i=0; i<numSamples; i++ ) {
//            samples[i] = _samples[i];
//        }
//
//        return packet;
//    }
//
    static Packet* create(int type, const char* buf, int len, int numSamples, long long timestamp) {
        int numBytes = len + 3*sizeof(int)+sizeof(long long);
        
        Packet* packet = (Packet*) malloc(numBytes);
        
        packet->type = type;
        packet->numBytes = numBytes;
        packet->numSamples = numSamples;
        packet->timestamp = timestamp;
        
        char* data = &packet->data;
        
        for( int i=0; i<len; i++ ) {
            data[i] = buf[i];
        }
        
        return packet;
    }
    
    
    int dataLength() {
        return numBytes - ( 3*sizeof(int)+sizeof(long long) );
    }
//    static Packet* createOpus(OpusEncoder* enc, const SAMPLE* _samples, int numSamples, long long timestamp) {
//
//        opus_encode(end, _samples, numSamples, <#unsigned char *data#>, <#opus_int32 max_data_bytes#>);
//
//        int numBytes = numSamples*sizeof(SAMPLE) + 3*sizeof(int)+sizeof(long long);
//
//        Packet* packet = (Packet*) malloc(numBytes);
//
//        packet->type = 1;
//        packet->numBytes = numBytes;
//        packet->numSamples = numSamples;
//        packet->timestamp = timestamp;
//
//        SAMPLE* samples = (SAMPLE*) (&packet->data);
//
//        for( int i=0; i<numSamples; i++ ) {
//            samples[i] = _samples[i];
//        }
//
//        return packet;
//    }
//
    

    static void destroy(Packet* p) {
        free(p);
    }
    
    SAMPLE* samples() {
        return (SAMPLE*) (&data);
    }
   
};



float rms(SAMPLE* samples, int numSamples) {
    float sum=0;
    for( int i=0; i<numSamples; i++ ) {
        sum += samples[i]*samples[i];
    }
    return (sum/32768.0/numSamples);// dont bother with sqrt
}



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
            Packet::destroy(packets[index]);
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
            Packet::destroy(p);
            return 0;
        }
        
        packets[index] = 0;
        return p;
    }
    
};






#endif /* packet_h */
