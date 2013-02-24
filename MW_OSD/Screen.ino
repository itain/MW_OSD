
char *ItoaPadded(int val, char *str, uint8_t bytes, uint8_t decimalpos)  {
  uint8_t neg = 0;
  if(val < 0) {
    neg = 1;
    val = -val;
  }

  str[bytes] = 0;
  for(;;) {
    if(bytes == decimalpos) {
      str[--bytes] = DECIMAL;
      decimalpos = 0;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes == 0 || (decimalpos == 0 && val == 0))
      break;
  }

  if(neg && bytes > 0)
    str[--bytes] = '-';

  while(bytes != 0)
    str[--bytes] = ' ';
  return str;
}

char *FormatGPSCoord(int32_t val, char *str, uint8_t p, char pos, char neg) {
  if(val < 0) {
    pos = neg;
    val = -val;
  }

  uint8_t bytes = p+8;

  str[bytes] = 0;
  str[--bytes] = pos;
  for(;;) {
    if(bytes == p) {
      str[--bytes] = DECIMAL;
      continue;
    }
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    if(bytes < 3 && val == 0)
       break;
   }

   while(bytes != 0)
     str[--bytes] = ' ';

   return str;
}

// Take time in Seconds and format it as 'MM:SS'
// Alternately Take time in Minutes and format it as 'HH:MM'
// If hhmmss is 1, display as HH:MM:SS
char *formatTime(uint16_t val, char *str, uint8_t hhmmss) {
  int8_t bytes = 5;
  if(hhmmss)
    bytes = 8;
  str[bytes] = 0;
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
    str[--bytes] = '0' + (val % 6);
    val = val / 6;
    str[--bytes] = ':';
  } while(hhmmss-- != 0);
  do {
    str[--bytes] = '0' + (val % 10);
    val = val / 10;
  } while(val != 0 && bytes != 0);

  while(bytes != 0)
     str[--bytes] = ' ';

  return str;
}

uint8_t FindNull(void)
{
  uint8_t xx;
  for(xx=0;screenBuffer[xx]!=0;xx++)
    ;
  return xx;
}

// Unit conversions
uint8_t CMsToKMh(uint16_t speed)       // cm/sec to km/h or to mph
{
  return GPS_speed *
           (Settings[S_UNITSYSTEM] ?
               (0.036*0.62137) :       // From MWii cm/sec to mph
               0.036);                 // From MWii cm/sec to Km/h
}

int16_t TempConverter(int16_t temp) { // deg-C to deg-C or deg-F
  return Settings[S_UNITSYSTEM] ?
           temp * 1.8 + 32 :       // convert to Fahrenheit
           temp;
}

int16_t CmToM(int32_t alt) {
  return alt *
           (Settings[S_UNITSYSTEM] ?
               0.032808 :
               0.01);
}

int16_t MToM(int16_t dist) {
  return (Settings[S_UNITSYSTEM] ?
             (dist * 3.2808) :
             dist);
}

int32_t MToMf(float dist) {
  return (Settings[S_UNITSYSTEM] ?
             (dist * 3.2808) :
             dist);
}

void displayTemperature(void)        // WILL WORK ONLY WITH V1.2
{
  itoa(TempConverter(temperature), screenBuffer, 10);
  uint8_t xx = FindNull();
  screenBuffer[xx++] = Settings[S_UNITSYSTEM] ? SYM_TEMP_F : SYM_TEMP_C;
  screenBuffer[xx] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(temperaturePosition));
}

void displayMode(void)
{
  // Put sensor symbold (was displaySensors)
  screenBuffer[0] = (MwSensorPresent&ACCELEROMETER) ? SYM_ACC : ' ';
  screenBuffer[1] = (MwSensorPresent&BAROMETER) ? SYM_BAR : ' ';
  screenBuffer[2] = (MwSensorPresent&MAGNETOMETER) ? SYM_MAG : ' ';
  screenBuffer[3] = (MwSensorPresent&GPSSENSOR) ? SYM_GPS : ' ';

  if(MwSensorActive&mode_stable)
  {
    screenBuffer[4]=SYM_STABLE;
    screenBuffer[5]=SYM_STABLE1;
  }
  else
  {
    screenBuffer[4]=SYM_ACRO;
    screenBuffer[5]=SYM_ACRO1;
  }
  screenBuffer[6]=' ';
  if(MwSensorActive&mode_gpshome)
    screenBuffer[7]=SYM_G_HOME;
  else if(MwSensorActive&mode_gpshold)
    screenBuffer[7]=SYM_HOLD;
  else if(GPS_fix)
    screenBuffer[7]=SYM_3DFIX;
  else
    screenBuffer[7]=' ';

  screenBuffer[8]=0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition));

  // Put ON indicator under sensor symbol
  screenBuffer[0] = (MwSensorActive&mode_stable) ? SYM_CHECK : ' ';
  screenBuffer[1] = (MwSensorActive&mode_baro) ? SYM_CHECK : ' ';
  screenBuffer[2] = (MwSensorActive&mode_mag) ? SYM_CHECK : ' ';
  screenBuffer[3] = (MwSensorActive&(mode_gpshome|mode_gpshome)) ? SYM_CHECK : ' ';
  screenBuffer[4] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition)+LINE);

/*
  if(MwSensorActive & mode_llights)
    screenBuffer[0] = 0x04;
  else
    screenBuffer[0] = ' ';

  if(MwSensorActive & mode_osd_switch)
    screenBuffer[1] = 0x05;
  else
    screenBuffer[1] = ' ';
  screenBuffer[2]=0;
  MAX7456_WriteString(screenBuffer,getPosition(sensorPosition)+2*LINE);
*/
}

void displayArmed(void)
{
  if(!armed)
    MAX7456_WriteString_P(disarmed_text, getPosition(motorArmedPosition));
  else if(Blink10hz && flyTime < 9)
    MAX7456_WriteString_P(armed_text, getPosition(motorArmedPosition));
}

void displayHorizon(int rollAngle, int pitchAngle)
{
  if(!fieldIsVisible(horizonPosition))
    return;

  uint16_t position = getPosition(horizonPosition);

  if(pitchAngle>200) pitchAngle=200;
  if(pitchAngle<-250) pitchAngle=-250;
  if(rollAngle>400) rollAngle=400;
  if(rollAngle<-400) rollAngle=-400;

  for(int X=0; X<=8; X++) {
    int Y = (rollAngle * (4-X)) / 64;
    Y -= pitchAngle / 8;
    Y += 41;
    if(Y >= 0 && Y <= 81) {
      uint16_t pos = position + LINE*(Y/9) + 2 - 2*LINE + X;
      screen[pos] = SYM_AH_BAR9_0+(Y%9);
      if(Y>=9 && (Y%9) == 0)
        screen[pos-LINE] = SYM_AH_BAR9_9;
    }
  }

  if(Settings[S_DISPLAY_HORIZON_BR]){
    //Draw center screen
    screen[position+2*LINE+6-1] = SYM_AH_CENTER_LINE;
    screen[position+2*LINE+6+1] = SYM_AH_CENTER_LINE;
    screen[position+2*LINE+6] =   SYM_AH_CENTER;
  }
  if (Settings[S_WITHDECORATION]){
    // Draw AH sides
    screen[position+2*LINE+1] =   SYM_AH_LEFT;
    screen[position+2*LINE+11] =  SYM_AH_RIGHT;
    screen[position+0*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+1*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+2*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+3*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+4*LINE] =     SYM_AH_DECORATION_LEFT;
    screen[position+0*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+1*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+2*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+3*LINE+12] =  SYM_AH_DECORATION_RIGHT;
    screen[position+4*LINE+12] = SYM_AH_DECORATION_RIGHT;
  }
}

void displayVoltage(void)
{
  if (Settings[S_VIDVOLTAGE_VBAT]) {
    vidvoltage=MwVBat;
  }
  if (Settings[S_MAINVOLTAGE_VBAT]) {
    voltage=MwVBat;
  }

  if (Settings[S_SHOWBATLEVELEVOLUTION]) {
    // For battery evolution display
    int BATTEV1 =Settings[S_BATCELLS] * 35;
    int BATTEV2 =Settings[S_BATCELLS] * 36;
    int BATTEV3 =Settings[S_BATCELLS] * 37;
    int BATTEV4 =Settings[S_BATCELLS] * 38;
    int BATTEV5 =Settings[S_BATCELLS] * 40;
    int BATTEV6 = Settings[S_BATCELLS] * 41;

    if (voltage < BATTEV1) screenBuffer[0]=SYM_BATT_EMPTY;
    else if (voltage < BATTEV2) screenBuffer[0]=SYM_BATT_1;
    else if (voltage < BATTEV3) screenBuffer[0]=SYM_BATT_2;
    else if (voltage < BATTEV4) screenBuffer[0]=SYM_BATT_3;
    else if (voltage < BATTEV5) screenBuffer[0]=SYM_BATT_4;
    else if (voltage < BATTEV6) screenBuffer[0]=SYM_BATT_5;
    else screenBuffer[0]=SYM_BATT_FULL;                              // Max charge icon
  }
  else {
    screenBuffer[0]=SYM_MAIN_BATT;
  }
  ItoaPadded(voltage, screenBuffer+1, 4, 3);
  screenBuffer[5] = SYM_VOLT;
  screenBuffer[6] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(voltagePosition));

  if (Settings[S_VIDVOLTAGE]){
    screenBuffer[0]=SYM_VID_BAT;
    ItoaPadded(vidvoltage, screenBuffer+1, 4, 3);
    screenBuffer[5]=SYM_VOLT;
    screenBuffer[6]=0;
    MAX7456_WriteString(screenBuffer,getPosition(vidvoltagePosition));
  }
}

void displayCurrentThrottle(void)
{

  if (MwRcData[THROTTLESTICK] > HighT) HighT = MwRcData[THROTTLESTICK] -5;
  if (MwRcData[THROTTLESTICK] < LowT) LowT = MwRcData[THROTTLESTICK];      // Calibrate high and low throttle settings  --defaults set in GlobalVariables.h 1100-1900
  screenBuffer[0]=SYM_THR;
  screenBuffer[1]=0;
  MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition));
  if(!armed) {
    screenBuffer[0]='-';
    screenBuffer[1]='-';
    screenBuffer[2]='-';
    screenBuffer[3]='-';
    screenBuffer[4]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+2);
  }
  else
  {
    int CurThrottle = map(MwRcData[THROTTLESTICK],LowT,HighT,0,100);
    ItoaPadded(CurThrottle,screenBuffer,3,0);
    screenBuffer[3]='%';
    screenBuffer[4]=0;
    MAX7456_WriteString(screenBuffer,getPosition(CurrentThrottlePosition)+2);
  }
}

void displayTime(void)
{
  if(armed) {
    if(flyTime < 3600) {
      screenBuffer[0] = SYM_FLY_M;
      formatTime(flyTime, screenBuffer+1, 0);
    }
    else {
      screenBuffer[0] = SYM_FLY_H;
      formatTime(flyTime/60, screenBuffer+1, 0);
    }
  }
  else {
    if(onTime < 3600) {
      screenBuffer[0] = SYM_ON_M;
      formatTime(onTime, screenBuffer+1, 0);
    }
    else {
      screenBuffer[0] = SYM_ON_H;
      formatTime(onTime/60, screenBuffer+1, 0);
    }
  }
  MAX7456_WriteString(screenBuffer,getPosition(timePosition));
}

void displayAmperage(void)
{
  // Real Ampere is ampere / 10
  ItoaPadded(amperage, screenBuffer, 4, 3);     // 99.9 ampere max!
  screenBuffer[4] = SYM_AMP;
  screenBuffer[5] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(amperagePosition));
}

void displaypMeterSum(void)
{
  if (Settings[S_ENABLEADC]){
    pMeterSum = amperagesum;
  }
  screenBuffer[0]=SYM_MAH;
  int xx = pMeterSum / EST_PMSum;
  itoa(xx,screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(pMeterSumPosition));
}

void displayRSSI(void)
{
  screenBuffer[0] = SYM_RSSI;
  // Calcul et affichage du Rssi
  itoa(rssi,screenBuffer+1,10);
  uint8_t xx = FindNull();
  screenBuffer[xx++] = '%';
  screenBuffer[xx] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(rssiPosition));
}

void displayHeading(void)
{
  int16_t heading = MwHeading;
  if (Settings[S_HEADING360]) {
    if(heading < 0)
      heading += 360;
    ItoaPadded(heading,screenBuffer,3,0);
    screenBuffer[3]=SYM_DEGREES;
    screenBuffer[4]=0;
  }
  else {
    ItoaPadded(heading,screenBuffer,4,0);
    screenBuffer[4]=SYM_DEGREES;
    screenBuffer[5]=0;
  }
  MAX7456_WriteString(screenBuffer,getPosition(MwHeadingPosition));
}

void displayHeadingGraph(void)
{
  int xx;
  xx = MwHeading * 4;
  xx = xx + 720 + 45;
  xx = xx / 90;

  uint16_t pos = getPosition(MwHeadingGraphPosition);
  memcpy_P(screen+pos, headGraph+xx, 10);
}

void displayIntro(void)
{

  MAX7456_WriteString_P(message0, RushduinoVersionPosition);

  if (Settings[S_VIDEOSIGNALTYPE])
    MAX7456_WriteString_P(message2, RushduinoVersionPosition+30);
  else
    MAX7456_WriteString_P(message1, RushduinoVersionPosition+30);

  MAX7456_WriteString_P(MultiWiiLogoL1Add, RushduinoVersionPosition+120);
  MAX7456_WriteString_P(MultiWiiLogoL2Add, RushduinoVersionPosition+120+LINE);
  MAX7456_WriteString_P(MultiWiiLogoL3Add, RushduinoVersionPosition+120+LINE+LINE);

  MAX7456_WriteString_P(message5, RushduinoVersionPosition+120+LINE+LINE+LINE);
  MAX7456_WriteString(itoa(MwVersion,screenBuffer,10),RushduinoVersionPosition+131+LINE+LINE+LINE);

  MAX7456_WriteString_P(message6, RushduinoVersionPosition+120+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P(message7, RushduinoVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE);
  MAX7456_WriteString_P(message8, RushduinoVersionPosition+125+LINE+LINE+LINE+LINE+LINE+LINE+LINE);
}

void displayFontScreen(void) {
  MAX7456_WriteString_P(PSTR("UPLOADING FONT"), 35);
  MAX7456_WriteString(itoa(nextCharToRequest, screenBuffer, 10), 51);

  for(uint16_t i = 0; i < 256; i++)
    screen[90+i] = i;
}

void displayGPSPosition(void)
{
  if(!GPS_fix)
    return;

  if(Settings[S_COORDINATES]){
    if(fieldIsVisible(MwGPSLatPosition)) {
      screenBuffer[0] = SYM_LAT;
      FormatGPSCoord(GPS_latitude,screenBuffer+1,3,'N','S');
      MAX7456_WriteString(screenBuffer,getPosition(MwGPSLatPosition));
    }

    if(fieldIsVisible(MwGPSLatPosition)) {
      screenBuffer[0] = SYM_LON;
      FormatGPSCoord(GPS_longitude,screenBuffer+1,4,'E','W');
      MAX7456_WriteString(screenBuffer,getPosition(MwGPSLonPosition));
    }
  }

  screenBuffer[0] = Settings[S_UNITSYSTEM] ? SYM_ALTFT : SYM_ALTM;
  itoa(MToM(GPS_altitude), screenBuffer+1, 10);
  MAX7456_WriteString(screenBuffer,getPosition(MwGPSAltPosition));
}

void displayNumberOfSat(void)
{
  screenBuffer[0] = SYM_SAT_L;
  screenBuffer[1] = SYM_SAT_R;
  itoa(GPS_numSat,screenBuffer+2,10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_numSatPosition));
}

void displayGPS_speed(void)
{
  if(!GPS_fix)
    return;

  screenBuffer[0] = Settings[S_UNITSYSTEM] ? SYM_MPH : SYM_KMH;
  itoa(CMsToKMh(GPS_speed), screenBuffer+1, 10);
  MAX7456_WriteString(screenBuffer,getPosition(speedPosition));
}

void displayAltitude(void)
{
  screenBuffer[0]=Settings[S_UNITSYSTEM] ? SYM_ALTFT : SYM_ALTM;
  itoa(CmToM(MwAltitude),screenBuffer+1,10);
  MAX7456_WriteString(screenBuffer,getPosition(MwAltitudePosition));
}

void displayClimbRate(void)
{
  if(MwVario > 70)       screenBuffer[0] = SYM_POS_CLIMB3;
  else if(MwVario > 50)  screenBuffer[0] = SYM_POS_CLIMB2;
  else if(MwVario > 30)  screenBuffer[0] = SYM_POS_CLIMB1;
  else if(MwVario > 20)  screenBuffer[0] = SYM_POS_CLIMB;
  else if(MwVario < -70) screenBuffer[0] = SYM_NEG_CLIMB3;
  else if(MwVario < -50) screenBuffer[0] = SYM_NEG_CLIMB2;
  else if(MwVario < -30) screenBuffer[0] = SYM_NEG_CLIMB1;
  else if(MwVario < -20) screenBuffer[0] = SYM_NEG_CLIMB;
  else                   screenBuffer[0] = SYM_ZERO_CLIMB;

  screenBuffer[1] = Settings[S_UNITSYSTEM] ? SYM_FTS : SYM_MS;
  itoa(CmToM(MwVario), screenBuffer+2, 10);

  MAX7456_WriteString(screenBuffer,getPosition(MwClimbRatePosition));
}

void displayDistanceToHome(void)
{
  if(!GPS_fix)
    return;

  screenBuffer[0] = Settings[S_UNITSYSTEM] ? SYM_DISTHOME_FT : SYM_DISTHOME_M;
  itoa(MToM(GPS_distanceToHome), screenBuffer+1, 10);
  MAX7456_WriteString(screenBuffer,getPosition(GPS_distanceToHomePosition));
}

void displayAngleToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;

  ItoaPadded(GPS_directionToHome,screenBuffer,3,0);
  screenBuffer[3] = SYM_DEGREES;
  screenBuffer[4] = 0;
  MAX7456_WriteString(screenBuffer,getPosition(GPS_angleToHomePosition));
}

void displayDirectionToHome(void)
{
  if(!GPS_fix)
    return;
  if(GPS_distanceToHome <= 2 && Blink2hz)
    return;

  int16_t d = MwHeading + 180 + 360 - GPS_directionToHome;
  d *= 4;
  d += 45;
  d = (d/90)%16;

  screenBuffer[0] = SYM_ARROW_SOUTH + d;
  //screenBuffer[1] = 0x81 + d;
  screenBuffer[1]=0;                //2
  MAX7456_WriteString(screenBuffer,getPosition(GPS_directionToHomePosition));
}
