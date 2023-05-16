#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <string.h>

#include "flooding_protocol.h"

/**
 * Produces a hash of the given string.
 * http://www.cse.yorku.ca/~oz/hash.html
 */
unsigned long hash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = (hash * 33) ^ c;
    }
    return hash;
}

/**
 * Writes characters to result that represent the hash of str.
 */
void hash_as_str(char *str, char *result) {
    unsigned long hashValue = hash(str);

    const char *hashChars = FP_HASH_CHARS;
    int hashCharCount = strlen(hashChars);

    for (int index = 0; index < FP_HASH_LENGTH; ++index) {
        int value = (int) (hashValue % hashCharCount);
        hashValue /= hashCharCount;
        result[index] = hashChars[value];
    }
}

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

bool str_to_float(const char *str, double *result) {
    int intResult;
    if (!str_to_int(str, &intResult)) {
        return false;
    }

    *result = ((double) intResult) / ((double) FP_FIXED_POINT_SCALE);
    return true;
}

int float_to_int(double value) {
    return (int) (((double) value) * ((double) FP_FIXED_POINT_SCALE));
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

    // Read Node ID.
    char numberBuffer[FP_MAX_PACKET_NUMBER_CHARS + 1];
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS + 1);
    success = str_to_int(numberBuffer, &result->nodeID);
    if (!success) {
        return -1;
    }

    // Read timestamp of packet.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_int(numberBuffer, &result->timestampMS);
    if (!success) {
        printf("b\n");
        return -1;
    }

    // Read Packet RSSI.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_float(numberBuffer, &result->packetRSSI);
    if (!success) {
        return -1;
    }

    // Read RSSI.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_float(numberBuffer, &result->RSSI);
    if (!success) {
        return -1;
    }

    // Read Packet SNR.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_float(numberBuffer, &result->packetSNR);
    if (!success) {
        return -1;
    }

    // Read GPS Latitude.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_float(numberBuffer, &result->gpsLat);
    if (!success) {
        return -1;
    }

    // Read GPS Longitdue.
    index = read_csv_field(packet, index, numberBuffer, FP_MAX_PACKET_NUMBER_CHARS);
    success = str_to_float(numberBuffer, &result->gpsLon);
    if (!success) {
        return -1;
    }

    return index;
}

Packet * parsePacket(char * packet, char * expectedCommunicationID) {
    // Read the hash.
    for (int index = 0; index < FP_HASH_LENGTH; ++index) {
        if (packet[index] == '\0') {
            return nullptr;
        }
    }
    if (packet[FP_HASH_LENGTH] != ':') {
        return nullptr;
    }
    char * receivedHash = packet;
    char * packetContent = &packet[FP_HASH_LENGTH + 1];

    // Read the packet.
    Packet * result = (Packet *) malloc(sizeof(Packet));
    if (result == nullptr) {
        return nullptr;
    }

    int index = parsePacketHeader(packetContent, expectedCommunicationID, &result->header);
    if (index < 0) {
        free(result);
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
        free(result);
        return nullptr;
    }

    for (int hop = 0; hop < result->nodeInfoCount; ++hop) {
        index = parsePacketNodeInfo(packetContent, index, &nodeInfo[hop]);
        if (index < 0) {
            free(result);
            free(nodeInfo);
            return nullptr;
        }
    }

    // Verify the hash.
    char contentHash[FP_HASH_LENGTH];
    hash_as_str(packetContent, contentHash);
    for (int index = 0; index < FP_HASH_LENGTH; ++index) {
        if (contentHash[index] != receivedHash[index]) {
            free(result);
            free(nodeInfo);
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
    int maxPacketLength = FP_HASH_LENGTH + 1 + FP_COMM_ID_LENGTH + 4 * (1 + FP_MAX_PACKET_NUMBER_CHARS);
    for (int hop = 0; hop < packet->header.hopCount; ++hop) {
        maxPacketLength += 4 * (1 + FP_MAX_PACKET_NUMBER_CHARS);
    }

    char * writtenPacket = (char *) malloc(sizeof(char) * (maxPacketLength + 1));
    if (writtenPacket == nullptr) {
        return nullptr;
    }

    // We will write the hash of the packet later.
    char * hashStart = writtenPacket;
    char * contentStart = &writtenPacket[FP_HASH_LENGTH + 1];
    char * writeHead = contentStart;

    // Write the packet header.
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
            ",%d,%d,%d,%d,%d,%d,%d",
            nodeInfo->nodeID,
            nodeInfo->timestampMS,
            float_to_int(nodeInfo->packetRSSI),
            float_to_int(nodeInfo->RSSI),
            float_to_int(nodeInfo->packetSNR),
            float_to_int(nodeInfo->gpsLat),
            float_to_int(nodeInfo->gpsLon)
        );
        writeHead = &writeHead[written];
        if (written < 0 || written >= maxPacketLength) {
            free(writtenPacket);
            return nullptr;
        }
    }

    // Finish the packet content.
    writeHead[written] = '\0';

    // Take a hash of the packet content and write it to the front of the packet.
    hash_as_str(contentStart, hashStart);
    hashStart[FP_HASH_LENGTH] = ':';

    return writtenPacket;
}

int main() {
    char * expectedCommunicationID = (char *) "AAAA";
    char * headerStr = (char *) "5U3M:AAAA,122,123,256,1,3,4,99999,1023040,123123,5,6";
    Packet * packet = parsePacket(headerStr, expectedCommunicationID);

    if (packet) {
        printf("communicationID = %s\n", packet->header.communicationID);
        printf("packetID = %d\n", packet->header.packetID);
        printf("sourceNodeID = %d\n", packet->header.sourceNodeID);
        printf("destNodeID = %d\n", packet->header.destNodeID);
        printf("hopCount = %d\n", packet->header.hopCount);
        printf("nodeID = %d\n", packet->nodeInfo->nodeID);
        printf("timestampMS = %d\n", packet->nodeInfo->timestampMS);
        printf("packetRSSI = %.6f\n", packet->nodeInfo->packetRSSI);
        printf("RSSI = %.6f\n", packet->nodeInfo->RSSI);
        printf("packetSNR = %.6f\n", packet->nodeInfo->packetSNR);
        printf("gpsLat = %.6f\n", packet->nodeInfo->gpsLat);
        printf("gpsLon = %.6f\n", packet->nodeInfo->gpsLon);

        char * rewritten = writePacket(packet);
        printf("rewritten = %s\n", rewritten);
    } else {
        printf("The packet is corrupted\n");
    }

    freePacket(&packet);
    return 0;
}
