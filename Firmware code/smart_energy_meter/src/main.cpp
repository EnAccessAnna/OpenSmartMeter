// defines

// Arduino base libraries
#include <Wire.h>
#include "Arduino.h"

// third party libraries
#include <ArduinoHttpClient.h>

// OpenSmartMeter libraries
#include "credit.hpp"
#include "global_defines.hpp"
#include "helpers.hpp"
#include "lcd_display.hpp"
#include "lcd_init.hpp"
#include "mem_init.hpp"
#include "power.hpp"
#include "relay.hpp"
#include "remote.hpp"
#include "sts_token.hpp"
#include "thingsboard.hpp"
#include "time_management.hpp"
#include "token_management.hpp"

HardwareSerial Serial2(PA3, PA2);

byte fe1[8] = {0b00011, 0b00011, 0b00011, 0b00011,
               0b00011, 0b11111, 0b11111, 0b11111};

byte fe3[8] = {0b11111, 0b11111, 0b11111, 0b00011,
               0b00011, 0b00011, 0b00011, 0b00011};

byte fe2[8] = {0b11111, 0b11111, 0b00000, 0b00000,
               0b00000, 0b01100, 0b01100, 0b01111};

byte fe4[8] = {0b01111, 0b01100, 0b01100, 0b00000,
               0b00000, 0b00000, 0b00000, 0b00000};

void setup() {
  
  Serial2.begin(115200);
  Serial2.print("Device Powered! \n");
  
  lcd.begin(16, 2);
  pinMode(buzzer, OUTPUT);
  pinMode(relaya, OUTPUT);
  pinMode(relayb, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);

    //Display Intro Screen.
  digitalWrite(buzzer, HIGH);
  lcd.createChar(0, fe1);
  lcd.createChar(1, fe2);
  lcd.createChar(2, fe3);
  lcd.createChar(3, fe4);
  lcd.setCursor(0, 0);
  lcd.print("    FIRST      ");
  lcd.setCursor(0, 1);
  lcd.print("    ELECTRIC CO.   ");
  lcd.setCursor(1, 0);
  lcd.write(byte(0));
  lcd.setCursor(2, 0);
  lcd.write(byte(1));
  lcd.setCursor(1, 1);
  lcd.write(byte(2));
  lcd.setCursor(2, 1);
  lcd.write(byte(3));
  delay(2000);
  lcd.clear();
  digitalWrite(buzzer, LOW);

  //Begin Device Initialization. 
  lcd.setCursor(0, 0);
  lcd.print(" SYSTEM BOOTING ");
  lcd.setCursor(0, 1);
  lcd.print(" #------------#");

  //Configure GSM modem.
  Serial2.println("Initializing modem...");
  Serial1.begin(115200);
  delay(1500);
  Serial1.write("AT+IPR=9600\r\n");
  Serial1.end();
  Serial1.begin(9600);
  modem.restart();
  String modemInfo = modem.getModemInfo();
  lcd.setCursor(0, 0);
  lcd.print("                     ");
  lcd.setCursor(0, 1);
  lcd.print("                    ");

  Serial2.print("Modem: ");
  Serial2.println(modemInfo);
  lcd.setCursor(0, 0);
  lcd.print("ModemID ");
  lcd.setCursor(0, 1);
  lcd.print(modemInfo);
  delay(2000);
  lcd.clear();

  //Configure RTC
  while(rtc.begin() == false ) { 
    lcd.print("Couldn't find RTC"); 
    delay(500);
  } 

  if (!rtc.isrunning()) {
    //If not running, enable and set Dat&time.
    rtc.adjust(DateTime(2020,1,1,3,0,0));     //<YEAR>,<MTH>,<DAY>,<HR><MIN><SEC>
    delay(500);
  }

  //Configure AFE.  
  ATM90E26.begin(9600);
  AFE_chip.SET_register_values();
  delay(500);

  //Get Meter AFE checksum
  lcd.setCursor(0, 0);
  lcd.print("CSOne :         ");
  lcd.setCursor(8, 0);
  lcd.print(AFE_chip.FETCH_MeterCSOne());

  lcd.setCursor(0, 1);
  lcd.print("CSTwo :         ");
  lcd.setCursor(8, 1);
  lcd.print(AFE_chip.FETCH_MeterCSTwo());
  delay(3000);

  // Configure EEPROM.
  // mem.writeLong(credit_eeprom_location, 200);
  //  mem.writeLong(eeprom_location_cnt, token_eeprom_location);
  //  delay(20);
  //  mem.writeLong(token_eeprom_location, 200);
  //  delay(20);
  creditt = mem.readLong(credit_eeprom_location);
  if (creditt >= 0) {
    mem.writeLong(credit_eeprom_location, creditt);     //Not sure why this is needed. Should be removed.
  }

  /* TODO: Get device configuration parameters from eeprom */
  // meter_const_config deviceConfig;     //Meter configuration struct.
  //
  // deviceConfig =  mem.readLong(deviceConfig_location);
  // while(deviceConfig.status == NOT_SET_CONFIG)
  // {
  //   Serial2.println("Device configuration required...\n");
  //   STEPS:
  //    1. Start timer and wait for command from serial 2 containing the configuration parameters.
  //    2. Verify Configuration parameters and save into struct "deviceConfig" 
  //    3. Save struct into the EEPROM. 
  // }

  delay(10);
  relay_on();
  select_mode();

  if (is_STSmode == true) {
#if defined(TIM1)
    TIM_TypeDef* Instance = TIM1;
#else
    TIM_TypeDef* Instance = TIM2;
#endif
    HardwareTimer* MyTim = new HardwareTimer(Instance);
    MyTim->setOverflow(20, HERTZ_FORMAT);
    MyTim->attachInterrupt(urgeent);
    MyTim->resume();
  } else {
    printf("OpenPAYGO code written here");
  }
}

void loop() {
  if (is_STSmode) {
  /**RUN CODE FOR STS MODE**/
    mesure();                         // Read AFE parameters. 
    if ((mains_input_value > 50))
      credit_reminder();
    if ((mains_input_value < 50)) {
      digitalWrite(red_led, LOW);
      digitalWrite(green_led, LOW);
    }
    get_time();                       //Update Date and Time.

    if ((sts_mode == 0) && (mains_input_value > 50))
      gsm_func();    
  } 
  else {
  /**RUN CODE FOR OpenPAYGO MODE**/
    printf("OpenPAYGO code written here");
  }
}
