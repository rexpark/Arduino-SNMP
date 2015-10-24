/*
  ArduinoSNMP.cpp - An Arduino library for a lightweight SNMP Agent.
  Copyright (C) 2013 Rex Park <rex.park@me.com>, Portions (C) 2010 Eric C. Gionet <lavco_eg@hotmail.com>
  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "ArduinoSNMP.h"
#include <EthernetUdp.h>

EthernetUDP Udp;

SNMP_API_STAT_CODES SNMPClass::begin(char *getCommName,
        char *setCommName, char *trapCommName, uint16_t port)
{
  //initialize request counter
    requestCounter = 1;
    _extra_data_size = 0;
    _udp_extra_data_packet = false;
    
  // set community name set/get sizes
  _setSize = strlen(setCommName);
  _getSize = strlen(getCommName);
  _trapSize = strlen(trapCommName);
  //
  // validate get/set community name sizes
  if ( _setSize > SNMP_MAX_NAME_LEN + 1 || _getSize > SNMP_MAX_NAME_LEN + 1 || _trapSize > SNMP_MAX_NAME_LEN + 1) {
    return SNMP_API_STAT_NAME_TOO_BIG;
  }
  //
  // set community names
  _getCommName = getCommName;
  _setCommName = setCommName;
  _trapCommName = trapCommName;
  
  // validate session port number
  if ( port == NULL || port == 0 ) port = SNMP_DEFAULT_PORT;
  //
  // init UDP socket
  Udp.begin(port);

  return SNMP_API_STAT_SUCCESS;
}

boolean SNMPClass::listen(void)
{
  // if bytes are available in receive buffer
  // and pointer to a function (delegate function)
  // isn't null, trigger the function
  
  if(Udp.parsePacket() > 1024){
    _udp_extra_data_packet = true;
  }else{
    _udp_extra_data_packet = false;
  }
  
  if (Udp.available()){
    if(_callback != NULL){
      (*_callback)();
    }else{
      return true;
    }
  }
  
  return false;
}

/**
 * Extra data: Can be used for storing PDU data directly into another buffer. 
 *  Useful if you have a large object that needs to be worked on elsewhere.
 */
SNMP_API_STAT_CODES SNMPClass::requestPdu(SNMP_PDU *pdu, char *extra_data, int extra_data_max_size)
{
  // sequence length
  uint16_t seqLen;
  // version
  byte verLen, verEnd;
  // community string
  byte comLen, comEnd;
  // pdu
  byte pduTyp, pduLen, pduEnd;
  byte ridLen, ridEnd;
  byte errLen, errEnd;
  byte eriLen, eriEnd;
  byte vblTyp, vblLen;
  byte vbiTyp, vbiLen;
  byte obiLen, obiEnd;
  byte valTyp, valLen, valEnd;
  int i;

  // set packet packet size (skip UDP header)
  _packetSize = Udp.available();
  _packetPos = 0;

  // reset packet array
  memset(_packet, 0, SNMP_MAX_PACKET_LEN);
  //
  // validate packet
  if ( _packetSize <= 0) {
    return SNMP_API_STAT_PACKET_TOO_BIG;
  }
  
  // get UDP packet
  if(_udp_extra_data_packet == true){

    if(extra_data != NULL){
      memset(extra_data, 0, extra_data_max_size);
    }
    
    Udp.read(_packet, _packetSize - extra_data_max_size);

    if(extra_data != NULL){
      Udp.read((byte*)extra_data, extra_data_max_size);
    }
  }else{
    Udp.read(_packet, _packetSize);
  }
  
//  Serial.println("Incomming: ");
//  for(int i = 0; i < _packetSize; i++){
//    Serial.print(_packet[i],HEX);
//    Serial.print(" ");
//  }
//  Serial.println();

  // packet check 1
  if ( _packet[0] != 0x30 ) {
    return SNMP_API_STAT_PACKET_INVALID;
  }
  

  // sequence length
  if(_packet[1] > 0x80){
    seqLen = combine_msb_lsb(_packet[2], _packet[3]);
    _packetPos = 4;
  }else{
    seqLen = _packet[1];
    _packetPos = 2;
  }
  
  // version
  if(_packet[_packetPos] != 0x02){
    return SNMP_API_STAT_PACKET_INVALID;
  }
  
  verLen = _packet[_packetPos+1];//used to be hard coded as index 3
  verEnd = _packetPos+1 + verLen;
  
  // community string
  comLen = _packet[verEnd + 2];
  comEnd = verEnd + 2 + comLen;
  
  // pdu
  pduTyp = _packet[comEnd + 1];
  if(_packet[comEnd + 2] > 0x80){
    pduLen = combine_msb_lsb(_packet[comEnd +3], _packet[comEnd +4]);
    pduEnd = comEnd + 2 + pduLen + 2;
    _packetPos = comEnd + 2 + 2;
  }else{
    pduLen = _packet[comEnd + 2];
    pduEnd = comEnd + 2 + pduLen;
    _packetPos = comEnd + 2;
  }
  
  //request id
  ridLen = _packet[_packetPos + 2];
  ridEnd = _packetPos + 2 + ridLen;
  
  //error
  errLen = _packet[ridEnd + 2];
  errEnd = ridEnd + 2 + errLen;
  
  //error index
  eriLen = _packet[errEnd + 2];
  eriEnd = errEnd + 2 + eriLen;
  
  //variable bindings
  vblTyp = _packet[eriEnd + 1];
  if(_packet[eriEnd + 2] > 0x80){
    vblLen = combine_msb_lsb(_packet[eriEnd +3], _packet[eriEnd +4]);
    _packetPos = eriEnd + 2 + 2;
  }else{
    vblLen = _packet[eriEnd + 2];
    _packetPos = eriEnd + 2;
  }
  
  //variable bindings id
  vbiTyp = _packet[_packetPos + 1];
  if(_packet[_packetPos + 2] > 0x80){
    vbiLen = combine_msb_lsb(_packet[_packetPos +3], _packet[_packetPos +4]);
    _packetPos = _packetPos + 2 + 2;
  }else{
    vbiLen = _packet[_packetPos + 2];
    _packetPos = _packetPos + 2;
  }
  
  //object identifier
  obiLen = _packet[_packetPos + 2];
  obiEnd = _packetPos + 2 + obiLen;
  
  //unknown
  valTyp = _packet[obiEnd + 1];
  
  if(_packet[obiEnd + 2] > 0x80){
    valLen = combine_msb_lsb(_packet[obiEnd +3], _packet[obiEnd +4]);
    valEnd = obiEnd + 2 + valLen + 2;
  }else{
    valLen = _packet[obiEnd + 2];
    valEnd = obiEnd + 2 + valLen;
  }

  // extract version
  pdu->version = _packet[verEnd];
  // validate version
  if(pdu->version != 0x0 && pdu->version != 0x1){
    return SNMP_API_STAT_PACKET_INVALID;
  }

  // pdu-type
  pdu->type = (SNMP_PDU_TYPES)pduTyp;
  _dstType = pdu->type;
  
  // validate community size
  if ( comLen > SNMP_MAX_NAME_LEN ) {
    // set pdu error
    pdu->error = SNMP_ERR_TOO_BIG;
    //
    return SNMP_API_STAT_NAME_TOO_BIG;
  }

  // validate community name
  if ( pdu->type == SNMP_PDU_SET && comLen == _setSize ) {
	if(memcmp(_setCommName,_packet+verEnd+3,_setSize) != 0){
		pdu->error = SNMP_ERR_NO_SUCH_NAME;
		return SNMP_API_STAT_NO_SUCH_NAME;
	}
  } else if ( pdu->type == SNMP_PDU_GET) {
	if(memcmp(_getCommName,_packet+verEnd+3,comLen) != 0 && memcmp(_setCommName,_packet+verEnd+3,comLen) != 0){
		pdu->error = SNMP_ERR_NO_SUCH_NAME;
		return SNMP_API_STAT_NO_SUCH_NAME;
	}
  } else {
    // set pdu error
    pdu->error = SNMP_ERR_NO_SUCH_NAME;
    //
    return SNMP_API_STAT_NO_SUCH_NAME;
  }

  // extract reqiest-id 0x00 0x00 0x00 0x01 (4-byte int aka int32)
  pdu->requestId = 0;
  for ( i = 0; i < ridLen; i++ ) {
    pdu->requestId = (pdu->requestId << 8) | _packet[ridEnd-ridLen+1 + i];
  }

  // extract error 
  pdu->error = SNMP_ERR_NO_ERROR;
  int32_t err = 0;
  for ( i = 0; i < errLen; i++ ) {
    err = (err << 8) | _packet[errEnd-errLen+1 + i];
  }
  pdu->error = (SNMP_ERR_CODES)err;

  // extract error-index 
  pdu->errorIndex = 0;
  for ( i = 0; i < eriLen; i++ ) {
    pdu->errorIndex = (pdu->errorIndex << 8) | _packet[eriEnd-eriLen+1 + i];
  }

  /**
   * OID 
   */
  //validate length
  if ( obiLen > SNMP_MAX_OID_LEN ) {
    pdu->error = SNMP_ERR_TOO_BIG;
    return SNMP_API_STAT_OID_TOO_BIG;
  }
  //decode OID
  memset(pdu->value.OID.data, 0, SNMP_MAX_OID_LEN);
  if(pdu->value.OID.decode(_packet + (obiEnd-obiLen+1), obiLen) != SNMP_API_STAT_SUCCESS){
    return SNMP_API_STAT_MALLOC_ERR;
  }

  /**
   * Value 
   */
  //syntax type
  pdu->value.syntax = (SNMP_SYNTAXES)valTyp;
  //check length of data

  if ( valLen > SNMP_MAX_VALUE_LEN && _udp_extra_data_packet == false) {
    pdu->error = SNMP_ERR_TOO_BIG;
    return SNMP_API_STAT_VALUE_TOO_BIG;
  }
  //set value size
  pdu->value.size = valLen;
  
  if(_udp_extra_data_packet == true){
//    memset(extra_data, '\0', extra_data_max_size);
//    for ( i = 0; i < valLen; i++ ) {
//      extra_data[i] = _packet[obiEnd+3 + i];
//    }    
  }else{
    memset(pdu->value.data, 0, SNMP_MAX_VALUE_LEN);
    for ( i = 0; i < valLen; i++ ) {
      pdu->value.data[i] = _packet[obiEnd+3 + i];
    }    
  }

  return SNMP_API_STAT_SUCCESS;
}

/**
 * Sends a PDU as a v1 trap. (needs testing after v2 trap changes)
 *   OID: The full enterprise OID for the trap you want to send: everything in the trap's OID from the initial .1 
 *     up to the enterprise number, including any subtrees within the enterprise but not the specific trap number.
 *   value: will be an array of pre-encoded data
 *
 * Original Author: Yazgoo
 * Updated: Rex Park, March 29, 2013. (Adjusting for new encoding changes and PDU structure)
 *
 * Broken as of November 10th 2013 due to writeHeaders re-write. Needs to be modified to work like responsePdu
 */
SNMP_API_STAT_CODES SNMPClass::sendTrapv1(SNMP_PDU *pdu, SNMP_TRAP_TYPES trap_type, int16_t specific_trap, IPAddress manager_address){

  byte i;
  SNMP_VALUE value;
  _dstType = pdu->type = SNMP_PDU_TRAP;
  _packetPos = 0;
  uint16_t size = 27 + 2 + pdu->value.size;
  
  this->writeHeaders(pdu);
  
  //unknown
  _packet[_packetPos++] = (byte) size - 1;
  i = _packetPos-1;//i now stores the location of the PDU size, need to update after OID encoding.
  
  //Enterprise OID
  _packetPos += pdu->value.OID.encode(_packet + _packetPos);
  //adjust sizes now that OID is encoded.
  _packetSize += pdu->value.OID.size-2;
  _packet[1] += pdu->value.OID.size-2;
  _packet[i] += pdu->value.OID.size-2;
  
  //Agent IP
  value.encode_address(SNMP_SYNTAX_IP_ADDRESS, pdu->agent_address, _packet + _packetPos);
  _packetPos += value.size;
  
  //trap type
  value.encode(SNMP_SYNTAX_INT, trap_type, _packet + _packetPos);
  _packetPos += value.size;
  
  //specific trap id
  value.encode(SNMP_SYNTAX_INT, specific_trap, _packet + _packetPos);
  _packetPos += value.size;
  
  //Time Ticks
  value.encode(SNMP_SYNTAX_TIME_TICKS, millis()/10, _packet + _packetPos);
  _packetPos +=6;//syntax + length + 4 octets

  //unknown
  _packet[_packetPos++] = (byte) SNMP_SYNTAX_SEQUENCE;
  _packet[_packetPos++] = (byte) 4 + pdu->value.size;

  //trap data, already encoded prior to function call
  for ( i = 0; i < pdu->value.size; i++ ) {
    _packet[_packetPos++] = pdu->value.data[i];
  }

  return writePacket(manager_address, SNMP_MANAGER_PORT);
}

uint16_t SNMPClass::writeHeaders(SNMP_PDU *pdu)
{
  byte i;

  // SNMP community string
  if(_dstType == SNMP_PDU_SET){
    for(i = _setSize-1; i >= 0 && i <= 30; i--){
      _packet[_packetPos--] = (byte)_setCommName[i];
    }
    _packet[_packetPos--] = (byte)_setSize;	// length
  }else if(_dstType == SNMP_PDU_TRAP || _dstType == SNMP_PDU_TRAP2){
    for(i = _trapSize-1; i >= 0 && i <= 30; i--){
      _packet[_packetPos--] = (byte)_trapCommName[i];
    }
    _packet[_packetPos--] = (byte)_trapSize;	// length
  }else {
    for(i = _getSize-1; i >= 0 && i <= 30; i--){
      _packet[_packetPos--] = (byte)_getCommName[i];
    }
    _packet[_packetPos--] = (byte)_getSize;	// length
  }
  _packet[_packetPos--] = (byte)SNMP_SYNTAX_OCTETS;// type
  
  // version
  _packet[_packetPos--] = (byte)pdu->version;// value
  _packet[_packetPos--] = 0x01;			// length
  _packet[_packetPos--] = (byte)SNMP_SYNTAX_INT;//type
  
  //start of header
  //length of all data after this point
  _packet[_packetPos--] = lsb(packet_length()+_extra_data_size);
  _packet[_packetPos--] = msb(packet_length()+_extra_data_size);
  _packet[_packetPos--] = 0x82;//Sending length in two octets

  _packet[_packetPos--] = (byte)SNMP_SYNTAX_SEQUENCE;
  
  //packet type
  if ( _dstType == SNMP_PDU_SET ) {
    _packetSize += _setSize;
  } else if ( _dstType == SNMP_PDU_TRAP ) {
    _packetSize += _trapSize;
  } else {
    _packetSize += _getSize;
  }

  return _packetPos;
}

SNMP_API_STAT_CODES SNMPClass::responsePdu(SNMP_PDU *pdu, IPAddress to_address, uint16_t to_port, byte *temp_buff, char *extra_data)
{
  memset(_packet, 0, SNMP_MAX_PACKET_LEN);
  _packetPos = SNMP_MAX_PACKET_LEN-1;
  int32_u u;
  int t = 0;
  _extra_data_size = 0;
  
  if(extra_data != NULL){
    _extra_data_size = strlen(extra_data);
  }

  // Varbind List
  if(pdu->type == SNMP_PDU_RESPONSE){
    
    t = pdu->add_data(&pdu->value,_packet + _packetPos,true,temp_buff, _extra_data_size);
    _packetPos -= t;
    
    
    //length of entire value being passed
    _packet[_packetPos--] = lsb(t+_extra_data_size);
    _packet[_packetPos--] = msb(t+_extra_data_size);
    _packet[_packetPos--] = 0x82;//Sending length in two octets
    
  }else if(pdu->type == SNMP_PDU_TRAP2){
    //set and increment requestId
    pdu->requestId = requestCounter++;
      
    for (t = pdu->value.size-1; t >= 0; t-- ) {
      _packet[_packetPos--] = pdu->value.data[t];
    }
    //length of entire value being passed
    _packet[_packetPos--] = lsb(pdu->value.size);
    _packet[_packetPos--] = msb(pdu->value.size);
    _packet[_packetPos--] = 0x82;//Sending length in two octets
  }

  _packet[_packetPos--] = (byte)SNMP_SYNTAX_SEQUENCE;// type
  
  // Error Index (size always 4 e.g. 4-byte int)
  u.int32 = pdu->errorIndex;
  _packet[_packetPos--] = u.data[0];
  _packet[_packetPos--] = u.data[1];
  _packet[_packetPos--] = u.data[2];
  _packet[_packetPos--] = u.data[3];
  _packet[_packetPos--] = 0x04;
  _packet[_packetPos--] = (byte)SNMP_SYNTAX_INT;// type

  // Error (size always 4 e.g. 4-byte int)
  u.int32 = pdu->error;
  _packet[_packetPos--] = u.data[0];
  _packet[_packetPos--] = u.data[1];
  _packet[_packetPos--] = u.data[2];
  _packet[_packetPos--] = u.data[3];
  _packet[_packetPos--] = 0x04;
  _packet[_packetPos--] = (byte)SNMP_SYNTAX_INT;// type

  // Request ID (size always 4 e.g. 4-byte int)
  u.int32 = pdu->requestId;
  _packet[_packetPos--] = u.data[0];
  _packet[_packetPos--] = u.data[1];
  _packet[_packetPos--] = u.data[2];
  _packet[_packetPos--] = u.data[3];
  _packet[_packetPos--] = 0x04;
  _packet[_packetPos--] = (byte)SNMP_SYNTAX_INT;// type

  //length value of all previous data
  _packet[_packetPos--] = lsb(packet_length()+_extra_data_size);
  _packet[_packetPos--] = msb(packet_length()+_extra_data_size);
  _packet[_packetPos--] = 0x82;//Sending length in two octets
    
  // SNMP PDU type
  _packet[_packetPos--] = (byte)pdu->type;

  //data needed for header
  _dstType = pdu->type;
  //byte header_size = this->writeHeaders(pdu);
  this->writeHeaders(pdu);
    
  _packetSize = packet_length();
  
  return writePacket(to_address, to_port, extra_data);
}

SNMP_API_STAT_CODES SNMPClass::writePacket(IPAddress address, uint16_t port, char *extra_data)
{
  Udp.beginPacket(address, port);
  Udp.write(_packet+_packetPos+1, _packetSize);
  
  if(extra_data != NULL){
    Udp.write((byte*)extra_data, _extra_data_size);
  }
  
  Udp.endPacket();
  
  return SNMP_API_STAT_SUCCESS;
}

void SNMPClass::onPduReceive(onPduReceiveCallback pduReceived)
{
  _callback = pduReceived;
}

void SNMPClass::freePdu(SNMP_PDU *pdu)
{
  pdu->clear();
}

IPAddress SNMPClass::remoteIP(){
  return Udp.remoteIP();
}

uint16_t SNMPClass::remotePort(){
  return Udp.remotePort();
}

uint16_t SNMPClass::packet_length(){
  return SNMP_MAX_PACKET_LEN - _packetPos - 1;
}

//returns the first byte of a two byte integer
byte SNMPClass::msb(uint16_t num){
  return num >> 8;
}

//returns the second byte of a two byte integer
byte SNMPClass::lsb(uint16_t num){
  return num & 0xFF;
}

uint16_t SNMPClass::combine_msb_lsb(byte msb, byte lsb){
  return (msb << 8)| lsb;
}

// Create one global object
SNMPClass SNMP;