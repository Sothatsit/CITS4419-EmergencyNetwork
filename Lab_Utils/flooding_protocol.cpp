#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "flooding_protocol.h"


#define MAX_NUM_CHARS 32


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

PacketHeader * parsePacketHeader(char * packet) {
    PacketHeader * result = (PacketHeader *) malloc(sizeof(PacketHeader));
    if (result == nullptr) {
        return nullptr;
    }

    char * communicationID = (char *) malloc(sizeof(char) * (COMM_ID_LENGTH + 1));
    if (communicationID == nullptr) {
        return nullptr;
    }

    int index = 0;
    bool success;

    // Read communication ID.
    index = read_csv_field(packet, index, communicationID, COMM_ID_LENGTH + 1);
    result->communicationID = communicationID;

    // Read timestamp of packet.
    char * timestamp = (char *) malloc(sizeof(char) * (MAX_NUM_CHARS + 1));
    if (timestamp == nullptr) {
        return nullptr;
    }
    index = read_csv_field(packet, index, timestamp, MAX_NUM_CHARS + 1);
    result->time = timestamp;

    // Read Node ID.
    char numberBuffer[MAX_NUM_CHARS + 1];
    index = read_csv_field(packet, index, numberBuffer, MAX_NUM_CHARS + 1);
    success = str_to_int(numberBuffer, &result->nodeID);
    if (!success) {
        return nullptr;
    }
    
    // Read Packet ID.
    index = read_csv_field(packet, index, numberBuffer, MAX_NUM_CHARS);
    success = str_to_int(numberBuffer, &result->packetID);
    if (!success) {
        return nullptr;
    }
    
    // Read Hop Count.
    index = read_csv_field(packet, index, numberBuffer, MAX_NUM_CHARS);
    success = str_to_int(numberBuffer, &result->hopCount);
    if (!success) {
        return nullptr;
    }

    // Read GPS Latitude.
    index = read_csv_field(packet, index, numberBuffer, MAX_NUM_CHARS);
    success = str_to_int(numberBuffer, &result->gpsLat);
    if (!success) {
        return nullptr;
    }

    // Read GPS Longitdue.
    index = read_csv_field(packet, index, numberBuffer, MAX_NUM_CHARS);
    success = str_to_int(numberBuffer, &result->gpsLon);
    if (!success) {
        return nullptr;
    }

    return result;
}

void freePacketHeader(PacketHeader ** headerVar) {
    PacketHeader *header = *headerVar;
    free(header->communicationID);
    free(header);
    *headerVar = nullptr;
}

char * writePacketHeader(PacketHeader * header) {
    int maxPacketLength = COMM_ID_LENGTH + 6 * (1 + MAX_NUM_CHARS);
    char * packet = (char *) malloc(sizeof(char) * (maxPacketLength + 1));
    if (packet == nullptr)
        return nullptr;

    int written = snprintf(
        packet,
        maxPacketLength + 1,
        "%s,%s,%d,%d,%d,%d,%d",
        header->communicationID,
        header->time,
        header->nodeID,
        header->packetID,
        header->hopCount,
        header->gpsLat,
        header->gpsLon
    );
    if (written < 0 || written >= maxPacketLength) {
        free(packet);
        return nullptr;
    }
    packet[written] = '\0';
    return packet;
}

int main() {
    char * headerStr = (char *) "AAAA,00:00:00,123,256,777,100,101";
    PacketHeader * header = parsePacketHeader(headerStr);
    if (header) {
        char * written = writePacketHeader(header);

        printf("commID = %s\nnodeID = %d\npacketID = %d\nhopCount = %d\npacket = %s\n", header->communicationID, header->nodeID, header->packetID, header->hopCount, written);
    } else {
        printf("null\n");
    }
    
    freePacketHeader(&header);

    return 0;
}
