/**
 * Example Agent implementation using the ArduinoSNMP library.
 * Copyright (C) 2013 Rex Park <rex.park@me.com>
 * All rights reserved.
 * 
 */
#include "SNMPAgent.h"

/**
 * Read Only Strings
 *
 * Note: .1.3.6.1.4.1.36582 is the Arduino Enterprise OID. 
 *  If you intend to use SNMP in a production environment you should register for your own Enterprise OID.
 */
prog_char SYS_OBJECT_ID[] PROGMEM ={"1.3.6.1.4.1.36582"};
prog_char SYS_DESCRIPTION[] PROGMEM ={"Arduino SNMP Agent Example"};
prog_char SNMP_PDU_RECEIVED[] PROGMEM ={"SNMP PDU Received"};

// RFC1213-MIB OIDs
// .iso.org.dod.internet.mgmt.mib-2.system.sysObjectID (.1.3.6.1.2.1.1.2)
prog_char sysObjectID[] PROGMEM ={"1.3.6.1.2.1.1.2.0"};  // read-only  (ObjectIdentifier)
// .iso.org.dod.internet.mgmt.mib-2.system.sysUpTime (.1.3.6.1.2.1.1.3)
prog_char sysUpTime[] PROGMEM = {"1.3.6.1.2.1.1.3.0"};  // read-only  (TimeTicks)

// SNMPv2-MIB
prog_char MIB2_SYS_OID[]                           PROGMEM ={"1.3.6.1.2.1.1"};
prog_char MIB2_SYS_DESC[]                          PROGMEM ={"1.3.6.1.2.1.1.1.0"};
prog_char MIB2_WARM_START_TRAP_OID[]               PROGMEM ={"1.3.6.1.6.3.1.1.5.2"};

// Example MIB OIDs
prog_char CONFIG_OID[]                         PROGMEM ={"1.3.6.1.4.1.36582.1.5"};
prog_char CONFIG_NETWORK_OID[]                 PROGMEM ={"1.3.6.1.4.1.36582.1.5.1"};
prog_char CONFIG_NETWORK_IP_OID[]              PROGMEM ={"1.3.6.1.4.1.36582.1.5.1.1.0"};
prog_char CONFIG_NETWORK_GATEWAY_OID[]         PROGMEM ={"1.3.6.1.4.1.36582.1.5.1.2.0"};
prog_char CONFIG_NETWORK_DNS_OID[]             PROGMEM ={"1.3.6.1.4.1.36582.1.5.1.3.0"};
prog_char CONFIG_NETWORK_SUBNET_OID[]          PROGMEM ={"1.3.6.1.4.1.36582.1.5.1.4.0"};
prog_char CONFIG_NETWORK_MAC_OID[]             PROGMEM ={"1.3.6.1.4.1.36582.1.5.1.5.0"};
prog_char CONFIG_NETWORK_DHCP_OID[]            PROGMEM ={"1.3.6.1.4.1.36582.1.5.1.6.0"};

prog_char CONFIG_SNMP_OID[]                    PROGMEM ={"1.3.6.1.4.1.36582.1.5.3"};
prog_char CONFIG_SNMP_MANAGER_PORT_OID[]       PROGMEM ={"1.3.6.1.4.1.36582.1.5.3.1.0"};
prog_char CONFIG_SNMP_READ_STRING_OID[]        PROGMEM ={"1.3.6.1.4.1.36582.1.5.3.2.0"};
prog_char CONFIG_SNMP_WRITE_STRING_OID[]       PROGMEM ={"1.3.6.1.4.1.36582.1.5.3.3.0"};
prog_char CONFIG_SNMP_MANAGER_IP_OID[]         PROGMEM ={"1.3.6.1.4.1.36582.1.5.3.4.0"};
prog_char CONFIG_SNMP_TRAP_STRING_OID[]        PROGMEM ={"1.3.6.1.4.1.36582.1.5.3.5.0"};


SNMPAgent::SNMPAgent(boolean debug): _send_tag_data(false){
  _debug = debug;
  DEBUG = Serial;//can be changed to any serial device.
}

void SNMPAgent::setup(){
  if(debug){
    DEBUG.begin(9600);
    debug_P(PSTR("Setup Running"));
  }

  //read in community strings and manager ip from EEPROM

  _oid_del = ".";
  
  _api_status = SNMP.begin(snmp_read_community,snmp_read_write_community,snmp_trap_community,SNMP_DEFAULT_PORT);
  
  if(_api_status == SNMP_API_STAT_SUCCESS){
    debug_P(PSTR("SNMP Agent Started"));
    delay(10);

    warm_start();
  }else{
    debug_P(PSTR("Error starting SNMP Library"));
  }
  delay(10);
}

void SNMPAgent::update(){
  
  if(SNMP.listen() == true){
    process_snmp_pdu();
  }
}

/**
 * SNMP PDU Handler
 */
void SNMPAgent::process_snmp_pdu(){
  boolean success = false;

  _api_status = SNMP.requestPdu(&_pdu,NULL,0);
  
  debug_P(SNMP_PDU_RECEIVED);
  
  //PDU Could Not Be Processed
  if(_api_status != SNMP_API_STAT_SUCCESS || _pdu.error != SNMP_ERR_NO_ERROR){
    debug_P(PSTR("API Status: "));
    debug(_api_status,true);
    debug_P(PSTR("Error: "));
    debug(_pdu.error,true);
  }//Process PDU
  else{
    memset(_oid, '\0', SNMP_MAX_OID_LEN);
    _pdu.value.OID.toString(_oid,SNMP_MAX_OID_LEN);
    debug_P(PSTR("OID: "));
    debug(_oid,true);
    
    //We only process GET and SET
    if(_pdu.type == SNMP_PDU_GET || _pdu.type == SNMP_PDU_SET){
      
      //Route the message to the correct method for further processing.
      if(strncmp_P(_oid, CONFIG_OID, 21) == 0){
        success = process_controller_command();
      }
      else if(strncmp_P(_oid, MIB2_SYS_OID, 9) == 0){
        success = process_mib2_command();
      }
      
      if(success == false){
        debug_P(PSTR("SNMP Error: OID Not Found"),true);
        _pdu.error = SNMP_ERR_NO_SUCH_NAME;
      }
    }else{
      debug_P(PSTR("SNMP: Invalid Type"));
      _pdu.error = SNMP_ERR_GEN_ERROR;
    }
  }
  
  //Send the response.
  if(!(_api_status == SNMP_API_STAT_NO_SUCH_NAME || _api_status == SNMP_API_STAT_PACKET_INVALID)){
    //send PDU response
    clear_buffer(big_buffer,BIG_BUFFER_SIZE);
    
    _pdu.type = SNMP_PDU_RESPONSE;
    
    if(_pdu.error != SNMP_ERR_NO_ERROR){
      _pdu.value.encode(SNMP_SYNTAX_NULL);
    }
    
    if(_send_tag_data == true){
      #ifdef UTRACK
      SNMP.responsePdu(&_pdu, SNMP.remoteIP(), SNMP.remotePort(),(byte*)big_buffer, tag_data_buffer);
      #endif
      _send_tag_data = false;
    }
    else{
      SNMP.responsePdu(&_pdu, SNMP.remoteIP(), SNMP.remotePort(),(byte*)big_buffer);
    }
  }
  
  //clear _pdu
  SNMP.freePdu(&_pdu);
}

/**
 * Process MIB2 OIDs
 */
boolean SNMPAgent::process_mib2_command(){  
  if(strcmp_P(_oid,MIB2_SYS_DESC) == 0){
    if(_pdu.type == SNMP_PDU_SET){
      _pdu.error = SNMP_ERR_READ_ONLY;
    }else{
      copy_message_to_buffer(SYS_DESCRIPTION,big_buffer,BIG_BUFFER_SIZE);
      
      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, big_buffer);
      _pdu.error = _status;
    }
    
    return true;
  }
  
  return false;
}


boolean SNMPAgent::process_controller_command(){
  
  //SNMP CONFIG Branch
  if(strncmp_P(_oid,CONFIG_SNMP_OID,23) == 0){
    
    //Outbound port for connecting to SNMP Managers
    if(strcmp_P(_oid,CONFIG_SNMP_MANAGER_PORT_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        
        _status = _pdu.value.decode(&temp_uint);
        
        if(_status == SNMP_ERR_NO_ERROR){
          if(temp_uint > 0 && temp_uint <= MAX_PORT_VALUE){
            snmp_port = temp_uint;
            auto_save = true;
          }
        }
      }
      
      _status = _pdu.value.encode(SNMP_SYNTAX_UINT32, snmp_port);
      _pdu.error = _status; 
      return true;
    }
    //Read Only Community String
    else if(strcmp_P(_oid,CONFIG_SNMP_READ_STRING_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);
        
        _status = _pdu.value.decode(copy_buffer,SNMP_MAX_COMMUNITY_SIZE);
        
        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < SNMP_MAX_COMMUNITY_SIZE){
            clear_buffer(snmp_read_community, SNMP_MAX_COMMUNITY_SIZE);
            strncpy(snmp_read_community,copy_buffer,_pdu.value.size);
            auto_save = true;
          }
        }
      }
      
      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, snmp_read_community);
      _pdu.error = _status; 
      return true;  
    }
    //Read Write Community String
    else if(strcmp_P(_oid,CONFIG_SNMP_WRITE_STRING_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);
        
        _status = _pdu.value.decode(copy_buffer,SNMP_MAX_COMMUNITY_SIZE);
        
        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < SNMP_MAX_COMMUNITY_SIZE){
            clear_buffer(snmp_read_write_community, SNMP_MAX_COMMUNITY_SIZE);
            strncpy(snmp_read_write_community,copy_buffer,_pdu.value.size);
            auto_save = true;
          }
        }
      }
      
      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, snmp_read_write_community);
      _pdu.error = _status; 
      return true;  
    }
    //SNMP Manager IP Address
    else if(strcmp_P(_oid,CONFIG_SNMP_MANAGER_IP_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(uri_buffer,URI_BUFFER_SIZE);
        
        _status = _pdu.value.decode((byte*)uri_buffer);
        
        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)uri_buffer[0] > 0 && (byte)uri_buffer[1] > 0 && (byte)uri_buffer[2] > 0 && (byte)uri_buffer[3] > 0){
            snmp_address[0] = uri_buffer[0];
            snmp_address[1] = uri_buffer[1];
            snmp_address[2] = uri_buffer[2];
            snmp_address[3] = uri_buffer[3];
            auto_save = true;
          }
        }
      }
      
      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, snmp_address);
      _pdu.error = _status; 
      return true;   
    }
    //Trap Community String
    else if(strcmp_P(_oid,CONFIG_SNMP_TRAP_STRING_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);
        
        _status = _pdu.value.decode(copy_buffer,SNMP_MAX_COMMUNITY_SIZE);
        
        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < SNMP_MAX_COMMUNITY_SIZE){
            clear_buffer(snmp_trap_community, SNMP_MAX_COMMUNITY_SIZE);
            strncpy(snmp_trap_community,copy_buffer,_pdu.value.size);
            auto_save = true;
          }
        }
      }
      
      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, snmp_trap_community);
      _pdu.error = _status; 
      return true;  
    }
  }

  return false;
}


/**
 * A warmStart trap signifies that the SNMP entity,
 * supporting a notification originator application,
 * is reinitializing itself such that its configuration
 * is unaltered.
 */
void SNMPAgent::warm_start(){
  g_pdu.clear();

  g_pdu.value.OID.fromString_P(MIB2_WARM_START_TRAP_OID);

  //encode data and append it to PDUs value
  g_pdu.value.size = 0;
  
  g_pdu.prepare_trapv2(&g_value);

  //send it
  SNMP.responsePdu(&g_pdu,broadcast,snmp_port);
}

/**
 * Prints string from Prog Mem to DEBUG.
 */
void debug_P(prog_char *message, boolean newline){
  if(debug_enabled){
    char buff[URI_BUFFER_SIZE];
    copy_message_to_buffer(message,buff,URI_BUFFER_SIZE);
    
    DEBUG.print(buff);
    
    if(newline == true){
      DEBUG.println();
    }
  }
}