
#define BUFFER_SIZE 100

struct Packet {
    int type; // 'PCM '
    int numBytes;
    int timestamp;
    int numSamples; // 
    SAMPLE* samples;
};

struct Packet* Packet_create(const SAMPLE *read, int numSamples, int _timestamp) {
    struct Packet* packet = malloc(sizeof(struct Packet));

    packet->type= 'PCM ';
    packet->numSamples = numSamples;
    packet->numBytes = numSamples*sizeof(SAMPLE)+4*sizeof(int);
    packet->samples = malloc(numSamples*sizeof(SAMPLE));

    for( int i=0; i<numSamples; i++ ) {
        packet->samples[i] = read[i];
    }
    packet->timestamp = _timestamp;
    return packet;
};

void Packet_destroy(struct Packet* p) {
    free(p->samples);
}

// 250ms of audio
struct PacketBuffer {
    struct Packet* packets[BUFFER_SIZE];
};

struct PacketBuffer* PacketBuffer_create() {
    struct PacketBuffer* pb = malloc(sizeof(struct PacketBuffer));
    for(int i=0; i<BUFFER_SIZE; i++) {
        pb->packets[i] = 0;
    }
    return pb;  
};

void PacketBuffer_push(struct PacketBuffer* pb, struct Packet* p) {
    int index = (p->timestamp+BUFFER_SIZE)%BUFFER_SIZE;
    if(pb->packets[index]) {
        Packet_destroy(pb->packets[index]);
    }
    pb->packets[index] = p;
}  

struct Packet* PacketBuffer_pull(struct PacketBuffer* pb,int timestamp) {
    int index = (timestamp+BUFFER_SIZE)%BUFFER_SIZE;
    struct Packet* p = pb->packets[index];
    if(!p) {
        return 0;
    }
    if(p->timestamp != timestamp) {
        fprintf(stderr, "wrong timestamp %lld != %lld\n", (long long) p->timestamp, (long long)  timestamp );
        pb->packets[index] = 0;
        Packet_destroy(p);
        return 0;
    }
    
    pb->packets[index] = 0;
    return p;
}