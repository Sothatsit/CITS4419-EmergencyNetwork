// Include GPS parser
#include <TinyGPSPlus.h>
TinyGPSPlus gps;

// Include LoRa PHY library
#include <LoRa.h>

// Include lab utility library
#include <lab_utils.h>

// Include the flooding protocol library
#include <flooding_protocol.h>

// Common experiment definitions
#include "experiment.h"

#include <time.h>
#define MAX_NUM_CHARS 32

void setup()
{
  do_lab_setup(115200, BAND);

  // Allow time to attach a console
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

void send_packet(int i, int j)
{
  String msg;
  smart_delay(DELAY / 2);

  // Add the comm ID (includes trailing comma itself)
  msg += COMM_ID;

  // Add the param number and packet number
  msg += i;
  msg += ',';
  msg += j;

  // Add the GPS coordinates of the sender
  msg += ',';
  msg += String(gps.location.lat(), 6);
  msg += ',';
  msg += String(gps.location.lng(), 6);

  // Print the data to the console
  Serial.println(msg);

  // Send data in a LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();

  delay(DELAY / 2);
}

void loop()
{
  use_LoRa_params(params[0]);
  delay(DELAY * 2);

  clock_t start = clock();

  int packetID = 1;
  while (true) {
    delay(DELAY);

    int timestampMS = (int) (clock() - start);// / (CLOCKS_PER_SEC / 1000);

    PacketHeader header;
    header.communicationID = COMM_ID;
    header.packetID = packetID;
    header.sourceNodeID = NODE_ID;
    header.destNodeID = 0;  // Broadcast
    header.hopCount = 1;

    PacketNodeInfo newNodeInfo;
    newNodeInfo.nodeID = NODE_ID;
    newNodeInfo.timestampMS = timestampMS;
    newNodeInfo.gpsLat = (int) (gps.location.lat() * FP_GPS_SCALE);
    newNodeInfo.gpsLon = (int) (gps.location.lng() * FP_GPS_SCALE);

    Packet packet;
    packet.header = header;
    packet.nodeInfoCount = 1;
    packet.nodeInfo = &newNodeInfo;
    char * writtenPacket = writePacket(&packet);

    Serial.print("SEND: ");
    Serial.println(writtenPacket);

    LoRa.beginPacket();
    LoRa.print(writtenPacket);
    LoRa.endPacket();

    free(writtenPacket);
    packetID += 1;
  }
}
