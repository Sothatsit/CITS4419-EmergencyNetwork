#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "flooding_protocol.h"


/**
 * Returns the index of the start of the next CSV field.
 */
int read_csv_field(char * packet, int index, char * destination, int maxLength) {
    // Propogate errors w/o doing anything.
    if (index < 0) {
        destination[0] = '\0';
        return index;
    }

    int writeIndex = 0;
    while (packet[index] != '\0' && packet[index] != ',') {
        destination[writeIndex] = packet[index];
        index += 1;
        writeIndex += 1;
        if (writeIndex >= maxLength) {
            destination[0] = '\0';
            return -1;
        }
    }
    destination[writeIndex] = '\0';
    if (packet[index] == '\0')
        return index;

    return index + 1;
}

bool str_to_int(const char *str, int *result) {
    if (str == nullptr || *str == '\0') {
        return false;
    }

    char *endptr;
    long long_res = strtol(str, &endptr, 10);

    // Check if the string contains invalid characters or overflow occurred
    if (*endptr != '\0' || errno == ERANGE || long_res > INT_MAX || long_res < INT_MIN) {
        return false;
    }

    *result = (int)long_res;
    return true;
}


int parsePacketHeader(char * packet, char * expectedCommunicationID, PacketHeader * result) {
    int index = 0;
    bool success;
    char numberBuffer[FP_MAX_PACKET_NUMBER_CHARS + 1];

    // Read Communication ID.
    char * communicationID = (char *) malloc(sizeof(char) * (FP_COMM_ID_LENGTH + 1));
    if (communicationID == nullptr) {
        return -1;
    }

    index = read_csv_field(packet, index, communicationID, FP_COMM_ID_LENGTH + 1);
    result->communicationID = communicationID;
    if (strcmp(communicationID, expectedCommunicationID) != 0) {
        return -1;
    }
    
    // Read Packet ID.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_int(numberBuffer, &result->packetID);
    if (!success) {
        return -1;
    }

    // Read Source Node ID.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS + 1);
    success = str_to_int(numberBuffer, &result->sourceNodeID);
    if (!success) {
        return -1;
    }

    // Read Destination Node ID.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS + 1);
    success = str_to_int(numberBuffer, &result->destNodeID);
    if (!success) {
        return -1;
    }
    
    // Read Hop Count.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_int(numberBuffer, &result->hopCount);
    if (!success) {
        return -1;
    }

    return index;
}


int parsePacketNodeInfo(char * packet, int index, PacketNodeInfo * result) {
    bool success;
    printf("parsePacketNodeInfo\n");

    // Read Node ID.
    char numberBuffer[FP_MAX_PACKET_NUMBER_CHARS + 1];
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS + 1);
    success = str_to_int(numberBuffer, &result->nodeID);
    if (!success) {
        printf("a\n");
        return -1;
    }

    // Read timestamp of packet.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_int(numberBuffer, &result->timestampMS);
    if (!success) {
        printf("b\n");
        return -1;
    }

    // Read GPS Latitude.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_int(numberBuffer, &result->gpsLat);
    if (!success) {
        printf("c\n");
        return -1;
    }

    // Read GPS Longitdue.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    printf("%s\n", numberBuffer);
    success = str_to_int(numberBuffer, &result->gpsLon);
    if (!success) {
        printf("d\n");
        return -1;
    }

    return index;
}

Packet * parsePacket(char * packet, char * expectedCommunicationID) {
    Packet * result = (Packet *) malloc(sizeof(Packet));
    if (result == nullptr) {
        return nullptr;
    }

    int index = parsePacketHeader(packet, expectedCommunicationID, &result->header);
    if (index < 0) {
        return nullptr;
    }

    result->nodeInfoCount = result->header.hopCount;
    if (result->nodeInfoCount == 0) {
        result->nodeInfo = nullptr;
        return result;
    }

    PacketNodeInfo * nodeInfo = (PacketNodeInfo *) malloc(sizeof(PacketNodeInfo) * result->nodeInfoCount);
    result->nodeInfo = nodeInfo;
    if (nodeInfo == nullptr) {
        return nullptr;
    }

    for (int hop = 0; hop < result->nodeInfoCount; ++hop) {
        index = parsePacketNodeInfo(packet, index, &nodeInfo[hop]);
        if (index < 0) {
            return nullptr;
        }
    }
    return result;
}


void freePacket(Packet ** packetVar) {
    Packet *packet = *packetVar;
    if (!packet) {
        return;
    }

    free(packet->header.communicationID);
    free(packet->nodeInfo);
    free(packet);
    *packetVar = nullptr;
}

char * writePacket(Packet * packet) {
    // Just allocate as much as we could possibly need,
    // as it won't be that much memory anyway.
    int maxPacketLength = FP_COMM_ID_LENGTH + 4 * (1 + FP_MAX_PACKET_NUMBER_CHARS);
    for (int hop = 0; hop < packet->header.hopCount; ++hop) {
        maxPacketLength += 4 * (1 + FP_MAX_PACKET_NUMBER_CHARS);
    }

    char * writtenPacket = (char *) malloc(sizeof(char) * (maxPacketLength + 1));
    if (writtenPacket == nullptr) {
        return nullptr;
    }

    // Write the packet header.
    char * writeHead = writtenPacket;
    int written = snprintf(
        writeHead,
        maxPacketLength + 1,
        "%s,%d,%d,%d,%d",
        packet->header.communicationID,
        packet->header.packetID,
        packet->header.sourceNodeID,
        packet->header.destNodeID,
        packet->header.hopCount
    );
    writeHead = &writeHead[written];
    if (written < 0 || written >= maxPacketLength) {
        free(writtenPacket);
        return nullptr;
    }

    // Write each node info.
    PacketNodeInfo * nodeInfos = packet->nodeInfo;
    for (int hop = 0; hop < packet->header.hopCount; ++hop) {
        PacketNodeInfo * nodeInfo = &nodeInfos[hop];
        written = snprintf(
            writeHead,
            maxPacketLength + 1,
            ",%d,%d,%d,%d",
            nodeInfo->nodeID,
            nodeInfo->timestampMS,
            nodeInfo->gpsLat,
            nodeInfo->gpsLon
        );
        writeHead = &writeHead[written];
        if (written < 0 || written >= maxPacketLength) {
            free(writtenPacket);
            return nullptr;
        }
    }

    writeHead[written] = '\0';
    return writtenPacket;
}

int main() {
    char * expectedCommunicationID = (char *) "AAAA";
    char * headerStr = (char *) "AAAA,122,123,256,1,3,4,5,6";
    Packet * packet = parsePacket(headerStr, expectedCommunicationID);
    if (packet) {
        printf("%lld\n", packet->nodeInfo);
        char * written = writePacket(packet);

        printf("%s\n", written);
    } else {
        printf("It broke\n");
    }

    freePacket(&packet);
    return 0;
}
