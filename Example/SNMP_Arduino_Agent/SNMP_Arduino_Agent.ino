/**
 * Example Agent implementation using the ArduinoSNMP library.
 * Copyright (C) 2013 Rex Park <rex.park@me.com>
 * All rights reserved.
 * 
 */
 
#include <Ethernet.h>
#include <ArduinoSNMP.h>
 //https://github.com/ivanseidel/LinkedList

static byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
IPAddress ip = IPAddress(192,169,1,2);
IPAddress manager = IPAddress(192,168,1,3);
static byte gateway[] = { 192, 168, 1, 1 };
static byte subnet[] = { 255, 255, 255, 0 };

SNMPAgent snmp_agent;

SNMP_PDU g_pdu;
SNMP_VALUE g_value;

LinkedList<SNMP_INFORM_TABLE_ENTRY> inform_holding_table = LinkedList<SNMP_INFORM_TABLE_ENTRY>();

void setup(){
  Ethernet.begin(mac, ip);
  
  snmp_agent = SNMPAgent();
  snmp_agent.setup();
}

void loop(){
  _snmp_agent.update();
}

send_inform(char *hex){
  g_pdu.clear();
  g_pdu.value.OID.fromString_P(PSTR("1.3.6.1.4.1.36582.1.0.2"));//trap oid

  //Unnecesary?
  //g_pdu.value.size = 0;
  
  //Pass it a value struct so it can use it for processing. Saves on overhead
  g_pdu.prepare_inform(&g_value);
 
  //Unnecessary?
  //g_value.clear();
  
  g_value.OID.fromString_P(PSTR("1.3.6.1.4.1.36582.1.3.1"));//OID of the value type being sent
  g_value.encode(SNMP_SYNTAX_OCTETS, "abcde");
  g_pdu.add_data(&g_value);

  //send it
  SNMP.send_message(&g_pdu,manager,SNMP_MANAGER_PORT);
}

send_trap(char *hex){
  g_pdu.clear();

  g_pdu.value.OID.fromString_P(PSTR("1.3.6.1.4.1.36582.1.0.2"));//trap oid

  //encode data and append it to PDUs value
  g_pdu.value.size = 0;
  
  g_pdu.prepare_trapv2(&g_value);
  
  g_value.OID.fromString_P(PSTR("1.3.6.1.4.1.36582.1.3.1"));//OID of the value type being sent
  g_value.encode(SNMP_SYNTAX_OCTETS, "abcde");
  g_pdu.add_data(&g_value);

  //send it
  SNMP.send_message(&g_pdu,manager,SNMP_MANAGER_PORT);
}