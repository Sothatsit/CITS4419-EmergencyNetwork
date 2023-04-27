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

  int packetID = 1;
  while (true) {
    delay(DELAY);

    time_t mytime;
    mytime = time(NULL);
    char * timestring = ctime(&mytime);
    timestring[strlen(timestring)-1] = '\0';

    PacketHeader header;
    header.communicationID = COMM_ID;

    header.time = timestring;
    header.nodeID = NODE_ID;
    header.packetID = packetID;
    header.hopCount = 0;
    header.gpsLat = gps.location.lat();
    header.gpsLon = gps.location.lng();

    char * packet = writePacketHeader(&header);

    // TODO : Append GPS data to packet.
    
    Serial.println(packet);

    LoRa.beginPacket();
    LoRa.print(packet);
    LoRa.endPacket();

    packetID += 1;
  }
}
