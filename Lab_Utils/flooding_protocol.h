#ifndef FLOODING_PROTOCOL_H
#define FLOODING_PROTOCOL_H


#define COMM_ID_LENGTH 4


typedef struct
{
    char * communicationID;
    int nodeID;
    int packetID;
    int hopCount;
} PacketHeader;

PacketHeader * parsePacketHeader(char * packet);

void freePacketHeader(PacketHeader ** header);

char * writePacketHeader(PacketHeader * header);


#endif
