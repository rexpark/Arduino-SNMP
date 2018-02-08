ArduinoSNMP v2.2
=============

An updated and enhanced version of the Agentuino (http://code.google.com/p/agentuino/) SNMP library.

Encoding updates referenced from Ruby-SNMP (https://github.com/hallidave/ruby-snmp).

User Guide:
---------------------

February 7 2018 Update: I've added the latest version of this library as well as a real world agent implementation (Example/Actual_SNMP_Agent). I'll try to answer any questions posted to the issues list. It's been a while since I've needed to work on this library so I'm not sure how accurate the information below is.

Working Agent example coming soon.

Setup:
`SNMP.begin(snmp_read_community,snmp_read_write_community,snmp_trap_community,SNMP_DEFAULT_PORT)`

Receiving an SNMP Message, Processing it, Responding:
```
SNMP_API_STAT_CODES _api_status;
SNMP_ERR_CODES _status;
SNMP_PDU _pdu;
char _oid[SNMP_MAX_OID_LEN];

//SNMP.listen() should be called often. Typically from Arduino's loop().
if(SNMP.listen() == true){
  _api_status = SNMP.requestPdu(&_pdu,NULL,0);

  if(_api_status != SNMP_API_STAT_SUCCESS || _pdu.error != SNMP_ERR_NO_ERROR){
    //Message can not be processed.
  }
  else{
    memset(_oid, '\0', SNMP_MAX_OID_LEN);
    _pdu.value.OID.toString(_oid,SNMP_MAX_OID_LEN);
    //We only process GET and SET
    if(_pdu.type == SNMP_PDU_GET || _pdu.type == SNMP_PDU_SET){
       //do something based on OID of the message
        if(strcmp_P(_oid,MIB2_SYS_DESC) == 0){
            if(_pdu.type == SNMP_PDU_SET){
                _pdu.error = SNMP_ERR_READ_ONLY;
            }else{
                copy_message_to_buffer(SYS_DESCRIPTION,big_buffer,BIG_BUFFER_SIZE);
                _status = _pdu.value.encode(SNMP_SYNTAX_OCTETS, big_buffer);
                _pdu.error = _status;
            }
        }
        else{
            //OID NOT FOUND
            _pdu.error = SNMP_ERR_NO_SUCH_NAME;
        }
    }
    else{
      //invalid message type
      _pdu.error = SNMP_ERR_GEN_ERROR;
    }
  }

  //Send the response.
  if(!(_api_status == SNMP_API_STAT_NO_SUCH_NAME || _api_status == SNMP_API_STAT_PACKET_INVALID)){

    //temp_buffer = byte array that can be used to hold data for processing.
    //Cuts down on the amount of memory required exclusively for ArduinoSNMP.
    byte temp_buffer[100];

    _pdu.type = SNMP_PDU_RESPONSE;

    if(_pdu.error != SNMP_ERR_NO_ERROR){
      _pdu.value.encode(SNMP_SYNTAX_NULL);
    }

    SNMP.responsePdu(&_pdu, SNMP.remoteIP(), SNMP.remotePort(),temp_buffer);
  }

  //clear _pdu
  SNMP.freePdu(&_pdu);
}
```

Sending a Trap:
```
SNMP_PDU _pdu;
SNMP_VALUE _value;

_pdu.clear();
_pdu.value.OID.fromString_P(your_trap_oid);//trap oid

//encode data and append it to PDUs value
_pdu.value.size = 0;
_pdu.prepare_trapv2(&_value);

//Value 1
_value.OID.fromString_P(PSTR("YOUR_DATA_OID_HERE"));//OID of the value type being sent
_value.encode(SNMP_SYNTAX_INT, 10);//Sending an integer value of 10
_pdu.value.size = _pdu.add_data(&_value);

//Value 2
_value.OID.fromString_P(PSTR("YOUR_DATA_OID_HERE"));//OID of the value type being sent
_value.encode(SNMP_SYNTAX_OCTETS, "Hi There");//Send a character array
_pdu.value.size = g_pdu.add_data(&g_value);

//send it
SNMP.responsePdu(&_pdu,snmp_manager_ip,snmp_manager_port);
```
