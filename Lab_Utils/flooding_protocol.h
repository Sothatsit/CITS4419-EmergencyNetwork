#ifndef FLOODING_PROTOCOL_H
#define FLOODING_PROTOCOL_H


#define COMM_ID_LENGTH 4


typedef struct
{
    char * communicationID;
    char * time;
    int nodeID;
    int packetID;
    int hopCount;
    int gpsLat;
    int gpsLon;
} PacketHeader;

PacketHeader * parsePacketHeader(char * packet);

void freePacketHeader(PacketHeader ** header);

char * writePacketHeader(PacketHeader * header);

int write_to_text(char * str);

#endif
