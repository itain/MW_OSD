#define SERIALBUFFERSIZE 256
static uint8_t serialBuffer[SERIALBUFFERSIZE]; // this hold the imcoming string from serial O string
static uint8_t receiverIndex;
static uint8_t dataSize;
static uint8_t cmdMSP;
static uint8_t rcvChecksum;
static uint8_t readIndex;
static uint8_t txCheckSum;

uint32_t read32() {
  uint32_t t = read16();
  t |= (uint32_t)read16()<<16;
  return t;
}

uint16_t read16() {
  uint16_t t = read8();
  t |= (uint16_t)read8()<<8;
  return t;
}

uint8_t read8()  {
  return serialBuffer[readIndex++];
}

void write8(uint8_t b) {
  Serial.write(b);
  txCheckSum ^= b;
}

void endSerialRequest() {
  Serial.write(txCheckSum);
}

void writeReqHeader(uint8_t size) {
  Serial.write('$');
  Serial.write('M');
  Serial.write('<');
  Serial.write(size);
  txCheckSum = size;
}

// --------------------------------------------------------------------------------------
// Here are decoded received commands from MultiWii
void serialMSPCheck()
{
  readIndex = 0;

  if (cmdMSP == MSP_OSD) {
    uint8_t cmd = read8();
    if(cmd == OSD_READ_CMD) {
      writeReqHeader(EEPROM_SETTINGS + 1);
      write8(MSP_OSD);
      write8(cmd);
      for(uint8_t i=0; i<EEPROM_SETTINGS; i++) {
        write8(Settings[i]);
      }
      endSerialRequest();
    }

    if (cmd == OSD_WRITE_CMD) {
      for(int en=0;en<EEPROM_SETTINGS; en++){
	uint8_t inSetting = read8();
	if (inSetting != Settings[en])
	  EEPROM.write(en,inSetting);
	Settings[en] = inSetting;
      }
      readEEPROM();
      setMspRequests();
    }

    if(cmd == OSD_GET_FONT) {
      if(dataSize == 5) {
        if(read16() == 7456) {
          nextCharToRequest = read8();
          lastCharToRequest = read8();
          initFontMode();
        }
      }
      else if(dataSize == 56) {
        for(uint8_t i = 0; i < 54; i++)
          fontData[i] = read8();
      
	uint8_t c = read8();

	fontCharacterReceived(c);
      }
    }
  }

  if (cmdMSP==MSP_IDENT)
  {
    MwVersion= read8();                             // MultiWii Firmware version
    modeMSPRequests &=~ REQ_MSP_IDENT;
  }

  if (cmdMSP==MSP_STATUS)
  {
    cycleTime=read16();
    I2CError=read16();
    MwSensorPresent = read16();
    MwSensorActive = read32();
    armed = (MwSensorActive & mode_armed) != 0;
  }

  if (cmdMSP==MSP_RAW_IMU)
  {
    for(uint8_t i=0;i<3;i++)
      MwAccSmooth[i] = read16();
  }

  if (cmdMSP==MSP_RC)
  {
    for(uint8_t i=0;i<8;i++)
      MwRcData[i] = read16();
    handleRawRC();
  }

  if (cmdMSP==MSP_RAW_GPS)
  {
    GPS_fix=read8();
    GPS_numSat=read8();
    GPS_latitude = read32();
    GPS_longitude = read32();
    GPS_altitude = read16();
    GPS_speed = read16();
  }

  if (cmdMSP==MSP_COMP_GPS)
  {
    GPS_distanceToHome=read16();
    GPS_directionToHome=read16();
  }

  if (cmdMSP==MSP_ATTITUDE)
  {
    for(uint8_t i=0;i<2;i++)
      MwAngle[i] = read16();
    MwHeading = read16();
    read16();
  }

  if (cmdMSP==MSP_ALTITUDE)
  {
    MwAltitude =read32();
    MwVario = read16();
  }

  if (cmdMSP==MSP_ANALOG)
  {
    MwVBat=read8();
    pMeterSum=read16();
    MwRssi = read16();
  }

  if (cmdMSP==MSP_RC_TUNING)
  {
    rcRate8 = read8();
    rcExpo8 = read8();
    rollPitchRate = read8();
    yawRate = read8();
    dynThrPID = read8();
    thrMid8 = read8();
    thrExpo8 = read8();
    modeMSPRequests &=~ REQ_MSP_RC_TUNING;
  }

  if (cmdMSP==MSP_PID)
  {
    for(uint8_t i=0; i<PIDITEMS; i++) {
      P8[i] = read8();
      I8[i] = read8();
      D8[i] = read8();
    }
    modeMSPRequests &=~ REQ_MSP_PID;
  }

#ifdef USE_BOXNAMES 
  if(cmdMSP==MSP_BOXNAMES) {
    uint32_t bit = 1;
    uint8_t remaining = dataSize;
    uint8_t len = 0;
    char firstc, lastc;

    mode_armed = 0;
    mode_stable = 0;
    mode_baro = 0;
    mode_mag = 0;
    mode_gpshome = 0;
    mode_gpshold = 0;
    mode_llights = 0;
    mode_osd_switch = 0;

    while(remaining > 0) {
      char c = read8();
      if(len == 0)
        firstc = c;
      len++;
      if(c == ';') {
        // Found end of name; set bit if first and last c matches.
        if(firstc == 'A') {
          if(lastc == 'M') // "ARM;"
            mode_armed |= bit;
          if(lastc == 'E') // "ANGLE;"
            mode_stable |= bit;
        }
        if(firstc == 'H' && lastc == 'N') // "HORIZON;"
          mode_stable |= bit;
        if(firstc == 'M' && lastc == 'G') // "MAG;"
           mode_mag |= bit;
        if(firstc == 'B' && lastc == 'O') // "BARO;"
          mode_baro |= bit;
        if(firstc == 'L' && lastc == 'S') // "LLIGHTS;"
          mode_llights |= bit;
        if(firstc == 'G') {
          if(lastc == 'E') // "GPS HOME;"
            mode_gpshome |= bit;
          if(lastc == 'D') // "GPS HOLD;"
            mode_gpshold |= bit;
        }
        if(firstc == 'O' && lastc == 'W') // "OSD SW;"
          mode_osd_switch |= bit;

        len = 0;
        bit <<= 1L;
      }
      lastc = c;
      --remaining;
    }
    modeMSPRequests &=~ REQ_MSP_BOX;
  }
#else // use MSP_BOXIDS
  if(cmdMSP==MSP_BOXIDS) {
    uint32_t bit = 1;
    uint8_t remaining = dataSize;

    mode_armed = 0;
    mode_stable = 0;
    mode_baro = 0;
    mode_mag = 0;
    mode_gpshome = 0;
    mode_gpshold = 0;
    mode_llights = 0;
    mode_osd_switch = 0;

    while(remaining > 0) {
      char c = read8();
      switch(c) {
      case 0:
        mode_armed |= bit;
        break;
      case 1:
      case 2:
        mode_stable |= bit;
        break;
      case 3:
        mode_baro |= bit;
        break;
      case 5:
        mode_mag |= bit;
        break;
      case 10:
        mode_gpshome |= bit;
        break;
      case 11:
        mode_gpshold |= bit;
        break;
      case 16:
        mode_llights |= bit;
        break;
      case 19:
        mode_osd_switch |= bit;
        break;
      }
      bit <<= 1;
      --remaining;
    }
    modeMSPRequests &=~ REQ_MSP_BOX;
  }
#endif
}
// End of decoded received commands from MultiWii

void serialMSPreceive()
{
  uint8_t c;

  static enum _serial_state {
    IDLE,
    HEADER_START,
    HEADER_M,
    HEADER_ARROW,
    HEADER_SIZE,
    HEADER_CMD,
  }
  c_state = IDLE;

  while(Serial.available())
  {
    c = Serial.read();

    if (c_state == IDLE)
    {
      c_state = (c=='$') ? HEADER_START : IDLE;
    }
    else if (c_state == HEADER_START)
    {
      c_state = (c=='M') ? HEADER_M : IDLE;
    }
    else if (c_state == HEADER_M)
    {
      c_state = (c=='>') ? HEADER_ARROW : IDLE;
    }
    else if (c_state == HEADER_ARROW)
    {
      if (c > SERIALBUFFERSIZE)
      {  // now we are expecting the payload size
        c_state = IDLE;
      }
      else
      {
        dataSize = c;
        c_state = HEADER_SIZE;
        rcvChecksum = c;
      }
    }
    else if (c_state == HEADER_SIZE)
    {
      c_state = HEADER_CMD;
      cmdMSP = c;
      rcvChecksum ^= c;
      receiverIndex=0;
    }
    else if (c_state == HEADER_CMD)
    {
      rcvChecksum ^= c;
      if(receiverIndex == dataSize) // received checksum byte
      {
        if(rcvChecksum == 0) {
            serialMSPCheck();
        }
        c_state = IDLE;
      }
      else
        serialBuffer[receiverIndex++]=c;
    }
  }
}

void blankserialRequest(uint8_t requestMSP)
{
  if(requestMSP == MSP_OSD && fontMode) {
    fontSerialRequest();
    return;
  }
  writeReqHeader(0);
  write8(requestMSP);
  endSerialRequest();
}

void fontSerialRequest() {
  int16_t cindex = getNextCharToRequest();
  writeReqHeader(3);
  write8(MSP_OSD);
  write8(OSD_GET_FONT);
  write8(cindex);
  write8(cindex>>8);
  endSerialRequest();
}
