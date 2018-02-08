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
#include <Ethernet.h>
#include "ArduinoSNMP.h"
#include "Time.h"
#include "global.h"

#define SNMP_MAX_COMMUNITY_SIZE SNMP_MAX_NAME_LEN
#define COPY_BUFFER_SIZE 41
#define BIG_BUFFER_SIZE 80

class SNMPAgent {
  private:
    SNMP_API_STAT_CODES _api_status;
    SNMP_ERR_CODES _status;
    SNMP_PDU _pdu;
    SNMP_VALUE _value;
    char _oid[SNMP_MAX_OID_LEN];
    boolean _send_tag_data;
    char *_oid_del;
    unsigned long snmp_inform_timeout;

    SNMP_INFORM_TABLE_ENTRY *tmp_entry;

    int _factor;
    uint16_t temp_uint;
    int16_t temp_int;
    char copy_buffer[COPY_BUFFER_SIZE];
    char big_buffer[BIG_BUFFER_SIZE];

    boolean debug_enabled;

    void process_snmp_pdu();
    boolean process_config_command();
    boolean process_mib2_command();

    void process_inform_table();
    boolean process_inform_response();
    void load_inform_table();

    void clear_buffer(char buffer[], byte buffer_size);
    void clear_buffer(byte buffer[], byte buffer_size);
    void copy_message_to_buffer(const char *message, char buffer[], byte length);
    void concat_message_to_buffer(const char *message, char to_b[], char buffer[], byte length);

  public:
    SNMPAgent(boolean debug);
    void setup();
    void update();
    boolean remove_inform(uint32_t request_id);
    uint32_t send_inform(const char *oid, const char *data);
    void set_next_request_id(uint32_t request_id);
};
#endif
