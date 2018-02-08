#ifndef GLOBAL_H // header guards
#define GLOBAL_H

#include <arduino.h>
#include <EthernetUdp.h>
#include "ArduinoSNMP.h"
#include "LinkedList.h"

//System Global Setup vars
//These should be saved/loaded from EEPROM or an SD Card.extern IPAddress ip;   //local ip
extern IPAddress gateway;   //gateway
extern IPAddress subnet;    //subnet mask
extern IPAddress DNS;       //DNS server ip address
extern IPAddress SNMPIP1;   //Remote NMS for SNMP Informs #1
extern IPAddress SNMPIP2;   //Remote NMS for SNMP Informs #2
extern String SiteID;          //Site ID, value read from SD card
extern String SiteCity;      // Site City
extern String SiteState;           // Site State
extern int SNMPInforms;  // SNMPInforms enabled by default, set to 0 to turn off
extern String snmp_read_community;
extern String snmp_read_write_community;
extern String snmp_trap_community;
extern IPAddress tsIP;  // Time Server ip address
extern String UserField;
extern int EnableTimeGet; //Setting to allow retrieval of current time and date from the time server
extern int SNMPTimeout;  // 5 minute default
extern String Password;
extern int timeZone; //Eastern Standard Time
extern boolean accept_changes;

typedef struct SNMP_INFORM_TABLE_ENTRY {
  uint32_t request_id;
  byte snmp_packet[SNMP_MAX_PACKET_LEN];
  uint16_t packet_length;
  time_t last_sent;
};

extern LinkedList<SNMP_INFORM_TABLE_ENTRY> inform_holding_table;

/**
 * Read Only Strings
 */
extern const char *SYS_DESCRIPTION;
extern const char *SNMP_PDU_RECEIVED;

// SNMPv2-MIB
extern const char *MIB2_SYS_OID;
extern const char *MIB2_SYS_DESC;
extern const char *MIB2_SYS_OBJ;
extern const char *MIB2_SYS_UPTIME;
extern const char *MIB2_WARM_START_TRAP_OID;

// Enterprise OIDs
extern const char *SYS_OBJECT_ID;

// Config
extern const char *CONFIG_OID;
extern const char *CONFIG_ACCEPT_CHANGES_OID;

// Config - Network
extern const char *CONFIG_NETWORK_OID;
extern const char *CONFIG_NETWORK_IP_OID;
extern const char *CONFIG_NETWORK_GATEWAY_OID;
extern const char *CONFIG_NETWORK_SUBNET_OID;
extern const char *CONFIG_NETWORK_DNS_OID;

// Config - SNMP
extern const char *CONFIG_SNMP_OID;
extern const char *CONFIG_SNMP_MANAGER_1_OID;
extern const char *CONFIG_SNMP_MANAGER_2_OID;
extern const char *CONFIG_SNMP_READ_STRING_OID;
extern const char *CONFIG_SNMP_WRITE_STRING_OID;
extern const char *CONFIG_SNMP_TRAP_STRING_OID;
extern const char *CONFIG_SNMP_INFORM_ENABLED_OID;
extern const char *CONFIG_SNMP_INFORM_TIMEOUT_OID;

// Config - Site
extern const char *CONFIG_SITE_OID;
extern const char *CONFIG_SITE_ID_OID;
extern const char *CONFIG_SITE_CITY_OID;
extern const char *CONFIG_SITE_STATE_OID;

// Config - Time
extern const char *CONFIG_TIME_OID;
extern const char *CONFIG_TIME_SERVER_OID;
extern const char *CONFIG_TIME_ENABLE_OID;
extern const char *CONFIG_TIME_ZONE_OID;
extern const char *CONFIG_USER_OID;

// Notifcations
extern const char *NOTIFICATIONS_OID;
extern const char *NOTIFICATIONS_OBJECT_OID;

// Notifcations - Informs
extern const char *NOTIFICATIONS_MAJOR_OID;
extern const char *NOTIFICATIONS_MINOR_OID;
extern const char *NOTIFICATIONS_CRITICAL_OID;
extern const char *NOTIFICATIONS_INFORMATIONAL_OID;
extern const char *NOTIFICATIONS_RECOVERY_OID;

#endif
