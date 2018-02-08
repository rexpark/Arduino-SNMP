/**
 * Example Agent implementation using the ArduinoSNMP library.
 * Copyright (C) 2013 Rex Park <rex.park@me.com>
 * All rights reserved.
 *
 */
#include "SNMPAgent.h"
#include "global.h"

SNMPAgent::SNMPAgent(boolean _debug): _send_tag_data(false){
  debug_enabled = true;
}

void SNMPAgent::setup(){
  //read in community strings and manager ip from EEPROM

  _oid_del = ".";
  snmp_inform_timeout = 60*SNMPTimeout;

  _api_status = SNMP.begin(snmp_read_community.c_str(),snmp_read_write_community.c_str(),snmp_trap_community.c_str(),SNMP_DEFAULT_PORT);

  if(_api_status == SNMP_API_STAT_SUCCESS){
    Serial.println("SNMP Agent Started");
    delay(10);

  }else{
    Serial.println("Error starting SNMP Library");
  }
  delay(10);
}

void SNMPAgent::update(){

  process_inform_table();

  if(SNMP.listen() == true){
    process_snmp_pdu();
  }
}

/**
 * SNMP PDU Handler
 */
void SNMPAgent::process_snmp_pdu(){
  boolean success = false;
  boolean reply_necessary = true;

  _api_status = SNMP.requestPdu(&_pdu,NULL,0);

  Serial.println(SNMP_PDU_RECEIVED);

  //PDU Could Not Be Processed
  if(_api_status != SNMP_API_STAT_SUCCESS || _pdu.error != SNMP_ERR_NO_ERROR){
    Serial.print("API Status: ");
    Serial.println(_api_status,true);
    Serial.print("Error: ");
    Serial.println(_pdu.error,true);
    Serial.println(_pdu.requestId);
  }//Process PDU
  else{
    memset(_oid, '\0', SNMP_MAX_OID_LEN);
    _pdu.value.OID.toString(_oid,SNMP_MAX_OID_LEN);
    Serial.print("OID: ");
    Serial.println(_oid);

    //Process inform responses
    if(_pdu.type == SNMP_PDU_RESPONSE){
      process_inform_response();
      reply_necessary = false;
    }//process get/set
    else if(_pdu.type == SNMP_PDU_GET || _pdu.type == SNMP_PDU_SET){

      //Route the message to the correct method for further processing.
      if(strncmp(_oid, CONFIG_OID, 21) == 0){
        success = process_config_command();
      }
      else if(strncmp(_oid, MIB2_SYS_OID, 9) == 0){
        success = process_mib2_command();
      }

      if(success == false){
        Serial.println("SNMP Error: OID Not Found");
        _pdu.error = SNMP_ERR_NO_SUCH_NAME;
      }
    }else{
      Serial.println("SNMP: Invalid Type");
      _pdu.error = SNMP_ERR_GEN_ERROR;
    }
  }

  //Send the response.
  if(reply_necessary == true && !(_api_status == SNMP_API_STAT_NO_SUCH_NAME || _api_status == SNMP_API_STAT_PACKET_INVALID)){
    //send PDU response
    clear_buffer(big_buffer,BIG_BUFFER_SIZE);

    _pdu.type = SNMP_PDU_RESPONSE;

    if(_pdu.error != SNMP_ERR_NO_ERROR){
      _pdu.value.encode(SNMP_SYNTAX_NULL);
    }

    SNMP.send_message(&_pdu, SNMP.remoteIP(), SNMP.remotePort(),(byte*)big_buffer);
  }

  //clear _pdu
  SNMP.freePdu(&_pdu);
}

/**
 * Process CONFIG OIDs
 */
boolean SNMPAgent::process_config_command(){
  //Network Config Branch
  if(strncmp(_oid,CONFIG_NETWORK_OID,23) == 0){

    if(strcmp_P(_oid,CONFIG_NETWORK_IP_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            ip[0] = copy_buffer[0];
            ip[1] = copy_buffer[1];
            ip[2] = copy_buffer[2];
            ip[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, ip);
      _pdu.error = _status;
      return true;
    }
    else if(strcmp_P(_oid,CONFIG_NETWORK_GATEWAY_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            gateway[0] = copy_buffer[0];
            gateway[1] = copy_buffer[1];
            gateway[2] = copy_buffer[2];
            gateway[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, gateway);
      _pdu.error = _status;
      return true;
    }
    else if(strcmp_P(_oid,CONFIG_NETWORK_DNS_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            DNS[0] = copy_buffer[0];
            DNS[1] = copy_buffer[1];
            DNS[2] = copy_buffer[2];
            DNS[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, DNS);
      _pdu.error = _status;
      return true;
    }
    else if(strcmp_P(_oid,CONFIG_NETWORK_SUBNET_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            subnet[0] = copy_buffer[0];
            subnet[1] = copy_buffer[1];
            subnet[2] = copy_buffer[2];
            subnet[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, subnet);
      _pdu.error = _status;
      return true;
    }
  }
  //SNMP CONFIG Branch
  else if(strncmp(_oid,CONFIG_SNMP_OID,23) == 0){

    //Read Only Community String
    if(strcmp_P(_oid,CONFIG_SNMP_READ_STRING_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode(copy_buffer,SNMP_MAX_COMMUNITY_SIZE);

        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < SNMP_MAX_COMMUNITY_SIZE){
            snmp_read_community = String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, snmp_read_community.c_str());
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
            snmp_read_write_community= String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, snmp_read_write_community.c_str());
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
            snmp_trap_community= String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, snmp_trap_community.c_str());
      _pdu.error = _status;
      return true;
    }
    //SNMP Manager IP Address 1
    else if(strcmp_P(_oid,CONFIG_SNMP_MANAGER_1_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            SNMPIP1[0] = copy_buffer[0];
            SNMPIP1[1] = copy_buffer[1];
            SNMPIP1[2] = copy_buffer[2];
            SNMPIP1[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, SNMPIP1);
      _pdu.error = _status;
      return true;
    }
    //SNMP Manager IP Address 2
    else if(strcmp_P(_oid,CONFIG_SNMP_MANAGER_2_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            SNMPIP2[0] = copy_buffer[0];
            SNMPIP2[1] = copy_buffer[1];
            SNMPIP2[2] = copy_buffer[2];
            SNMPIP2[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, SNMPIP2);
      _pdu.error = _status;
      return true;
    }
    else if(strcmp_P(_oid,CONFIG_SNMP_INFORM_ENABLED_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        _status = _pdu.value.decode(&temp_int);

        if(_status == SNMP_ERR_NO_ERROR){
          if(temp_int == 0 || temp_int == 1){
            SNMPInforms = temp_int;
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_INT32, (int16_t)SNMPInforms);
      _pdu.error = _status;
      return true;
    }
    else if(strcmp_P(_oid,CONFIG_SNMP_INFORM_TIMEOUT_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){

        _status = _pdu.value.decode(&temp_int);

        if(_status == SNMP_ERR_NO_ERROR){
          if(temp_int > 0 && temp_int <= 99){
            SNMPTimeout = temp_int;
            snmp_inform_timeout = 60*SNMPTimeout;
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_INT32, (int16_t)SNMPTimeout);
      _pdu.error = _status;
      return true;
    }
  }
  //Site CONFIG Branch
  else if(strncmp(_oid,CONFIG_SITE_OID,23) == 0){

    //Site ID
    if(strcmp_P(_oid,CONFIG_SITE_ID_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode(copy_buffer,31);

        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < 31){
            SiteID = String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, SiteID.c_str());
      _pdu.error = _status;
      return true;
    }
    //City
    else if(strcmp_P(_oid,CONFIG_SITE_CITY_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode(copy_buffer,31);

        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < 31){
            SiteCity = String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, SiteCity.c_str());
      _pdu.error = _status;
      return true;
    }
    //State
    else if(strcmp_P(_oid,CONFIG_SITE_STATE_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode(copy_buffer,3);

        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < 3){
            SiteState = String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, SiteState.c_str());
      _pdu.error = _status;
      return true;
    }
  }
  //NTP CONFIG Branch
  else if(strncmp(_oid,CONFIG_TIME_OID,23) == 0){

    //NTP IP
    if(strcmp_P(_oid,CONFIG_TIME_SERVER_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode((byte*)copy_buffer);

        if(_status == SNMP_ERR_NO_ERROR){
          if((byte)copy_buffer[0] >= 0 && (byte)copy_buffer[1] >= 0 && (byte)copy_buffer[2] >= 0 && (byte)copy_buffer[3] >= 0){
            tsIP[0] = copy_buffer[0];
            tsIP[1] = copy_buffer[1];
            tsIP[2] = copy_buffer[2];
            tsIP[3] = copy_buffer[3];
          }
        }
      }

      _status = _pdu.value.encode_address(SNMP_SYNTAX_IP_ADDRESS, tsIP);
      _pdu.error = _status;
      return true;
    }
    //NTP Enabled
    else if(strcmp_P(_oid,CONFIG_TIME_ENABLE_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        _status = _pdu.value.decode(&temp_int);

        if(_status == SNMP_ERR_NO_ERROR){
          if(temp_int == 0 || temp_int == 1){
            EnableTimeGet = temp_int;
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_INT32, (int16_t)EnableTimeGet);
      _pdu.error = _status;
      return true;
    }
    //Timezone
    else if(strcmp_P(_oid,CONFIG_TIME_ZONE_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){

        _status = _pdu.value.decode(&temp_int);

        if(_status == SNMP_ERR_NO_ERROR){
          if(temp_int >= -10 && temp_int <= 10){
            timeZone = temp_int;
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_INT32, (int16_t)timeZone);
      _pdu.error = _status;
      return true;
    }
    //User Field
    else if(strcmp_P(_oid,CONFIG_USER_OID) == 0){
      if(_pdu.type == SNMP_PDU_SET){
        clear_buffer(copy_buffer,COPY_BUFFER_SIZE);

        _status = _pdu.value.decode(copy_buffer,31);

        if(_status == SNMP_ERR_NO_ERROR){
          if(_pdu.value.size < 31){
            UserField = String(copy_buffer);
          }
        }
      }

      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, UserField.c_str());
      _pdu.error = _status;
      return true;
    }
  }
  //accept changes
  else if(strcmp_P(_oid,CONFIG_ACCEPT_CHANGES_OID) == 0){

    if(_pdu.type == SNMP_PDU_SET){
      _status = _pdu.value.decode(&temp_int);

      if(_status == SNMP_ERR_NO_ERROR){
        if(temp_int == 0 || temp_int == 1){
          accept_changes = temp_int;
        }
      }
    }

    _status = _pdu.value.encode(SNMP_SYNTAX_INT32, (int16_t)accept_changes);
    _pdu.error = _status;
    return true;
  }

  return false;
}

/**
 * Process MIB2 OIDs
 */
boolean SNMPAgent::process_mib2_command(){
  if(strcmp(_oid,MIB2_SYS_DESC) == 0){

    if(_pdu.type == SNMP_PDU_SET){
      _pdu.error = SNMP_ERR_READ_ONLY;
    }else{
      _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, SYS_DESCRIPTION);
      _pdu.error = _status;
    }

    return true;
  }

  return false;
}

void SNMPAgent::process_inform_table(){
  for(byte i = 0; i < inform_holding_table.size(); i++){

    tmp_entry = inform_holding_table.get(i);

    if(now() - tmp_entry->last_sent > snmp_inform_timeout){//5 minutes, hard coded for now
      Serial.print("Resending inform ");
      Serial.print(tmp_entry->request_id);
      Serial.println("...");
      if(SNMPIP1[0] != 0){
        SNMP.send_message(SNMPIP1, SNMP_MANAGER_PORT, tmp_entry->snmp_packet, tmp_entry->packet_length);

        if(SNMPIP2[0] != 0){
          SNMP.send_message(SNMPIP2, SNMP_MANAGER_PORT, tmp_entry->snmp_packet, tmp_entry->packet_length);
        }
      }
      tmp_entry->last_sent = now();
      delay(1000);
    }
  }
}

/**
 * Process Inform Response
 *    Removes entry from inform_holding_table
 */
boolean SNMPAgent::process_inform_response(){
  for(byte i = 0; i < inform_holding_table.size(); i++){

    tmp_entry = inform_holding_table.get(i);

    if(tmp_entry->request_id == _pdu.requestId){
      inform_holding_table.remove(i);
      break;
    }
  }
}

/**
 * Remove Inform from inform_holding_table
 */
boolean SNMPAgent::remove_inform(uint32_t request_id){

  for(byte i = 0; i < inform_holding_table.size(); i++){

    tmp_entry = inform_holding_table.get(i);

    if(tmp_entry->request_id == request_id){
      inform_holding_table.remove(i);
      return true;
    }
  }

  return false;
}

/**
 * Send SNMP Inform
 */
uint32_t SNMPAgent::send_inform(const char *oid, const char *data){
  _pdu.clear();
  _pdu.value.OID.fromString(oid);//trap oid

  //Pass it a value struct so it can use it for processing. Saves on overhead
  _pdu.prepare_inform(&_value);

  _value.OID.fromString(NOTIFICATIONS_OBJECT_OID);//Notification Object
  _value.encode(SNMP_SYNTAX_OCTETS, data);
  _pdu.add_data(&_value);

  //send it
  SNMP_INFORM_TABLE_ENTRY new_entry;

  if(SNMPIP1[0] != 0){
    new_entry.request_id = SNMP.send_message(&_pdu,SNMPIP1,SNMP_MANAGER_PORT);//manager 1

    if(SNMPIP2[0] != 0){
      SNMP.resend_message(SNMPIP2,SNMP_MANAGER_PORT);//manager 2
    }

    Serial.print("Inform ");
    Serial.print(new_entry.request_id);
    Serial.println(" sent...");
  }

  new_entry.packet_length = SNMP.copy_packet(new_entry.snmp_packet);
  new_entry.last_sent = now();
  inform_holding_table.add(new_entry);

  SNMP.clear_packet();
  //clear _pdu
  SNMP.freePdu(&_pdu);
}

void SNMPAgent::clear_buffer(char buffer[], byte buffer_size){
  memset(buffer,'\0',buffer_size);
}

void SNMPAgent::clear_buffer(byte buffer[], byte buffer_size){
  memset(buffer,0,buffer_size);
}

void SNMPAgent::copy_message_to_buffer(const char *message, char buffer[], byte length){
  clear_buffer(buffer, length);

  //copy up to length characters
  strncpy(buffer,message,length);
}

void SNMPAgent::concat_message_to_buffer(const char *message, char to_b[], char buffer[], byte length){
  copy_message_to_buffer(message,buffer,length);
  strcat(to_b,buffer);
}

void SNMPAgent::set_next_request_id(uint32_t request_id){
  SNMP.requestCounter = request_id;
}
