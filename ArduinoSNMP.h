/*
  ArduinoSNMP.h - An Arduino library for a lightweight SNMP Agent. v2.3
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

#ifndef ArduinoSNMP_h
#define ArduinoSNMP_h

#define SNMP_DEFAULT_PORT	161
#define SNMP_MANAGER_PORT	162
#define SNMP_MIN_OID_LEN	2
#define SNMP_MAX_OID_LEN	64 // 128
#define SNMP_MAX_NAME_LEN	20
#define SNMP_MAX_VALUE_LEN      256
#define SNMP_MAX_PACKET_LEN     SNMP_MAX_VALUE_LEN + SNMP_MAX_OID_LEN + 25 //25 is arbitrary
#define SNMP_FREE(s)   do { if (s) { free((void *)s); s=NULL; } } while(0)
//Frees a pointer only if it is !NULL and sets its value to NULL. 

#include "Arduino.h"
#include "Udp.h"

extern "C" {
  // callback function
  typedef void (*onPduReceiveCallback)(void);
}

//typedef long long int64_t;
typedef unsigned long long uint64_t;
//typedef long int32_t;
//typedef unsigned long uint32_t;
//typedef unsigned char uint8_t;
//typedef short int16_t;
//typedef unsigned short uint16_t;


typedef union uint64_u {
  uint64_t uint64;
  byte data[8];
};

typedef union int32_u {
  int32_t int32;
  byte data[4];
};

typedef union uint32_u {
  uint32_t uint32;
  byte data[4];
};

typedef union int16_u {
  int16_t int16;
  byte data[2];
};

typedef union uint16_u {
  uint16_t uint16;
  byte data[2];
};

//typedef union uint16_u {
//	uint16_t uint16;
//	byte data[2];
//};

typedef enum ASN_BER_BASE_TYPES {
  //   ASN/BER base types
  ASN_BER_BASE_UNIVERSAL 	 = 0x0,
  ASN_BER_BASE_APPLICATION = 0x40,
  ASN_BER_BASE_CONTEXT 	 = 0x80,
  ASN_BER_BASE_PUBLIC 	 = 0xC0,
  ASN_BER_BASE_PRIMITIVE 	 = 0x0,
  ASN_BER_BASE_CONSTRUCTOR = 0x20
};

typedef enum SNMP_PDU_TYPES {
  // PDU choices
  SNMP_PDU_GET	  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 0,
  SNMP_PDU_GET_NEXT = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 1,
  SNMP_PDU_RESPONSE = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 2,
  SNMP_PDU_SET	  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 3,
  SNMP_PDU_TRAP	  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 4,//obsolete
  SNMP_PDU_GET_BULK_REQUEST  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 5,
  SNMP_PDU_INFORM_REQUEST  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 6,
  SNMP_PDU_TRAP2  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 7,
  SNMP_PDU_REPORT  = ASN_BER_BASE_CONTEXT | ASN_BER_BASE_CONSTRUCTOR | 8,
};

typedef enum SNMP_TRAP_TYPES {
  //   Trap generic types:
  SNMP_TRAP_COLD_START 	      = 0,
  SNMP_TRAP_WARM_START 	      = 1,
  SNMP_TRAP_LINK_DOWN 	      = 2,
  SNMP_TRAP_LINK_UP 	      = 3,
  SNMP_TRAP_AUTHENTICATION_FAIL = 4,
  SNMP_TRAP_EGP_NEIGHBORLOSS    = 5,
  SNMP_TRAP_ENTERPRISE_SPECIFIC = 6
};

typedef enum SNMP_ERR_CODES {
  SNMP_ERR_NO_ERROR 	  		= 0,
  SNMP_ERR_TOO_BIG 	  		= 1,
  SNMP_ERR_NO_SUCH_NAME 			= 2,
  SNMP_ERR_BAD_VALUE 	  		= 3,
  SNMP_ERR_READ_ONLY 	  		= 4,
  SNMP_ERR_GEN_ERROR 	  		= 5,

  SNMP_ERR_NO_ACCESS	  		= 6,
  SNMP_ERR_WRONG_TYPE   			= 7,
  SNMP_ERR_WRONG_LENGTH 			= 8,
  SNMP_ERR_WRONG_ENCODING			= 9,
  SNMP_ERR_WRONG_VALUE			= 10,
  SNMP_ERR_NO_CREATION			= 11,
  SNMP_ERR_INCONSISTANT_VALUE 		= 12,
  SNMP_ERR_RESOURCE_UNAVAILABLE		= 13,
  SNMP_ERR_COMMIT_FAILED			= 14,
  SNMP_ERR_UNDO_FAILED			= 15,
  SNMP_ERR_AUTHORIZATION_ERROR		= 16,
  SNMP_ERR_NOT_WRITABLE			= 17,
  SNMP_ERR_INCONSISTEN_NAME		= 18
};

typedef enum SNMP_API_STAT_CODES {
  SNMP_API_STAT_SUCCESS = 0,
  SNMP_API_STAT_MALLOC_ERR = 1,
  SNMP_API_STAT_NAME_TOO_BIG = 2,
  SNMP_API_STAT_OID_TOO_BIG = 3,
  SNMP_API_STAT_VALUE_TOO_BIG = 4,
  SNMP_API_STAT_PACKET_INVALID = 5,
  SNMP_API_STAT_PACKET_TOO_BIG = 6,
  SNMP_API_STAT_NO_SUCH_NAME = 7,
};

//
// http://oreilly.com/catalog/esnmp/chapter/ch02.html Table 2-1: SMIv1 Datatypes

typedef enum SNMP_SYNTAXES {
  //   SNMP ObjectSyntax values
  SNMP_SYNTAX_SEQUENCE 	       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_CONSTRUCTOR | 0x10,
  //   These values are used in the "syntax" member of VALUEs
  SNMP_SYNTAX_BOOL 	       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 1,
  SNMP_SYNTAX_INT 	       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 2,
  SNMP_SYNTAX_BITS 	       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 3,
  SNMP_SYNTAX_OCTETS 	       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 4,
  SNMP_SYNTAX_NULL 	       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 5,
  SNMP_SYNTAX_OID		       = ASN_BER_BASE_UNIVERSAL | ASN_BER_BASE_PRIMITIVE | 6,
  SNMP_SYNTAX_INT32 	       = SNMP_SYNTAX_INT,
  SNMP_SYNTAX_IP_ADDRESS         = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 0,
  SNMP_SYNTAX_COUNTER 	       = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 1,
  SNMP_SYNTAX_GAUGE 	       = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 2,
  SNMP_SYNTAX_TIME_TICKS         = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 3,
  SNMP_SYNTAX_OPAQUE 	       = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 4,
  SNMP_SYNTAX_NSAPADDR 	       = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 5,
  SNMP_SYNTAX_COUNTER64 	       = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 6,
  SNMP_SYNTAX_UINT32 	       = ASN_BER_BASE_APPLICATION | ASN_BER_BASE_PRIMITIVE | 7,
};

typedef struct SNMP_OID {
  unsigned int data[SNMP_MAX_OID_LEN];  // ushort array insted??
  size_t size;
  
  /**
   * Decodes an OID
   * Decoding algorithm is based on decode_object_id from https://github.com/hallidave/ruby-snmp/blob/master/lib/snmp/ber.rb
   *   Sets size equal to the decoded OID's legnth.
   *
   * Original Author: Rex Park
   * Updated: April 2, 2013
   */
  SNMP_API_STAT_CODES decode(const byte *value, byte length){
    clear();

    byte value_index = 0;
    
    if(value[0] == 0x2b){
      data[0] = 1;
      data[1] = 3;
    }
    else{
      data[1] = value[0] % 40;
      data[0] = (value[0] - data[1]) / 40;
      
      if(data[0] > 2){ return SNMP_API_STAT_MALLOC_ERR; }
    }
    size = 2;
    value_index = 1;
    
    unsigned int n = 0;
    for(value_index; value_index < length; value_index++){
      n = (n << 7) + (value[value_index] & 0x7f);
      if(value[value_index] < 0x80){
        data[size++] = n;
        n = 0;
        if(size >= SNMP_MAX_OID_LEN){
          value_index = length;//break loop
          return SNMP_API_STAT_OID_TOO_BIG;
        }
      }
    }
    
    return SNMP_API_STAT_SUCCESS;
  }
  
  /**
   * Prepares an OID for tranmission.
   *   Sets the syntax value, encoded data length, and encodes the OID itself.
   * Assumes that size has been set and is accurate.
   * Encoding algorithm is based on encode_object_id from https://github.com/hallidave/ruby-snmp/blob/master/lib/snmp/ber.rb
   *
   * Returns the number of bytes modified in the buffer.
   *
   * Original Author: Rex Park
   * Updated: March 29, 2013
   */
  byte encode(byte *buffer) {
    byte buffer_index = 2;
    
    if(size > 1){
      if(data[0] < 2 && data[1] > 40){ return 0; }//invalid, get out of here
      
      buffer[0] = SNMP_SYNTAX_OID;
      
      buffer[buffer_index++] = 40*data[0]+data[1];//first encoded byte is a combination of first two OID values.
      
      //loop through rest of OID
      for(byte i = 2; i < size; i++){
        if(data[i] < 0x80){
          buffer[buffer_index++] = data[i];
        }else{
          unsigned int n = data[i];
          byte octets[10];
          byte octet_index = 9;
          
          while(n != 0){
            octets[octet_index--] = (n & 0x7f | 0x80);
            n = n >> 7;
          }
          octets[9] = (octets[9] & 0x7f);
          octet_index++;//counteract the last decrementer

          while(octet_index < 10){
            buffer[buffer_index++] = octets[octet_index++];
          }
        }
      }
    }
    else if (size == 1) {
      buffer[buffer_index++] = (40*data[0]);
    }
    
    buffer[1] = buffer_index-2;//length of encoded OID
    size = buffer_index;

    return buffer_index;
  }
  
  /**
   * Parses OID byte data from char string.
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 4, 2013 (re-wrote integer parsing)
   */
  byte fromString(const char *buffer){
    clear();
    
    byte b_size = strlen(buffer);
    byte n = 0;
    for(byte i = 0; i <= b_size; i++){
      char t[6];
      if(buffer[i] == '.' || n == 5 || buffer[i] == '\0'){
        t[n] = '\0';
        data[size++] = atoi(t);
        n = 0;
      }else if(n < 5){
        t[n++] = buffer[i];
        delay(10);//Delay needed to function on Due
      }
    }
    
    return size;
  }
  
  // byte fromString_P(prog_char *buffer){
  //     clear();
      
  //     byte b_size = strlen_P(buffer);
  //     byte n = 0;
  //     for(byte i = 0; i <= b_size; i++){
  //       char t[6];
  //       if(pgm_read_byte(buffer+i) == '.' || n == 5 || pgm_read_byte(buffer+i) == '\0'){
  //         t[n] = '\0';
  //         data[size++] = atoi(t);
  //         n = 0;
  //       }else if(n < 5){
  //         t[n++] = pgm_read_byte(buffer+i);
  //       }
  //     }
      
  //     return size;
  //   }
  
  /**
   * Copies OID data into a char buffer using dot notation.
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 2, 2013 (simplified process and removed hard coded 1.3 data)
   */
  void toString(char *buffer, byte buffer_length) {
    if(size <= 0){
      buffer[0] = '\0';
      
      return;
    }

    char buff[16];
    for(byte i = 0; i < size; i++){
      utoa(data[i], buff, 10);
      
      if(strlen(buffer) + strlen(buff) + 1 < buffer_length){
        if(i != 0){
          strcat_P(buffer, PSTR("."));
        }
        strcat(buffer, buff);
      }else{
        i = size;
      }
    }
    
    buffer[buffer_length-1] = '\0';//make sure last byte is a null character
  }
  
  void clear(void) {
    memset(data, 0, SNMP_MAX_OID_LEN);
    size = 0;
  }
};

/**
 * SNMP Value structure
 * data = fully encoded byte value (syntax + length of actual data + actual data)
 * size = length of data array.
 *
 * Original Author: Agentuino Project
 * Updated: Rex Park, April 3, 2013 (added comments)
 */
typedef struct SNMP_VALUE {
  byte data[SNMP_MAX_VALUE_LEN];
  size_t size;
  SNMP_SYNTAXES syntax;
  SNMP_OID OID;
  
  uint16_t i; // for encoding/decoding functions

  //
  // ASN.1 decoding functions
  //
  
  /**
   * Decodes ASN Data Types: octet string, opaque syntax
   *  to char string
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 3, 2013 (updated to account for storing syntax and length in data array)
   * Updated: November 13, 2013 (After resolving issues with the requestPdu and responsePdu methods, size and data are no longer stored in data for decodes)
   */
  SNMP_ERR_CODES decode(char *value, size_t max_size) {
    if ( syntax == SNMP_SYNTAX_OCTETS || syntax == SNMP_SYNTAX_OID || syntax == SNMP_SYNTAX_OPAQUE ) {
      if ( size <= max_size ) {
        memcpy(value,data,size);
        value[size] = '\0';
        
        return SNMP_ERR_NO_ERROR;
      } else {
        clear();	
        return SNMP_ERR_TOO_BIG;
      }
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Decodes ASN Data Types: int
   *   to int16
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 3, 2013 (updated to account for storing syntax and length in data array)
   * Updated: November 13, 2013 (After resolving issues with the requestPdu and responsePdu methods, size and data are no longer stored in data for decodes)
   * Updated: December 13, 2013 (Modified decoding function to be accurate.)
   * Updated: November 18, 2015 (Fixed negative value decoding)
   */
  SNMP_ERR_CODES decode(int16_t *value) {
    Serial.println("I'm Here");
    if ( syntax == SNMP_SYNTAX_INT ) {
      memset(value, 0, sizeof(*value));
      byte temp = (1 << 7) & data[0];
      
      if(temp != 0){
        *value = 0xFF;
      }
      
      for(i = 0;i < size;i++)
      {
        *value = *value<<8 | data[i];
      }
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Decodes ASN Data Types: uint32, counter, time-ticks, guage
   *   to uint16
   *
   * Original Author: Agentuino Project
   * Updated: December 13, 2013 (Decodes a UNIT32 into a unsigned int. Has the possibility to cut off data.)
   * Updated: December 13, 2013 (Modified decoding function to be accurate.)
   */
  SNMP_ERR_CODES decode(uint16_t *value) {
    if ( syntax == SNMP_SYNTAX_COUNTER || syntax == SNMP_SYNTAX_TIME_TICKS
      || syntax == SNMP_SYNTAX_GAUGE || syntax == SNMP_SYNTAX_UINT32 ) {
      memset(value, 0, sizeof(*value));
      
      for(i = 0;i < size;i++)
      {
        *value = *value<<8 | data[i];
      }
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Decodes ASN Data Types: int32
   *   to int32
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 3, 2013 (updated to account for storing syntax and length in data array)
   * Updated: November 13, 2013 (After resolving issues with the requestPdu and responsePdu methods, size and data are no longer stored in data for decodes)
   * Updated: December 13, 2013 (Modified decoding function to be accurate.)
   */
  SNMP_ERR_CODES decode(int32_t *value) {
    if ( syntax == SNMP_SYNTAX_INT32 ) {
      memset(value, 0, sizeof(*value));
      for(i = 0;i < size;i++)
      {
        *value = *value<<8 | data[i];
      }
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }

  /**
   * Decodes ASN Data Types: uint32, counter, time-ticks, guage
   *   to uint32
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 3, 2013 (updated to account for storing syntax and length in data array)
   * Updated: November 13, 2013 (After resolving issues with the requestPdu and responsePdu methods, size and data are no longer stored in data for decodes)
   * Updated: December 13, 2013 (Modified decoding function to be accurate.)
   */
  SNMP_ERR_CODES decode(uint32_t *value) {
    if ( syntax == SNMP_SYNTAX_COUNTER || syntax == SNMP_SYNTAX_TIME_TICKS
      || syntax == SNMP_SYNTAX_GAUGE || syntax == SNMP_SYNTAX_UINT32 ) {
      memset(value, 0, sizeof(*value));
      for(i = 0;i < size;i++)
      {
        *value = *value<<8 | data[i];
      }
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }

  /**
   * Decodes ASN Data Types: ip-address, nsap-address
   *   to byte array
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 3, 2013 (updated to account for storing syntax and length in data array)
   * Updated: November 13, 2013 (After resolving issues with the requestPdu and responsePdu methods, size and data are no longer stored in data for decodes)
   * Updated: December 14, 2013 (Data was coming in backwards)
   */
  SNMP_ERR_CODES decode(byte *value) {
    if ( syntax == SNMP_SYNTAX_IP_ADDRESS || syntax == SNMP_SYNTAX_NSAPADDR ) {
      memcpy(value,data,size);

      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }

  /**
   * Decodes ASN Data Types: boolean
   *   to bool
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 3, 2013 (updated to account for storing syntax and length in data array)
   * Updated: November 13, 2013 (After resolving issues with the requestPdu and responsePdu methods, size and data are no longer stored in data for decodes)
   */
  SNMP_ERR_CODES decode_bool(bool *value) {
    if ( syntax == SNMP_SYNTAX_BOOL || syntax == SNMP_SYNTAX_INT32 ) {
      *value = (data[0] != 0);
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  //
  //
  // ASN.1 encoding functions
  //
  
  /**
   * Encodes char string
   * ASN Data Types: octets and opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 1, 2013 (added the ability to specify a buffer, added syntax and length)
   * Updated: Rex Park, November 14, 2013 (modified length to long-form)
   * Updated Rex Park, November 15, 2013 (modified to not add data to buffer, allows sending data that has its own buffer.)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, const char *value, byte *buffer=NULL, boolean ignore_data = false) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    
    if ( syn == SNMP_SYNTAX_OCTETS || syn == SNMP_SYNTAX_OPAQUE ) {
      if ( strlen(value) - 1 < SNMP_MAX_VALUE_LEN || ignore_data == true) {
         size = strlen(value);
        
         buffer[0] = syn;//syntax
         buffer[1] = 0x82;
         buffer[2] = msb(size);
         buffer[3] = lsb(size);
         size += 4;//add in syntax & length bytes
        
        if(ignore_data == false){
          for ( i = 4; i < size; i++ ) {
            buffer[i] = (byte)value[i-4];
          }
        }else{
          size = 4;
        }

        syntax = syn;
        return SNMP_ERR_NO_ERROR;
      } else {
        clear();	
        return SNMP_ERR_TOO_BIG;
      }
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes byte array
   * ASN Data Types: octets and opaque
   *
   * Original Author: Agentuino Project
   * Updated Rex Park, January 08, 2014 (Created based off char array encoder.)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, byte *value, byte length, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    
    if ( syn == SNMP_SYNTAX_OCTETS || syn == SNMP_SYNTAX_OPAQUE ) {
      if ( length < SNMP_MAX_VALUE_LEN) {
        size = length;
        
        buffer[0] = syn;//syntax
        buffer[1] = 0x82;
        buffer[2] = msb(size);
        buffer[3] = lsb(size);
        size += 4;//add in syntax & length bytes
        
        for ( i = 4; i < size; i++ ) {
          buffer[i] = value[i-4];
        }

        syntax = syn;
        return SNMP_ERR_NO_ERROR;
      } else {
        clear();	
        return SNMP_ERR_TOO_BIG;
      }
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes int16 (2 byte signed integer)
   * ASN Data Types: int and opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 1, 2013 (modified encoding algorithm, added the ability to specify a buffer, added syntax and length)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, int16_t value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    if ( syn == SNMP_SYNTAX_INT || syn == SNMP_SYNTAX_OPAQUE ) {
      i = 3;//individual bytes are stored in reverse, start at the end
            
      buffer[0] = syn;
      buffer[1] = 2;
      
      if(value >= 0){
        //i <= 5 stops an infinite loop caused by using decrementer
        while(i > 1 && i <= 3){
          if(value > 0){
            buffer[i--] = (value & 0xff);
            value = value >> 8;
          }else{
            buffer[i--] = 0;
          }
        }
      }else{
        //i <= 5 stops an infinite loop caused by using decrementer
        while(i > 1 && i <= 3){
          if(value < 0){
            buffer[i--] = (value & 0xff);
            value = value >> 8;
          }else{
            buffer[i--] = 0;
          }
        }
      }
      
      syntax = syn;
      size = 4;
      
      /* hack for size bug when sending large messages*/
      if(buffer[2] == 0 && buffer[3] < 0x80){
        buffer[1] = 1;
        buffer[2] = buffer[3];
        buffer[3] = 0;
        size = 3;
      }
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes uint16 (2 byte unsigned integer)
   * ASN Data Types: uint32
   *
   * Updated: Rex Park, December 13, 2013 (Converts unsigned int into a SNMP UInT32)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, uint16_t value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    
    if(syn == SNMP_SYNTAX_UINT32){
      
      i = 4;//individual bytes are stored in reverse
      
      buffer[0] = syn;
      buffer[1] = 3;
      buffer[2] = 0;//for compatibility with NET-SNMP
      
      while(value > 0 && i >= 0){
        buffer[i--] = (value & 0xff);
        value = value >> 8;
      }
      
      syntax = syn;
      size = 5;
      return SNMP_ERR_NO_ERROR;
    }else{
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes int32 (4 byte signed integer)
   * ASN Data Types: int32 and opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 1, 2013 (modified encoding algorithm, added the ability to specify a buffer, added syntax and length)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, int32_t value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    
    if(syn == SNMP_SYNTAX_INT32 || syn == SNMP_SYNTAX_OPAQUE){
      i = 5;//individual bytes are stored in reverse, start at the end
      
      buffer[0] = syn;
      buffer[1] = 4;
      
      if(value >= 0){
        //i <= 5 stops an infinite loop caused by using decrementer
        while(i > 1 && i <= 5){
          if(value > 0){
            buffer[i--] = (value & 0xff);
            value = value >> 8;
          }else{
            buffer[i--] = 0;
          }
        }
      }else{
        //i <= 5 stops an infinite loop caused by using decrementer
        while(i > 1 && i <= 5){
          if(value < 0){
            buffer[i--] = (value & 0xff);
            value = value >> 8;
          }else{
            buffer[i--] = 0;
          }
        }
      }
      
      syntax = syn;
      size = 6;
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes uint32 (4 byte unsigned integer)
   * ASN Data Types: unit32, counter, time-ticks, gauge32, and opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, March 29, 2013 (modified encoding algorithm, added the ability to specify a buffer, added syntax and length)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, uint32_t value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    
    if(syn == SNMP_SYNTAX_COUNTER || syn == SNMP_SYNTAX_TIME_TICKS 
      || syn == SNMP_SYNTAX_GAUGE || syn == SNMP_SYNTAX_UINT32 
      || syn == SNMP_SYNTAX_OPAQUE){
      
      i = 5;//individual bytes are stored in reverse
      
      buffer[0] = syn;
      buffer[1] = 4;
      
      while(value > 0 && i >= 0){
        buffer[i--] = (value & 0xff);
        value = value >> 8;
      }
      
      syntax = syn;
      size = 6;
      return SNMP_ERR_NO_ERROR;
    }else{
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  /**
   * Encodes IPAddress
   * ASN Data Types: ip address, nsap address, opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 2, 2013 (modified encoding algorithm, added the ability to specify a buffer, added syntax and length)
   */
  SNMP_ERR_CODES encode_address(SNMP_SYNTAXES syn, const IPAddress value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
        
    if ( syn == SNMP_SYNTAX_IP_ADDRESS || syn == SNMP_SYNTAX_NSAPADDR || syn == SNMP_SYNTAX_OPAQUE ) {
      buffer[0] = syn;
      buffer[1] = 4;
            
      buffer[2] = value[0];
      buffer[3] = value[1];
      buffer[4] = value[2];
      buffer[5] = value[3];

      syntax = syn;
      size = 6;
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes boolean
   * ASN Data Types: boolean, opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 2, 2013 (added the ability to specify a buffer, added syntax and length)
   * Updated: Rex Park, November 19, 2013 (BOOL is not a valid type. Encode as INT32)
   */
  SNMP_ERR_CODES encode_bool(SNMP_SYNTAXES syn, const bool value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }

    if ( syn == SNMP_SYNTAX_BOOL || syn == SNMP_SYNTAX_OPAQUE ) {
      buffer[0] = SNMP_SYNTAX_UINT32;
      buffer[1] = 1;

      buffer[2] = value ? 0x1 : 0x0;
      
      syntax = syn;
      size = 3;
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes IPAddress
   * ASN Data Types: ip address, nsap address, opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 2, 2013 (added the ability to specify a buffer, added syntax and length)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, const uint64_t value, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
        
    if ( syn == SNMP_SYNTAX_COUNTER64 || syn == SNMP_SYNTAX_OPAQUE ) {
      buffer[0] = syn;
      buffer[1]= 8;
            
      uint64_u tmp;
      tmp.uint64 = value;
      
      buffer[2] = tmp.data[7];
      buffer[3] = tmp.data[6];
      buffer[4] = tmp.data[5];
      buffer[5] = tmp.data[4];
      buffer[6] = tmp.data[3];
      buffer[7] = tmp.data[2];
      buffer[8] = tmp.data[1];
      buffer[9] = tmp.data[0];
      
      syntax = syn;
      size = 10;
      return SNMP_ERR_NO_ERROR;
    } else {
      clear();
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  /**
   * Encodes null
   * ASN Data Types: null, opaque
   *
   * Original Author: Agentuino Project
   * Updated: Rex Park, April 2, 2013 (added the ability to specify a buffer, added syntax and length)
   */
  SNMP_ERR_CODES encode(SNMP_SYNTAXES syn, byte *buffer=NULL) {
    if(buffer == NULL){
      clear();
      buffer = data;
    }
    
    if ( syn == SNMP_SYNTAX_NULL || syn == SNMP_SYNTAX_OPAQUE ) {
      buffer[0] = syn;
      buffer[1] = 0;
      
      syntax = syn;
      size = 2;
      return SNMP_ERR_NO_ERROR;
    } else {
      return SNMP_ERR_WRONG_TYPE;
    }
  }
  
  // clear's buffer and sets size to 0
  void clear(void) {
    //OID.clear(); Breaks encoding
    memset(data, 0, SNMP_MAX_VALUE_LEN);
    size = 0;
    i = 0;
  }
  
  //returns the first byte of a two byte integer
  byte msb(uint16_t num){
    return num >> 8;
  }

  //returns the second byte of a two byte integer
  byte lsb(uint16_t num){
    return num & 0xFF;
  }
};

typedef struct SNMP_PDU {
  SNMP_PDU_TYPES type;
  int32_t version;
  int32_t requestId;
  SNMP_ERR_CODES error;
  int32_t errorIndex;
  SNMP_VALUE value;
  IPAddress agent_address;
  
  /**
   * Adds standard v2c trap data
   *
   * Original Auther: Rex Park
   * Updated: April 5, 2013
   */
  void prepare_trapv2(SNMP_VALUE *t_v){
    version = 1;
    type = SNMP_PDU_TRAP2;
    error = SNMP_ERR_NO_ERROR;
    errorIndex = 0;
    
    //sysUpTime
    t_v->OID.clear();
    t_v->clear();
    t_v->OID.fromString("1.3.6.1.2.1.1.3.0");//OID of the value type being sent
    //This line below causes error, millis() type doesn't have a definite type. 
    //This produces different candidates for one overloaded function call.
    //t_v->encode(SNMP_SYNTAX_TIME_TICKS, millis()/10);
    t_v->encode(SNMP_SYNTAX_TIME_TICKS, (const uint16_t)millis()/10);
    value.size = add_data_private(t_v);
    
    //SNMPv2 trapOID
    t_v->OID.clear();
    t_v->clear();
    t_v->OID.fromString("1.3.6.1.6.3.1.1.4.1.0");//OID of the value type being sent
    t_v->size = value.OID.encode(t_v->data);
    value.size = add_data_private(t_v);
  }

  /**
   * Adds standard v2c inform data
   *
   * Original Auther: Rex Park
   * Updated: October 24, 2015
   */
  void prepare_inform(SNMP_VALUE *t_v){
    prepare_trapv2(t_v);

    //overrides type set from prepare_trapv2
    type = SNMP_PDU_INFORM_REQUEST;
  }

  /** 
   * Encodes data for transmission with the trap.
   * Each trap data item:
   * SNMP_SYNTAX_SEQUENCE
   * length_of_sequence (remainder, does not include previous syntax type or length byte) 
   * OID Syntax
   * OID length
   * OID encoded bytes
   * Data Syntax
   * Data length
   * Data encoded bytes
   *
   * Reverse section adds compatibility to responsePDU rewrite.
   *
   * Original Auther: Rex Park
   * Updated: October 26, 2015 (Created: Calls add_data_private and then updates value.size. One less step for end user.)
   */
    void add_data(SNMP_VALUE *data, byte *buffer=NULL, boolean reverse = false, byte *temp_buffer = NULL, int extra_data_size = 0){
    
      value.size = add_data_private(data,buffer,reverse,temp_buffer,extra_data_size);
    }

  /** 
   * Encodes data for transmission with the trap.
   * Each trap data item:
   * SNMP_SYNTAX_SEQUENCE
   * length_of_sequence (remainder, does not include previous syntax type or length byte) 
   * OID Syntax
   * OID length
   * OID encoded bytes
   * Data Syntax
   * Data length
   * Data encoded bytes
   *
   * Reverse section adds compatibility to responsePDU rewrite.
   *
   * Original Auther: Rex Park
   * Updated: October 26, 2015 (Renamed to add_data_private)
   * Updated: November 4, 2013
   */
  byte add_data_private(SNMP_VALUE *data, byte *buffer=NULL, boolean reverse = false, byte *temp_buffer = NULL, int extra_data_size = 0){
    int index = 0;
    byte start_index = index;
    byte t_index = 0;
    byte i = 0;
    
    if(buffer == NULL){
      buffer = value.data;
      index = value.size;
      start_index = index;
    }
    
    if(reverse == true){
      //250 is arbitrary, just accounting for -1 being 255.
      for(i = data->size-1; i >= 0 && i < 250; i--){//data syn + data len + data
        buffer[index--] = data->data[i];
      }
      
      t_index = data->OID.encode(temp_buffer);//oid syn + oid len + oid data
      //250 is arbitrary, just accounting for -1 being 255.
      for(i = t_index-1; i >= 0 && i < 250; i--){
        buffer[index--] = temp_buffer[i];
      }

      //length of remainder of value
      buffer[index] = lsb(start_index - index + extra_data_size);
      buffer[index-1] = msb(start_index - index + extra_data_size);
      index -= 2;
      buffer[index--] = 0x82;//Sending length in two octets
      buffer[index--] = SNMP_SYNTAX_SEQUENCE;  
      
    }else{
      buffer[index++] = SNMP_SYNTAX_SEQUENCE;
      //length of remainder of value
      buffer[index++] = 0x82;//Sending length in two octets
      buffer[index++] = 0;
      buffer[index++] = 0;
      
      t_index = data->OID.encode(buffer+index);//oid syn + oid len + oid data
      index += t_index;
      
      for(i = 0; i < data->size; i++){//data syn + data len + data
        buffer[index++] = data->data[i];
      }
      
      buffer[start_index+2] = msb(index - start_index - 4 + extra_data_size);//set length
      buffer[start_index+3] = lsb(index - start_index - 4 + extra_data_size);
    }
    
    return start_index + data->size + t_index + 4;//current size of buffer data was stored in
  }
  
  void clear(){
    version = 0;
    requestId = 0;
    errorIndex = 0;
    error = SNMP_ERR_NO_ERROR;
    value.clear();
    value.OID.clear();
  }
  
  //returns the first byte of a two byte integer
  byte msb(uint16_t num){
    return num >> 8;
  }

  //returns the second byte of a two byte integer
  byte lsb(uint16_t num){
    return num & 0xFF;
  }
};

class SNMPClass {
public:
  SNMP_API_STAT_CODES begin(const char *getCommName,const char *setCommName,const char *trapComName, uint16_t port);
  boolean listen(void);
  SNMP_API_STAT_CODES requestPdu(SNMP_PDU *pdu, char *extra_data = NULL, int extra_data_max_size = 0);
  void send_message(IPAddress address, uint16_t port, byte *packet, uint16_t packet_size);
  uint32_t send_message(SNMP_PDU *pdu, IPAddress to_address, uint16_t to_port, byte* b = NULL, char *extra_data = NULL);
  void resend_message(IPAddress address, uint16_t port, char *extra_data = NULL);
  uint32_t sendTrapv1(SNMP_PDU *pdu, SNMP_TRAP_TYPES trap_type, int16_t specific_trap, IPAddress manager_address);
  void onPduReceive(onPduReceiveCallback pduReceived);
  void freePdu(SNMP_PDU *pdu);
  void clear_packet();
  uint16_t copy_packet(byte *packet_store);
  IPAddress remoteIP();
  uint16_t remotePort();
  uint32_t requestCounter;

private:
  uint16_t writeHeaders(SNMP_PDU *pdu);
  void writePacket(IPAddress address, uint16_t port, char *extra_data = NULL);
  byte _packet[SNMP_MAX_PACKET_LEN];
  uint16_t _packetSize;
  uint16_t _packetPos;
  uint16_t _packetTrapPos;
  SNMP_PDU_TYPES _dstType;
  uint8_t _dstIp[4];
  uint16_t _dstPort;
  const char *_getCommName;
  size_t _getSize;
  const char *_setCommName;
  size_t _setSize;
  const char *_trapCommName;
  size_t _trapSize;
  onPduReceiveCallback _callback;
  uint16_t packet_length();
  byte lsb(uint16_t num);
  byte msb(uint16_t num);
  uint16_t combine_msb_lsb(byte msb, byte lsb);
  int _extra_data_size;
  boolean _udp_extra_data_packet;
};

extern SNMPClass SNMP;

#endif
