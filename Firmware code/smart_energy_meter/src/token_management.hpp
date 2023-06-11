#pragma once

// defines
#define Lengths 20

// Arduino base libraries

// third party libraries
#include <Keypad.h>

// OpenSmartMeter libraries
#include "lcd_init.hpp"
#include "mem_init.hpp"
#include "sts_token.hpp"

char customKey;

// keypad
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {{'1', '2', '3', 'A'},
                             {'4', '5', '6', 'B'},
                             {'7', '8', '9', 'C'},
                             {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {PB9, PB8, PB1, PB0};
byte colPins[COLS] = {PA7, PA6, PA5, PC13};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
char Data[Lengths];

int lcd_count = 0;
byte data_count = 0;
byte dt = 0;
byte parameters = 0;
byte token_used = 0;

//    for meter
unsigned long sts_eeprom_fetched = 0;
unsigned long token_eeprom_location = 20;
unsigned long eeprom_location_cnt = 40;

unsigned long sts_value = 0;
unsigned long sts_mode = 0;

void buss() {
  digitalWrite(buzzer, HIGH);
  delay(10);
  digitalWrite(buzzer, LOW);
}

void check_tokenused() {
  token_eeprom_location = mem.readLong(eeprom_location_cnt);
  if (token_eeprom_location > 4000) {
    mem.writeLong(eeprom_location_cnt, 40);
    token_eeprom_location = 40;
  }
  // token_eeprom_location = token_eeprom_location +1;
  unsigned int locationcount;
  for (locationcount = 40; locationcount < token_eeprom_location;
       locationcount++) {
    sts_eeprom_fetched = mem.readLong(locationcount);
    String ee_fetched = String(sts_eeprom_fetched);
    String sts_keyco = sts_data.substring(0, 10);
    String keyco = ee_fetched.substring(0, 10);

    long conv_keyco = keyco.toInt();
    long conv_sts_keyco = sts_keyco.toInt();
    if ((conv_keyco == conv_sts_keyco)) {
      if (c_chek == 0) {
        warntime = warn_now;
        c_chek = 1;
      }
      lcd.setCursor(0, 0);
      lcd.print("  WARNING!!!!!  ");
      lcd.setCursor(0, 1);
      lcd.print("   USED TOKEN   ");
      digitalWrite(buzzer, HIGH);
      token_used = 1;
    }
  }
}

void preset_keyprocess()
{
    data_count = lcd_count = dt = sts_value = parameters =0;
    lcd.clear();
    buss();
    lcd.setCursor(0, 0);
}

void check_meterCMD(void){

}

void STS_keypad() {       //Process KEYPRESS on Keypad. 
  customKey = customKeypad.getKey();    //scan keypad.

  if (customKey == 'A') { //SCROLL up device parameter on LCD.
    if(sts_mode) sts_mode = false;
    parameters = parameters + 1;
  }
  else if (customKey == 'B' && parameters) {  //SCROLL down device parameter on LCD 
    if(sts_mode) sts_mode = false;
    parameters = parameters - 1;
  }
  else if (customKey == '#' && sts_mode == 1 ) {  //Enter Key is Pressed.     
    if(data_count == 20){   //Check if 20digits token command.
      Serial2.println("# Token Entered\n");
      sts_data = Data;
      check_tokenused();
      if (token_used == 0) {
        STStoken_decode();
      }
    }
    else if (data_count == 3 ){ //Check if 3digits command.
      Serial2.println("# Command entered\n");   
      check_meterCMD();
      preset_keyprocess();  
    }
    else {//ERROR, only 20 digits and 3 digits are allowed.
      Serial2.println("Invalid command\n");
      digitalWrite(buzzer, HIGH);      
      lcd.setCursor(0, 0);
      lcd.print("INVALID !!!");
      delay(1000);
      lcd.clear();
      digitalWrite(buzzer, LOW);      
      preset_keyprocess();  
    }  
  }
//  else if (customKey != NO_KEY && customKey != '*' && customKey != '#' && customKey != 'D' ) { //Detect Keys and Display on LCD. 
  else if (customKey != NO_KEY && (0x30 >= customKey <= 0x39) ) { //Detect Numeric Keys (0-9) to display on LCD. 
    if(!sts_mode) {   //If first Key, set screen to display keys pressed.
      dt = 0;
      sts_value = lcd_count = parameters = 0;
      delay(20);
      lcd.clear();
      buss();
      lcd.setCursor(0, 0);
      sts_mode = 1;      
    }
    if (customKey) {
      if (dt < 21) {
        delay(20);
        buss();
        dt++;
        if (dt < 17) {
          Data[data_count] = customKey;
          lcd.setCursor(lcd_count, 0);
          lcd.print(Data[data_count]);
          data_count++;
        }
        if (lcd_count > 15) {
          lcd_count = 0;
        }
        if (dt > 16) {
          Data[data_count] = customKey;
          lcd.setCursor(lcd_count, 1);
          lcd.print(Data[data_count]);
          data_count++;
        }
        lcd_count++;
        sts_value = data_count;
      }
    }
  }
}