/****************************************************************************
 * Max Exposure setting library.
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

extern "C" {
#include <cxd56_i2c.h>
#include <cxd56_pmic.h>
}
#include <errno.h>

#include <Wire.h>

#include "isx019_ext.h"

//#define DEBUG // enable debug mode

extern TwoWire Wire2(2); // ISX019 i2c interface

bool create_isx019_read_command(uint8_t cat, uint16_t addr, uint8_t read_size, uint8_t *command) {
  if (read_size > sizeof(uint16_t)) return false;
  command[0] = 9; // Byte num of this commands. This example handles only one command.
  command[1] = 1; // The number of commands. This example handles only one command.
  command[2] = 6; // Byte num of this command
  command[3] = ISX019_I2C_CMD_READ;  
  command[4] = cat;
  command[5] = addr >> 8;
  command[6] = addr & 0xff;
  command[7] = read_size; // read size of the register value.
  // calc checksum
  uint16_t chksum = 0;
  for (int i = 0; i < 9; ++i) {
    chksum += command[i];
  }
  command[8] = (uint8_t)(chksum & 0xff);
  return true;
}

bool create_isx019_write_command(uint8_t cat, uint16_t addr, uint16_t value, uint8_t write_size, uint8_t *command) {
  if (write_size > sizeof(uint16_t)) return false;
  command[1] = 1; // The number of commands. This example handles only one command.
  command[3] = ISX019_I2C_CMD_WRITE;  
  command[4] = cat;
  command[5] = addr >> 8;
  command[6] = addr & 0xff;
  if (write_size == sizeof(uint8_t)) {
    command[0] = 9; // Byte num of this commands. This example handles only one command.
    command[2] = 6; // Byte num of this command
    command[7] = (uint8_t)(value & 0xff);
  } else if (write_size == sizeof(uint16_t)) {
    command[0] = 10; // Byte num of this commands. This example handles only one command.
    command[2] = 7;  // Byte num of this command
    command[7] = (uint8_t)(value >> 8);
    command[8] = (uint8_t)(value & 0xff);
  } 

  // calc checksum
  uint16_t chksum = 0;
  for (int i = 0; i < 8; ++i) {
    chksum += command[i];
  }
  command[8] = (uint8_t)(chksum & 0xff);
  return true;
}

uint16_t read_register_command_mode(uint8_t cat, uint16_t addr, uint8_t reg_size) {

  uint8_t command[ISX019_I2C_READ_COMMAND_SIZE+1] = {0};
  uint16_t ret_data_size = reg_size + 5;
  uint16_t reg_value = 0;

  const uint8_t ISX019_RET_MAX_DATA = 7;
  uint8_t value[ISX019_RET_MAX_DATA+1] = {0};

  bool ret = create_isx019_read_command(cat, addr, reg_size, command);
  if (!ret) {
    printf("ERROR!!: register size error!\n");
    return 0;
  }

#ifdef DEBUG
  for (int n = 0; n < ISX019_I2C_READ_COMMAND_SIZE; ++n) {
    printf("[%02X]", command[n]);
  }
  printf("\n");
#endif

  Wire2.beginTransmission(ISX019_I2C_SLVADDR);
  Wire2.write(command, ISX019_I2C_READ_COMMAND_SIZE);
  Wire2.endTransmission(false);

  ret_data_size = reg_size + 5;
  Wire2.requestFrom(ISX019_I2C_SLVADDR, ret_data_size, true);

  for (int n = 0; n < ret_data_size; ++n) {
    value[n] = Wire2.read();
#ifdef DEBUG
    printf("[%02X]", value[n]);
#endif
  }
#ifdef DEBUG
  printf("\n");
#endif
  memcpy(&reg_value, &value[4], reg_size);
 
  return reg_value;
} 

uint16_t write_register_command_mode(uint8_t cat, uint16_t addr, uint16_t value, uint8_t reg_size) {

  uint8_t command[ISX019_I2C_WRITE_MAX_COMMAND_SIZE+1] = {0};

  bool ret = create_isx019_write_command(cat, addr, value, reg_size, command);
  if (!ret) {
    printf("ERROR!!: register size error!\n");
    return 0;
  }
  uint16_t write_command_size = 0;
  if (reg_size == sizeof(uint8_t)) {
    write_command_size = ISX019_I2C_WRITE_MAX_COMMAND_SIZE-1;
  } else if (reg_size == sizeof(uint16_t)) {
    write_command_size = ISX019_I2C_WRITE_MAX_COMMAND_SIZE;
  }
#ifdef DEBUG
  for (int n = 0; n < write_command_size; ++n) {
    printf("[%02X]", command[n]);
  }
  printf("\n");
#endif

  Wire2.beginTransmission(ISX019_I2C_SLVADDR);
  Wire2.write(command, write_command_size);
  Wire2.endTransmission();

  return reg_size;
}

// AELINE_LIMIT_F
// cat=20
// adrs ofst = 0x0090
// size = 1
// 0:off, 1:on

// AELINE_MAXSHT_LIMIT
// cat=20
// adrs ofst = 0x0094
// size = 4
// 3000us : val=0x00000BB8

// AELINE_MAXSHT_LIMIT_UNIT
// cat=20
// adrs ofst = 0x0098
// size = 1
// 3:[us] unit

// Set maximum exposure time in 100usec units.
bool setMaxExposure(int32_t exposure_time){
  uint16_t val;
  bool ret;

// Set maximum exposure time function enable
  ret = write_register_command_mode(0x14, 0x0090, 0x01, sizeof(uint8_t));
  if(!ret){
    return 1;
  }

// Set maximum exposure time unit
  ret = write_register_command_mode(0x14, 0x0098, 0x03, sizeof(uint8_t));
  if(!ret){
    return 1;
  }

  exposure_time = exposure_time * 100;
  val = exposure_time & 0xff;
  ret = write_register_command_mode(0x14, 0x0094, val, sizeof(uint8_t));
  if(!ret){
    return 1;
  }
  val = (exposure_time >> 8) & 0xff;
  ret = write_register_command_mode(0x14, 0x0095, val, sizeof(uint8_t));
  if(!ret){
    return 1;
  }
  val = (exposure_time >> 16)& 0xff;
  ret = write_register_command_mode(0x14, 0x0096, val, sizeof(uint8_t));
  if(!ret){
    return 1;
  }
  val = (exposure_time >> 24)& 0xff;
  ret = write_register_command_mode(0x14, 0x0097, val, sizeof(uint8_t));
  if(!ret){
    return 1;
  }
  Serial.print("Set maximum exposure time = ");   
  Serial.print(exposure_time);
  Serial.println("[usec]");

  return 0;
}

uint16_t setMaxExposureFlag(bool flag){
  uint16_t val;
  bool ret;
  
  val = flag;

  if( flag == false ){
    Serial.println("Set maximum exposure time function disable");   
  }else{
    Serial.println("Set maximum exposure time function enable");   
  }

// Set maximum exposure time function
  ret = write_register_command_mode(0x14, 0x0090, val, sizeof(uint8_t));
  if(!ret){
    return 1;
  }
  
  return 0;  
}

uint8_t getMaxExposureFlag(){
  uint16_t reg = 0;

  reg = read_register_command_mode(0x14, 0x0090, sizeof(uint8_t));
  if(reg == 0){
    printf("Maximum Exposure Flag = Off\n");
  }else if(reg == 1){
    printf("Maximum Exposure Flag = On\n");
  }else{
    printf("Maximum Exposure Flag = %d (Illeagal value)\n", reg);
  }
  
  return reg;
}

uint8_t getMaxExposureUnit(){
  uint16_t reg = 0;

  reg = read_register_command_mode(0x14, 0x0098, sizeof(uint8_t));
  if(reg == 3){
    printf("Maximum Exposure unit = [usec]\n");
  }else{
    printf("Maximum Exposure unit error!! val=%d\n", reg);
  }
  
  return reg; 
}

uint32_t getMaxExposure(){
  uint16_t reg = 0;
  uint32_t exposure_time = 0;

  reg = read_register_command_mode(0x14, 0x0096, sizeof(uint16_t));
  exposure_time = reg << 16;
  reg = read_register_command_mode(0x14, 0x0094, sizeof(uint16_t));
  exposure_time += reg;
  printf("Maximum Exposure time = %d[us]\n", exposure_time);
  
  return exposure_time;
}

// EVSEL
// cat=20
// adrs ofst = 0x009c
// size = 1
// -6: -2[EV], -5: -5/3[EV], -4 :-4/3[EV],
// -3: -1[EV], -2 :-2/3[EV], -1 :-1/3[EV],
// 0: 補正なし1: +1/3[EV], 2: +2/3[EV], 3: +1[EV],
// 4: +4/3[EV], 5: +5/3[EV], 6: +2[EV]

uint16_t printEV(int16_t val ){

  Serial.print("EV = ");
  switch(val){
    case -6:
      Serial.println("-2[EV]");
      break;
    case -5:
      Serial.println("-5/3[EV]");
      break;
    case -4:
      Serial.println("-4/3[EV]");
      break;
    case -3:
      Serial.println("-1[EV]");
      break;
    case -2:
      Serial.println("-2/3[EV]");
      break;
    case -1:
      Serial.println("-1/3[EV]");
      break;
    case 0:
      Serial.println("0[EV]");
      break;
    case 1:
      Serial.println("+1/3[EV]");
      break;
    case 2:
      Serial.println("+2/3[EV]");
      break;
    case 3:
      Serial.println("+1[EV]");
      break;
    case 4:
      Serial.println("+4/3[EV]");
      break;
    case 5:
      Serial.println("+5/3[EV]");
      break;
    case 6:
      Serial.println("+2[EV]");
      break;
    default:
      Serial.println("Illigal data");
  }

  return 0;
}

int16_t getExposureCompensation(){
  int16_t reg = 0;
  reg = read_register_command_mode(0x14, 0x009c, sizeof(uint8_t));
  printEV(reg);

  return reg;
}
  
uint16_t setExposureCompensation(int16_t val){
  bool ret;
  
  Serial.print(" Set ");
  printEV(val);

// Set exposure compensation
  ret = write_register_command_mode(0x14, 0x009c, val, sizeof(uint8_t));
  if(!ret){
    return 1;
  }
  
  return 0;  
}
  



  
