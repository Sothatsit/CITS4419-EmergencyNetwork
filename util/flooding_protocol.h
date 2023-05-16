#ifndef FLOODING_PROTOCOL_H
#define FLOODING_PROTOCOL_H


/**
 * The length of the communication ID field.
 */
#define FP_COMM_ID_LENGTH 4

/**
 * The maximum number of characters used to write a single number into a packet.
 */
#define FP_MAX_PACKET_NUMBER_CHARS 32

/**
 * We send floating point values as ints that are the rounded result
 * of multipling the floating point values by this scale.
 * Reason for Choice: 1-meter GPS precision requires 5 decimal places.
 */
#define FP_FIXED_POINT_SCALE 100000

/**
 * The number of characters to use to write a hash of each packet.
 */
#define FP_HASH_LENGTH 4

/**
 * The characters to use to write the hash to the packet.
 */
#define FP_HASH_CHARS "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"


/**
 * One header per packet.
 */
typedef struct
{
    /**
     * The communication ID is used to filter out irrelevant traffic.
     */
    char * communicationID;

    /**
     * A unique ID for this packet.
     */
    int packetID;

    /**
     * The ID of the node that last broadcast this packet.
     */
    int sourceNodeID;

    /**
     * The node ID that is the target destination of this packet.
     */
    int destNodeID;

    /**
     * The number of nodes that this packet has been forwarded from.
     */
    int hopCount;

} PacketHeader;


/**
 * One node info per each hop a packet has taken.
 */
typedef struct
{

    /**
     * The ID of this node that was hopped past.
     */
    int nodeID;

    /**
     * The time that this packet was sent, in milliseconds.
     * This is only relevant to the node that the time was
     * recorded on, as the timestamp is relative to when
     * the node was turned on.
     * 
     * This will overflow and break at 24 days of uptime.
     */
    int timestampMS;

    /**
     * The Received Signal Strength Indicator of the packet.
    */
    double packetRSSI;

    /**
     * The Received Signal Strength Indicator.
    */
    double RSSI;

    /**
     * The Signal to Noise Ratio.
     */
    double packetSNR;

    /**
     * The latitude of this node, from the GPS.
     */
    double gpsLat;

    /**
     * The longitude of this node, from the GPS.
     */
    double gpsLon;

} PacketNodeInfo;


/**
 * The whole packet, including a header and the
 * info about each of node its hopped through.
 */
typedef struct
{
    /**
     * The header of this packet.
     */
    PacketHeader header;

    /**
     * The number of past hops of this packet.
     */
    int nodeInfoCount;

    /**
     * An array of info about each node that was hopped through.
     */
    PacketNodeInfo * nodeInfo;
} Packet;

/**
 * Read the packet header from the given packet text.
 */
Packet * parsePacket(char * packet, char * expectedCommunicationID);

/**
 * Frees the resources allocated with the given packet header.
 */
void freePacket(Packet ** header);

/**
 * Writes the given packet header to packet text to be transmitted.
 */
char * writePacket(Packet * header);



int write_to_text(char * str);

#endif
