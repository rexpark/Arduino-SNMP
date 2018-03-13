#include <Ethernet.h>
#include "SNMPAgent.h"
#include "global.h"
#include <String.h>

//SNMPAgent with debug enabled
SNMPAgent snmp_agent = SNMPAgent(true);

bool send_trap = true;

void setup() {
  // Start serial port
  Serial.begin(9600);
  Serial.println("\n\n\n\nArduino Running...");

  // Start Ethernet
  Ethernet.begin(mac, ip);// these are defined in global.cpp

  // Start SNMP Agent
  snmp_agent.setup();
}


void loop() {
  char StringToSendToSNMP[80];

  // listen/handle for incoming SNMP requests
  snmp_agent.update();
  

  // send a basic trap with just a text string.
  if (send_trap) {
   String TrapMessage = "Whatever data you want to send.";

    int i = TrapMessage.length();
    
    TrapMessage.toCharArray(StringToSendToSNMP, i+1);
    StringToSendToSNMP[i+1] = NULL;

    //Send the trap message
    snmp_agent.send_inform(NOTIFICATIONS_MAJOR_OID, StringToSendToSNMP);

    send_trap = false;
  }
}
