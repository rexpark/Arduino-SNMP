#include "global.h"

//System Global Setup vars
//These should be saved/loaded from EEPROM or an SD Card.
IPAddress ip(192, 168, 1, 2);   //local ip
IPAddress gateway(0, 0, 0, 0);   //gateway
IPAddress subnet(0, 0, 0, 0);    //subnet mask
IPAddress DNS(0, 0, 0, 0);       //DNS server ip address
IPAddress SNMPIP1(192, 168, 1, 5);   //Remote NMS for SNMP Informs #1
IPAddress SNMPIP2(192, 168, 1, 6);   //Remote NMS for SNMP Informs #2
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
String SiteID = "999999";          //Site ID, value read from SD card
String SiteCity = "Springfield";      // Site City
String SiteState = "FX";           // Site State
int SNMPInforms = 1;  // SNMPInforms enabled by default, set to 0 to turn off
String snmp_read_community = "public";
String snmp_read_write_community = "private";
String snmp_trap_community = "private";
IPAddress tsIP(192,168,1,1);  // Time Server ip address
String UserField = "User-Data";  // User Field
int EnableTimeGet = 1; //Setting to allow retrieval of current time and date from the time server
int SNMPTimeout = 5;  // 5 minute default
String Password = "1234ABCD";
int timeZone = -6; //Central Standard Time
boolean accept_changes = false;

LinkedList<SNMP_INFORM_TABLE_ENTRY> inform_holding_table = LinkedList<SNMP_INFORM_TABLE_ENTRY>();

/**
 * Read Only Strings
 */
const char *SYS_DESCRIPTION = {"Control Box"};
const char *SNMP_PDU_RECEIVED ={"SNMP PDU Received"};

// SNMPv2-MIB
const char *MIB2_SYS_OID                        ={"1.3.6.1.2.1.1"};
const char *MIB2_SYS_DESC                       ={"1.3.6.1.2.1.1.1.0"};
const char *MIB2_SYS_OBJ                        ={"1.3.6.1.2.1.1.2.0"};
const char *MIB2_SYS_UPTIME                     ={"1.3.6.1.2.1.1.3.0"};
const char *MIB2_WARM_START_TRAP_OID            ={"1.3.6.1.6.3.1.1.5.2"};

// Your OIDs, 12345 = your organizations private Enterprise OID from IANA.
const char *SYS_OBJECT_ID                       ={"1.3.6.1.4.1.12345"};

// Config
const char *CONFIG_OID                          ={"1.3.6.1.4.1.12345.1.1"};
const char *CONFIG_ACCEPT_CHANGES_OID		        ={"1.3.6.1.4.1.12345.1.1.5.0"};

// Config - Network
const char *CONFIG_NETWORK_OID                  ={"1.3.6.1.4.1.12345.1.1.1"};
const char *CONFIG_NETWORK_IP_OID               ={"1.3.6.1.4.1.12345.1.1.1.1.0"};
const char *CONFIG_NETWORK_GATEWAY_OID          ={"1.3.6.1.4.1.12345.1.1.1.2.0"};
const char *CONFIG_NETWORK_SUBNET_OID           ={"1.3.6.1.4.1.12345.1.1.1.3.0"};
const char *CONFIG_NETWORK_DNS_OID              ={"1.3.6.1.4.1.12345.1.1.1.4.0"};

// Config - SNMP
const char *CONFIG_SNMP_OID                     ={"1.3.6.1.4.1.12345.1.1.2"};
const char *CONFIG_SNMP_MANAGER_1_OID           ={"1.3.6.1.4.1.12345.1.1.2.1.0"};
const char *CONFIG_SNMP_MANAGER_2_OID           ={"1.3.6.1.4.1.12345.1.1.2.2.0"};
const char *CONFIG_SNMP_READ_STRING_OID         ={"1.3.6.1.4.1.12345.1.1.2.3.0"};
const char *CONFIG_SNMP_WRITE_STRING_OID        ={"1.3.6.1.4.1.12345.1.1.2.4.0"};
const char *CONFIG_SNMP_TRAP_STRING_OID         ={"1.3.6.1.4.1.12345.1.1.2.5.0"};
const char *CONFIG_SNMP_INFORM_ENABLED_OID      ={"1.3.6.1.4.1.12345.1.1.2.6.0"};
const char *CONFIG_SNMP_INFORM_TIMEOUT_OID      ={"1.3.6.1.4.1.12345.1.1.2.7.0"};

// Config - Site
const char *CONFIG_SITE_OID                     ={"1.3.6.1.4.1.12345.1.1.3"};
const char *CONFIG_SITE_ID_OID                  ={"1.3.6.1.4.1.12345.1.1.3.1.0"};
const char *CONFIG_SITE_CITY_OID                ={"1.3.6.1.4.1.12345.1.1.3.2.0"};
const char *CONFIG_SITE_STATE_OID               ={"1.3.6.1.4.1.12345.1.1.3.3.0"};

// Config - Time
const char *CONFIG_TIME_OID                     ={"1.3.6.1.4.1.12345.1.1.4"};
const char *CONFIG_TIME_SERVER_OID              ={"1.3.6.1.4.1.12345.1.1.4.1.0"};
const char *CONFIG_TIME_ENABLE_OID              ={"1.3.6.1.4.1.12345.1.1.4.2.0"};
const char *CONFIG_TIME_ZONE_OID                ={"1.3.6.1.4.1.12345.1.1.4.3.0"};
const char *CONFIG_USER_OID           		      ={"1.3.6.1.4.1.12345.1.1.4.4.0"};

// Notifcations
const char *NOTIFICATIONS_OID                   ={"1.3.6.1.4.1.12345.1.2"};
const char *NOTIFICATIONS_OBJECT_OID            ={"1.3.6.1.4.1.12345.1.2.2"};

// Notifcations - Informs
const char *NOTIFICATIONS_MAJOR_OID             ={"1.3.6.1.4.1.12345.1.2.1.1"};
const char *NOTIFICATIONS_MINOR_OID             ={"1.3.6.1.4.1.12345.1.2.1.2"};
const char *NOTIFICATIONS_CRITICAL_OID          ={"1.3.6.1.4.1.12345.1.2.1.3"};
const char *NOTIFICATIONS_INFORMATIONAL_OID     ={"1.3.6.1.4.1.12345.1.2.1.4"};
const char *NOTIFICATIONS_RECOVERY_OID          ={"1.3.6.1.4.1.12345.1.2.1.5"};
