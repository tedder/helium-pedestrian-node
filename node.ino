#include <LongFi.h>
#include <Wire.h>

#include <Arduino.h>
#include <SPI.h>
#include <longfi-device/radio/sx1276/sx1276.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"

#include "secrets.h"

#ifdef ENABLE_SD_LOG
#include "SparkFun_Qwiic_OpenLog_Arduino_Library.h"
#endif
#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
// possible to add:
// UBX-NAV-SAT 0x01 0x35 for sats seen/healthy


// manual/docu links:
// http://j.mp/2qYx7Mc


//const uint8_t LED = 13;

// time between packet sends
const uint32_t SEND_INTERVAL = 30;
const uint32_t GPS_INTERVAL = 10;

// set OUI and device_id to work with LongFi routing
const uint32_t oui = 1;

// put these in secrets.h
// const uint16_t device_id = 
// const uint8_t preshared_key[16] = {0xXX, 0xXX, 0xXX, etc}


#ifdef _VARIANT_ARDUINO_STM32_
// Discovery L072CZ-LRWAN1
const uint8_t RADIO_RESET_PIN = RADIO_RESET_PORT;
const uint8_t RADIO_DIO_0_PIN = RADIO_DIO_0_PORT;
const uint8_t RADIO_MOSI_PIN  = RADIO_MOSI_PORT;
const uint8_t RADIO_MISO_PIN  = RADIO_MISO_PORT;
const uint8_t RADIO_SCLK_PIN  = RADIO_SCLK_PORT;
const uint8_t RADIO_SS_PIN    = RADIO_NSS_PORT;
const uint8_t LED             = LED_BLUE; 
LongFi LongFi(LongFi::RadioType::SX1276, RADIO_RESET_PIN, RADIO_SS_PIN, RADIO_DIO_0_PIN);
#endif
#ifdef _VARIANT_ARDUINO_CATENA_461x_
// MCCI Catena 4610
const uint8_t RADIO_RESET_PIN = RADIO_RESET;
const uint8_t RADIO_TCXO_PIN  = 33;
const uint8_t RADIO_DIO_0_PIN = 25;
const uint8_t RADIO_MOSI_PIN  = RADIO_MOSI;
const uint8_t RADIO_MISO_PIN  = RADIO_MISO;
const uint8_t RADIO_SCLK_PIN  = RADIO_SCK;
const uint8_t RADIO_SS_PIN    = RADIO_SS;
const uint8_t LED             = LED_RED;
LongFi LongFi(LongFi::RadioType::SX1276, RADIO_RESET_PIN, RADIO_SS_PIN,
  RADIO_DIO_0_PIN, RADIO_TCXO_PIN);
#endif
#ifdef _VARIANT_ARDUINO_ZERO_
// Adafruit Feather M0 Lora
const uint8_t RADIO_RESET_PIN = 4;
const uint8_t RADIO_DIO_0_PIN = 3;
const uint8_t RADIO_SS_PIN    = 8;
const uint8_t LED             = LED_BUILTIN;
LongFi LongFi(LongFi::RadioType::SX1276, RADIO_RESET_PIN, RADIO_SS_PIN, RADIO_DIO_0_PIN);
#endif

// pins: http://j.mp/2R5CkN2
const uint8_t LD7 = LED_GREEN;
const uint8_t BUTTON = USER_BTN;

//uint8_t data[80] = {0};
char data[120] = {0};

uint32_t last_read_time = 0;
uint32_t last_sent_time = 0;
uint32_t last_oled_time = 0;
bool gps_good = false;
bool haz_data = false;
uint16_t gps_bad_count = 0;
uint16_t gps_good_count = 0;

SFE_UBLOX_GPS gps;
SSD1306AsciiWire oled;
#ifdef ENABLE_SD_LOG
OpenLog logger;
#endif

struct PAYLOAD {
  uint8_t counter = 0;
  byte satellite_count = 0;
  
  int32_t lon = 0;
  int32_t lat = 0;
  
  uint32_t time_of_week = 0;
  uint32_t horizontal_accuracy = 0;
  uint32_t vertical_accuracy = 0;
  uint32_t time_accuracy = 0;
  uint8_t version_high = 0;
  uint8_t version_low = 0;
};
uint32_t last_time_of_week = 0;
PAYLOAD payload;


/*
void write_payload(PAYLOAD payload) {
  // write to SD card
  StaticJsonDocument<200> json;
  doc["counter"] = payload.counter;
  doc["satellite_count"] = payload.satellite_count;
  
  doc["lon"] = payload.lon;
  doc["lat"] = payload.lat;
  
  doc["time_of_week"] = payload.time_of_week;
  doc["horizontal_accuracy"] = payload.horizontal_accuracy;
  doc["vertical_accuracy"] = payload.vertical_accuracy;
  doc["time_accuracy"] = payload.time_accuracy;
  doc["version_high"] = payload.version_high;
  doc["version_low"] = payload.version_low;

  // append
  //serializeJson(doc, std::cout);

}
*/

uint16_t log_count = 0;
void log_send(char* ymd, PAYLOAD payload) {
#ifdef ENABLE_SD_LOG
  // set our filename
  logger.append(String(ymd) + String(".txt"));
  
  logger.print(F("counter="));
  logger.print(payload.counter);
  logger.print(F(" satellite_count="));
  logger.print(payload.satellite_count);
  logger.print(F(" lon="));
  logger.print(payload.lon);
  logger.print(F(" lat="));
  logger.print(payload.lat);
  logger.print(F(" time_of_week="));
  logger.print(payload.time_of_week);
  logger.print(F(" horizontal_accuracy="));
  logger.print(payload.horizontal_accuracy);
  logger.print(F(" version_high="));
  logger.print(payload.version_high);
  logger.print(F(" version_low="));
  logger.print(payload.version_low);
  logger.println(F(""));
  logger.println(F("--------------"));

  logger.syncFile();
#endif
  ++log_count;
}

void flash(int pin, int finish=HIGH, int _delay=200) {
  pinMode(pin, OUTPUT);
  for(int i = 0; i<5; ++i) {
    digitalWrite(pin, HIGH);
    delay(_delay);
    digitalWrite(pin, LOW);
    delay(_delay);
  }
  digitalWrite(pin, finish);
}

bool init_gps() {
  flash(LED, HIGH, 80);
  
  //if (! gps.begin(Wire2, 0x42)) {
  if (! gps.begin(Wire, 0x42)) {
    digitalWrite(LED, HIGH);
    Serial.println(F("gps not found on begin()"));
    oled.println(F("no gps at 1"));
    return false;
  }
  oled.println(F("yes gps at 1"));

/*
 * a good packet:
Sending: CLS:1 ID:7 Len: 0x0 Payload:
Zero bytes available
Zero bytes available
Zero bytes available
Zero bytes available
Size: 92 Received: CLS:1 ID:7 Len: 0x5C Payload: 28 ED D2 8 E3 7 B 12 11 7 \
  3 37 2 2D 31 1 8C 1B 0 0 0 0 4 0 8B E0 C5 D5 14 6C 35 17 8 3B 80 0 CF C9 \
  80 0 FF FF FF FF 0 CF 84 DF 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3F 4E\
  0 0 80 A8 12 1 F 27 0 0 E0 4A 23 0 0 0 0 0 0 0 0 0
CLS/ID match!
Lat: 389377044 Long: -708452213 (degrees * 10^-7) Alt: 8403720 (mm) SIV: 0

*/

  if (! gps.isConnected()) {
    digitalWrite(LED, HIGH);
    Serial.println(F("gps not found on isConnected()"));
    oled.println(F("no gps at 2"));
    return false;
  }
  oled.println(F("yes gps at 2"));

  //gps.factoryDefault();
  gps.disableDebugging();
  //gps.enableDebugging(Serial);

  gps.setI2COutput(COM_TYPE_UBX); // UBX only
  //gps.saveConfiguration(); //Save the current settings to flash and BBR
  
  flash(LED, HIGH, 400);
  
  return true;
}

void setup() {
  Serial.begin(115200);

  Serial.println(F("Setup Start"));

  //Wire2.setClock(100000);
  Wire.begin();
#ifdef ENABLE_SD_LOG
  logger.begin(0x29);
#endif


  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);
  oled.clear();
  oled.print(F("Hello world!"));
  scan_i2c();
  
  pinMode(BUTTON, INPUT_PULLDOWN);
  
  pinMode(LED, OUTPUT);
  pinMode(LD7, OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(LD7, HIGH);
  
  #if defined(_VARIANT_ARDUINO_STM32_) || defined(_VARIANT_ARDUINO_CATENA_461x_) 
  SPI.setMOSI(RADIO_MOSI_PIN);
  SPI.setMISO(RADIO_MISO_PIN);
  SPI.setSCLK(RADIO_SCLK_PIN);
  SPI.setSSEL(RADIO_SS_PIN);
  #endif
  SPI.begin();
  delay(1000);
  LongFi.init(oui, device_id, preshared_key);

  for(int i = 0; i < 10; ++i) {
    gps_good = init_gps();
    Serial.print(F("init gps? "));
    Serial.println(gps_good);
    if (gps_good) break;
    delay(2000);
  }
  
  Serial.print(F("gps? "));

  Serial.print(gps_good);
  Serial.print(F(" "));
  Serial.println(gps.isConnected());
  
  
  Serial.println(F("Setup Complete"));
  data[72] = 0;

  flash(LD7, HIGH, 60); // aka LED_GREEN==4==LED_LD1 // top green
  flash(LED, HIGH, 600); // middle blue
  flash(LED_LD2, LOW, 2); // upper section green
  //flash(LED_LD1, HIGH, 600); // top green
  //flash(LED_LD4, HIGH, 600); // lowest red
  //flash(LED_LD4, HIGH, 60);
      
  for(int i=0; i<120; ++i) {
    data[i] = 0x40;
  }
  
}

void show_oled() {
  bool gps_ok = true;
  uint8_t gps_pts = 0;
  oled.clear();

  oled.print("gps ");
  if (gps.isConnected()) {
    oled.print(F("ok"));
  } else {
    oled.print(F("bad "));
    oled.print(gps_bad_count);
  }
  oled.print(F(" "));
  oled.println(gps_good_count);

  // median of "seen" points is ~13500
  oled.print(F("loc "));
  if (payload.horizontal_accuracy < 15000 && payload.satellite_count > 1) {
    oled.println(F("ok"));
  } else {
    oled.println(F("bad"));
  }

  oled.print(F("gps read "));
  if (haz_data) {
    uint32_t last_gps_sec = (millis()-last_read_time)/1000;
    oled.print(last_gps_sec);
    oled.println(F("sec"));
  } else {
    oled.println(F("n/a"));
  }
  
  oled.print(F("    sent "));
  if (payload.counter > 0) {
    uint32_t last_sent_sec = (millis()-last_sent_time)/1000;
    oled.print(last_sent_sec);
    oled.println(F("sec"));
  } else {
    oled.println(F("n/a"));
  }

  oled.print(F("sd card "));
  oled.println(log_count);
  
  oled.print(F("vers "));
  oled.print(payload.version_high);
  oled.print(F("."));
  oled.println(payload.version_low);
}

void scan_i2c() {
  byte error, address;
  int nDevices;

  Serial.println(F("Scanning..."));
  oled.println(F("i2c?"));

  nDevices = 0;
  for(address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print(F("I2C device found at address 0x"));
      if (address < 16) {
        Serial.print(F("0"));
        oled.print(F("0"));
      }
      Serial.println(address, HEX);
      oled.println(address, HEX);

      nDevices++;
    }
    else if (error == 4) {
      Serial.print(F("Unknown error at address 0x"));
      if (address < 16) 
        Serial.print(F("0"));
      Serial.println(address, HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println(F("No I2C devices found"));
    oled.println(F("zero i2c :O"));
    flash(LED_LD2, HIGH, 60);
  }
  else {
    Serial.println(F("done"));
    flash(LED_LD2, LOW, 2);
  }

}


uint8_t n_reads = 0;
uint8_t counter = 4;

void loop() {
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if ( (millis() - last_oled_time) > 10*1000) {
    last_oled_time = millis();
    show_oled();
  }

  if (millis() - last_read_time > GPS_INTERVAL*1000) {
    last_read_time = millis(); //Update the timer
    if (gps.getProtocolVersion()) {
      payload.version_low = gps.getProtocolVersionLow();
      payload.version_high = gps.getProtocolVersionHigh();
    }
    
    if (!gps.getPVT() ) {
      ++gps_bad_count;      
    } else {
      ++gps_good_count;
      flash(LD7, HIGH, 600); // aka LED_GREEN==4==LED_LD1 // top green
       
      Serial.print(F("gps? "));
  
      Serial.print(gps_good);
      Serial.print(F(" "));
      Serial.println(gps.isConnected());
  
      payload.lon = gps.getLongitude();
      payload.lat = gps.getLatitude();
      int32_t altitude = gps.getAltitude();
      
      Serial.print(F("LLA "));
      Serial.print(payload.lon);
      Serial.print(F(" "));
      Serial.print(payload.lat);
      Serial.print(F(" "));
      Serial.println(altitude);
  
      Serial.print(F("FL "));
      Serial.println();
  
      payload.horizontal_accuracy = gps.getHorizontalAccuracy();
      payload.vertical_accuracy = gps.getVerticalAccuracy();
      payload.time_accuracy = gps.getTimeAccuracy();
      payload.time_of_week = gps.getTimeOfWeek();
      payload.satellite_count = gps.getSIV();
      //Serial.print("TOW: ");
      //Serial.print(timeofweek);
  
      if ( abs(payload.lat) > 0 || abs(payload.lon) > 0 || abs(altitude) > 0) {
        flash(LD7, HIGH, 60); // aka LED_GREEN==4==LED_LD1 // top green
        haz_data = true;
        Serial.print(F(" Lat: "));
        Serial.print(payload.lat);
    
        Serial.print(F(" Long: "));
        Serial.print(payload.lon);
        Serial.print(F(" (degrees * 10^-7)"));
    
        Serial.print(F(" Alt: "));
        Serial.print(altitude);
        Serial.print(F(" (mm)"));
  
        digitalWrite(LD7, LOW);
      }
  
  
      Serial.print(F(" SIV: "));
      Serial.print(payload.satellite_count);
      Serial.println();
  
      if (payload.time_of_week > 0) {
        haz_data = true;
        Serial.print(F(" TOW: "));
        Serial.print(payload.time_of_week);
      }
      Serial.print(F(" hACC: "));
      Serial.print(payload.horizontal_accuracy);
      Serial.print(F(" vACC: "));
      Serial.print(payload.vertical_accuracy);
      Serial.print(F(" tACC: "));
      Serial.print(payload.time_accuracy);

  
      if (payload.version_low > 0 || payload.version_high > 0) {
        Serial.print(F("fw version: "));
        Serial.print(payload.version_low);
        Serial.print(F("."));
        Serial.println(payload.version_high);
      }
  
      Serial.print(F(" millis delta: "));
      Serial.println(millis()-last_sent_time);
  
  
      if ((millis()-last_sent_time) > SEND_INTERVAL*1000 &&
        payload.time_of_week != last_time_of_week) {
  
        char ymd_str[13] = {0};
        sprintf(ymd_str, "%04d%02d%02d", gps.getYear(), gps.getMonth(), gps.getDay());
  
        last_sent_time = millis();
        Serial.println(F("sending lora."));
        payload.counter++;
        
        // https://github.com/mikalhart/TinyGPSPlus/blob/master/src/TinyGPS%2B%2B.h
  
        LongFi.send((uint8_t*)&payload, sizeof(payload));
        log_send(ymd_str, payload);
        Serial.println(F("one done."));
        flash(LED, HIGH, 60); // middle blue
  
        last_time_of_week = payload.time_of_week;
  
      }
    }
  }
}
