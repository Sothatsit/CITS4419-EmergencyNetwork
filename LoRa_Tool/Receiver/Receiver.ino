// Include GPS parser
#include <TinyGPSPlus.h>
TinyGPSPlus gps;

// Include LoRa PHY library
#include <LoRa.h>

// Include lab utility library
#include <lab_utils.h>

// Include the flooding protocol library
#include <flooding_protocol.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Common experiment definitions
#include "experiment.h"


#define PACKET_ID_HISTORY_COUNT 256
#define NO_PACKET_ID_SPECIAL_VALUE -1

int seenPacketIDs[PACKET_ID_HISTORY_COUNT];

void clearSeenPacketIDs() {
  for (int index = 0; index < PACKET_ID_HISTORY_COUNT; ++index) {
    seenPacketIDs[index] = NO_PACKET_ID_SPECIAL_VALUE;
  }
}

bool hasSeenPacketID(int packetID) {
  for (int index = 0; index < PACKET_ID_HISTORY_COUNT; ++index) {
    if (seenPacketIDs[index] == packetID)
      return true;
  }
  return false;
}

void addSeenPacketID(int packetID) {
  for (int index = PACKET_ID_HISTORY_COUNT - 1; index > 0; --index) {
    seenPacketIDs[index] = seenPacketIDs[index - 1];
  }
  seenPacketIDs[0] = packetID;
}


void setup()
{
  clearSeenPacketIDs();
  do_lab_setup(115200, BAND);
  delay(DELAY * 2);
}

static void smart_delay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (GPSSerial.available())
    {
      gps.encode(GPSSerial.read());
    }
  } while (millis() - start < ms);
}

void print_gps()
{
  Serial.print("{");

  // Print my GPS time
  Serial.print("\"time\": \"");
  if (gps.time.hour() < 10) Serial.print("0");
  Serial.print(gps.time.hour());

  Serial.print(":");
  if (gps.time.minute() < 10) Serial.print("0");
  Serial.print(gps.time.minute());

  Serial.print(":");
  if (gps.time.second() < 10) Serial.print("0");
  Serial.print(gps.time.second());
  Serial.print("\", ");

  // Print my GPS location
  Serial.print("\"lat\": \"");
  Serial.print(String(gps.location.lat(), 6));
  Serial.print("\", ");

  Serial.print("\"lng\": \"");
  Serial.println(String(gps.location.lng(), 6));
  Serial.print("\"");

  Serial.print("}");
}

void print_lora_params()
{
  Serial.print("{");
  
  // Print LoRa parameters.
  Serial.print("\"LoRa\": ");
  Serial.print(",");
  Serial.print(cur_params.bandwidth);

  Serial.print(",");
  Serial.print(cur_params.codingRate);

  Serial.print(",");
  Serial.print(cur_params.frequency);

  Serial.print(",");
  Serial.print(cur_params.spreadingFactor);

  Serial.print(",");
  Serial.print(cur_params.txPower);

  Serial.print("}");
}

void print_lora_metrics()
{
  Serial.print("{");
  
  // Print LoRa metrics
  Serial.print(",");
  Serial.print(LoRa.packetRssi());

  Serial.print(",");
  Serial.print(LoRa.rssi());

  Serial.print(",");
  Serial.print(LoRa.packetSnr());

  Serial.print("}");
}

void print_packet(int sequenceNumber, String& data_json)
{
  Serial.print("{\n");

  // Print sequence number.
  Serial.print("\"sequence\": ");
  Serial.print(sequenceNumber);
  Serial.print(",\n");

  // Print packet contents.
  Serial.print("\"data\": ");
  Serial.print(data_json);
  Serial.print(",\n");

  // Print GPS information.
  Serial.print("\"gps\": ");
  print_gps();
  Serial.print(",\n");

  // Print LoRa parameters.
  Serial.print("\"LoRa\": ");
  print_lora_params();
  Serial.print(",\n");

  // Print LoRa metrics.
  Serial.print("\"metrics\": ");
  print_lora_metrics();

  Serial.print("}\n");
}

char * read_packet()
{
  int packet_sz = LoRa.parsePacket();
  if (!packet_sz)
    return nullptr;
  
  String LoRa_data;
  while (LoRa.available())
  {
    LoRa_data += LoRa.readString();
  }

  int length = LoRa_data.length();
  char * data = (char *) malloc(sizeof(char) * (length + 1));
  if (data) {
    for (int index = 0; index < length; ++index) {
      data[index] = LoRa_data[index];
    }
    data[length] = '\0';
  }
  return data;

  // if (packet_sz)
  // {
  //   smart_delay(DELAY / 2);

  //   // Get packet contents
  //   String LoRa_data;
  //   while (LoRa.available())
  //   {
  //     LoRa_data += LoRa.readString();
  //   }


  //   // Reject packet if not starting with COMM_ID
  //   if (!LoRa_data.startsWith(COMM_ID))
  //   {
  //     return;
  //   }

  //   // Extract Pkt_Set and reject packet if not from expected set
  //   int pkt_set_idx_start = strlen(COMM_ID);
  //   int pkt_set_idx_end = LoRa_data.indexOf(',', pkt_set_idx_start);
  //   int actual_pkt_set = LoRa_data.substring(pkt_set_idx_start, pkt_set_idx_end).toInt();

  //   if (actual_pkt_set != expect_pkt_set)
  //   {
  //     consec_wrong_pkts++;
  //     return;
  //   }

  //   // Print the packet data
  //   print_packet(LoRa_data);

  //   // Expect the next packet after the actual current one
  //   int pkt_id_idx_start = pkt_set_idx_end + 1;
  //   int pkt_id_idx_end = LoRa_data.indexOf(',', pkt_id_idx_start);
  //   int actual_pkt = LoRa_data.substring(pkt_id_idx_start, pkt_id_idx_end).toInt();

  //   expect_pkt = actual_pkt + 1;

  //   // And report a successful reception
  //   last_pkt_time = millis();

  //   // Breaks the chain of wrong packets
  //   consec_wrong_pkts = 0;
  // }
}

void loop()
{
  use_LoRa_params(params[0]);

  clock_t start = clock();

  while (true) {
    if (GPSSerial.available()) {
      gps.encode(GPSSerial.read());
    }

    char * inPacket = read_packet();
    if (inPacket == nullptr) {
      continue;
    }

    Packet * packet = parsePacket(inPacket, COMM_ID);
    if (packet == nullptr) {
      Serial.print("CORRUPT: ");
      Serial.println(inPacket);
      continue;
    }

    // Skip packets we've already seen.
    if (hasSeenPacketID(packet->header.packetID)) {
      Serial.print("DUPLICATE: ");
      Serial.println(inPacket);
      continue;
    }

    Serial.print("RECV: ");
    Serial.println(inPacket);

    addSeenPacketID(packet->header.packetID);
  
    int timestampMS = (int) (clock() - start);

    PacketHeader newHeader;
    newHeader.communicationID = COMM_ID;
    newHeader.packetID = packet->header.packetID;
    newHeader.sourceNodeID = NODE_ID;
    newHeader.destNodeID = packet->header.destNodeID;
    newHeader.hopCount = packet->header.hopCount + 1;

    PacketNodeInfo newNodeInfo;
    newNodeInfo.nodeID = NODE_ID;
    newNodeInfo.timestampMS = timestampMS;
    newNodeInfo.packetRSSI = (double) LoRa.packetRssi();
    newNodeInfo.RSSI = (double) LoRa.rssi();
    newNodeInfo.packetSNR = (double) LoRa.packetSnr();
    newNodeInfo.gpsLat = (double) gps.location.lat();
    newNodeInfo.gpsLon = (double) gps.location.lng();

    Packet newPacket;
    newPacket.header = newHeader;
    newPacket.nodeInfoCount = packet->nodeInfoCount + 1;
    newPacket.nodeInfo = (PacketNodeInfo *) malloc(sizeof(PacketNodeInfo) * newPacket.nodeInfoCount);
    memcpy(newPacket.nodeInfo, packet->nodeInfo, sizeof(PacketNodeInfo) * packet->nodeInfoCount);
    newPacket.nodeInfo[packet->nodeInfoCount] = newNodeInfo;

    char * writtenPacket = writePacket(&newPacket);

    Serial.print("SEND: ");
    Serial.println(writtenPacket);

    LoRa.beginPacket();
    LoRa.print(writtenPacket);
    LoRa.endPacket();

    freePacket(&packet);
    free(newPacket.nodeInfo);
    free(writtenPacket);
    free(inPacket);
  }
}
