/**
 * Example Agent implementation using the ArduinoSNMP library.
 * Copyright (C) 2013 Rex Park <rex.park@me.com>
 * All rights reserved.
 * 
 */
 
#ifndef __SNMP_AGENT_H__
#define __SNMP_AGENT_H__
#include <ctype.h>
#include "Arduino.h"
#include "Module.h"
#include <Ethernet.h>
#include <ArduinoSNMP.h>

template <class T>
void debug (T message, boolean newline = false){
    if(debug_enabled){
    DEBUG.print(message);
    
    if(newline == true){
      DEBUG.println();
    }
  }
}

class SNMPAgent {
  private:
    SNMP_API_STAT_CODES _api_status;
    SNMP_ERR_CODES _status;
    SNMP_PDU _pdu;
    char _oid[SNMP_MAX_OID_LEN];
    boolean _send_tag_data;
    char *_oid_del;
    
    int _factor;

    HardwareSerial DEBUG;
    boolean debug;

    char snmp_read_community[SNMP_MAX_COMMUNITY_SIZE+1];
    char snmp_read_write_community[SNMP_MAX_COMMUNITY_SIZE+1];
    char snmp_trap_community[SNMP_MAX_COMMUNITY_SIZE+1];

    void process_snmp_pdu();
    boolean process_mib2_command();
    boolean process_controller_command();
    
    void warm_start();
    void debug_P(prog_char *message, boolean newline);
  
  public:
    SNMPAgent(boolean debug);
    void setup();
    void update();
};
#endif
