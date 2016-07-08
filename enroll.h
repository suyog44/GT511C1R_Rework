#ifndef ENROLL_H
#define ENROLL_H


uint8_t getFingerprintEnroll(
    HardwareSerial& Serial, GT511_Fingerprint& finger, 
    uint8_t id,
    void (*emit)(const char* event) 
);


#endif

