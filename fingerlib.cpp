
#include "GT511_Fingerprint.h"

#include <unistd.h>
void delay(int n) {
    usleep( n*1000 );
}

#include <string.h>


GT511_Fingerprint::GT511_Fingerprint(HardwareSerial *ss) {
  passwd = 0;
  addr = 0xFFFFFFFF;
  hwSerial = ss;
  serial = hwSerial;
}

void GT511_Fingerprint::begin(uint16_t baudrate) {
  delay(1000);  // one second delay to let the sensor 'boot up'
  if (hwSerial) hwSerial->begin(baudrate);
}

boolean GT511_Fingerprint::vrfypass(void) {
  uint8_t packt[] = {FINGERPRINT_VERIFYPASSWORD, 
                      (passwd >> 24), (passwd >> 16),
                      (passwd >> 8), passwd};
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, 7, packt);
  uint8_t len = retriveresponce(packt);
  
  if ((len == 1) && (packt[0] == FINGERPRINT_ACKPACKET) && (packt[1] == FINGERPRINT_OK))
    return true;

  return false;
}

uint8_t GT511_Fingerprint::retriveimg(void) {
  uint8_t packt[] = {FINGERPRINT_GETIMAGE};
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, 3, packt);
  uint8_t len = retriveresponce(packt);
  
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packt[1];
}

uint8_t GT511_Fingerprint::image2Tz(uint8_t slot) {
  uint8_t packt[] = {FINGERPRINT_IMAGE2TZ, slot};
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
  uint8_t len = retriveresponce(packt);
  
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET)){
   return -1;
  }
  return packt[1];
}


uint8_t GT511_Fingerprint::createModel(void) {
  uint8_t packt[2];
  packt[0] = FINGERPRINT_REGMODEL;
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, 3, packt);
  uint8_t len = retriveresponce(packt);
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packt[1];
}



uint8_t GT511_Fingerprint::storeModel(uint16_t id) {
  uint8_t packt[] = {FINGERPRINT_STORE, 0x01, id >> 8, id & 0xFF};
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
  uint8_t len = retriveresponce(packt);
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packt[1];
}
    
//read a fingerprint template from flash into Char Buffer 1
uint8_t GT511_Fingerprint::loadModel(uint16_t id) {
    uint8_t packt[] = {FINGERPRINT_LOAD, 0x01, id >> 8, id & 0xFF};
    uint8_t resp_packt[256];
    wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
    uint8_t len = retriveresponce(resp_packt);
    if ((len != 1) && (resp_packt[0] != FINGERPRINT_ACKPACKET)){
        return -1;
    }
    return resp_packt[1];
}

//transfer a fingerprint template from Char Buffer 1 to host computer
uint8_t GT511_Fingerprint::getModel(void) {
    uint8_t packt[] = {FINGERPRINT_UPLOAD, 0x01};
    wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
    uint8_t len = retriveresponce(packt);
    if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
        return -1;
    return packt[1];
}
    
uint8_t GT511_Fingerprint::deleteModel(uint16_t id) {
    uint8_t packt[] = {FINGERPRINT_DELETE, id >> 8, id & 0xFF, 0x00, 0x01};
    wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
    uint8_t len = retriveresponce(packt);
    if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
        return -1;
    return packt[1];
}

uint8_t GT511_Fingerprint::emptyDatabase(void) {
   uint8_t packt[2];
   packt[0] = FINGERPRINT_EMPTY;
   wrtpackt(addr, FINGERPRINT_COMMANDPACKET, 3, packt);
  uint8_t len = retriveresponce(packt);
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
   return -1;
  return packt[1];
}

uint8_t GT511_Fingerprint::fingerFastSearch(void) {
  fingerID = 0xFFFF;
  confidence = 0xFFFF;
  uint8_t packt[] = {FINGERPRINT_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3};
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
  uint8_t len = retriveresponce(packt);
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
   return -1;
  fingerID = packt[2];
  fingerID <<= 8;
  fingerID |= packt[3];
  confidence = packt[4];
  confidence <<= 8;
  confidence |= packt[5];
  return packt[1];
}

uint8_t GT511_Fingerprint::getTemplateCount(void) {
  templateCount = 0xFFFF;
  uint8_t packt[] = {FINGERPRINT_TEMPLATECOUNT};
  uint8_t overflow[8]; 
  memset(overflow,0,sizeof(overflow));
  wrtpackt(addr, FINGERPRINT_COMMANDPACKET, sizeof(packt)+2, packt);
  uint8_t len = retriveresponce(packt);
  if ((len != 1) && (packt[0] != FINGERPRINT_ACKPACKET))
   return -1;
  templateCount = packt[2];
  templateCount <<= 8;
  templateCount |= packt[3];
  return packt[1];
}

void GT511_Fingerprint::wrtpackt(uint32_t addr, uint8_t packttype, 
				       uint16_t len, uint8_t *packt) {
 
  serial->print((uint8_t)(FINGERPRINT_STARTCODE >> 8), BYTE);
  serial->print((uint8_t)FINGERPRINT_STARTCODE, BYTE);
  serial->print((uint8_t)(addr >> 24), BYTE);
  serial->print((uint8_t)(addr >> 16), BYTE);
  serial->print((uint8_t)(addr >> 8), BYTE);
  serial->print((uint8_t)(addr), BYTE);
  serial->print((uint8_t)packttype, BYTE);
  serial->print((uint8_t)(len >> 8), BYTE);
  serial->print((uint8_t)(len), BYTE);
  
  uint16_t sum = (len>>8) + (len&0xFF) + packttype;
  for (uint8_t i=0; i< len-2; i++) {
    serial->print((uint8_t)(packt[i]), BYTE);
    sum += packt[i];
  }
  serial->print((uint8_t)(sum>>8), BYTE);
  serial->print((uint8_t)sum, BYTE);
}


uint8_t GT511_Fingerprint::retriveresponce(uint8_t packt[], uint16_t timeout) {
  uint8_t reply[256], idx;
  uint16_t timer=0;
  
  idx = 0;
while (true) {
    while (!serial->available()) {
      delay(1);
      timer++;
      if (timer >= timeout) return FINGERPRINT_TIMEOUT;
    }
    // something to read!
    reply[idx] = serial->read();
    if ((idx == 0) && (reply[0] != (FINGERPRINT_STARTCODE >> 8)))
      continue;
    idx++;
    
    // check packt!
    if (idx >= 9) {
      if ((reply[0] != (FINGERPRINT_STARTCODE >> 8)) ||
          (reply[1] != (FINGERPRINT_STARTCODE & 0xFF)))
          return FINGERPRINT_BADPACKET;
      uint8_t packttype = reply[6];
      //Serial.print("Packet type"); Serial.println(packttype);
      uint16_t len = reply[7];
      len <<= 8;
      len |= reply[8];
      len -= 2;
      //Serial.print("Packet len"); Serial.println(len);
      if (idx <= (len+10)) continue;

//printf("before set packt\n");
      packt[0] = packttype;      
//printf("before set payload %d bytes \n", len);
      for (uint8_t i=0; i<len; i++) {
        packt[1+i] = reply[9+i];
      }

//printf("return length \n");
      return len;
    }
  }
}

#define CHUNK_SIZE 20
uint8_t GT511_Fingerprint::uploadModel(uint16_t id, uint8_t* model, uint16_t len)
{
    uint8_t packt[256];
    packt[0] = FINGERPRINT_MODEL_XFER;
    packt[1] = 0x01; 
    wrtpackt(addr, FINGERPRINT_COMMANDPACKET, 4, packt);
    uint8_t reply_len = retriveresponce(packt);
    uint16_t i, chunk_len;
    uint8_t chunk[CHUNK_SIZE]; 
    if (reply_len != 1) {
        fprintf(stderr,"uploadModel: expected 2 byte reply got %d bytes packt[0]=0x%02x packt[1]=0x%02x\n", 
           reply_len, packt[0], packt[1] );
        return -1;
    } 
    if (packt[0] != FINGERPRINT_ACKPACKET) {
        fprintf(stderr,"uploadModel: expected ACK, got %d", packt[0]);
        return -1;
    }
    if (packt[1] != FINGERPRINT_OK) { 
        fprintf(stderr,"uploadModel: expected payload = OK, got %d", packt[1]);
        return -1;
    }
    // ready to begin serial xfer    
    chunk_len = 0;
    for(i = 0; i < len; i++) {
         chunk[ chunk_len ] = model[i];
         chunk_len++;

         if ( i == (len-1) ) { // last byte
             //printf("writing end packt\n");
             wrtpackt(addr, FINGERPRINT_ENDDATAPACKET, chunk_len, chunk);  
             
         } else if (chunk_len == CHUNK_SIZE) {        
             //printf("writing data packt\n");
         
             wrtpackt(addr, FINGERPRINT_DATAPACKET, chunk_len, chunk);
             chunk_len = 0;
         }
    }
    printf("store model %d\n", id);  
    return this->storeModel( id );
}
