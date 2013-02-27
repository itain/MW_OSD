// --------------------------------------------------------------------------------------

void handleRawRC() {
  static uint8_t waitStick;
  static uint32_t stickTime;
  static uint32_t timeout;

  if(MwRcData[PITCHSTICK] > 1300 && MwRcData[PITCHSTICK] < 1700 &&
     MwRcData[ROLLSTICK] > 1300 && MwRcData[ROLLSTICK] < 1700 &&
     MwRcData[YAWSTICK] > 1300 && MwRcData[YAWSTICK] < 1700) {
	waitStick = 0;
        timeout = 1000;
  }
  else if(waitStick == 1) {
    if((millis() - stickTime) > timeout)
      waitStick = 0;
      timeout = 300;
  }

  if(!waitStick)
  {
    if((MwRcData[PITCHSTICK]>MAXSTICK)&&(MwRcData[YAWSTICK]>MAXSTICK)&&(MwRcData[THROTTLESTICK]>MINSTICK)&&!configMode&&(onTime>5)&&!armed)
    {
      // Enter config mode using stick combination
      waitStick =  2;	// Sticks must return to center before continue!
      configMode = 1;
      setMspRequests();
    }
    else if(configMode) {
      if(previousarmedstatus&&(MwRcData[THROTTLESTICK]>MINSTICK))
      {
	// EXIT from SHOW STATISTICS (menu page 6) AFTER DISARM (push throttle up) (Carlonb) NEB
	waitStick = 2;
	configExit();
      }
      if(!previousarmedstatus&&configMode&&(MwRcData[THROTTLESTICK]<MINSTICK)) // EXIT NEB mod for autostatistics
      {
	waitStick = 2;
	configExit();
      }
      else if(configMode&&(MwRcData[ROLLSTICK]>MAXSTICK)) // MOVE RIGHT
      {
	waitStick = 1;
	COL++;
	if(COL>3) COL=3;
      }
      else if(configMode&&(MwRcData[ROLLSTICK]<MINSTICK)) // MOVE LEFT
      {
	waitStick = 1;
	COL--;
	if(COL<1) COL=1;
      }
      else if(configMode&&(MwRcData[PITCHSTICK]>MAXSTICK)) // MOVE UP
      {
	waitStick = 1;
	ROW--;
	if(ROW<1)
	  ROW=1;
      }
      else if(configMode&&(MwRcData[PITCHSTICK]<MINSTICK)) // MOVE DOWN
      {
	waitStick = 1;
	ROW++;
	if(ROW>10)
	  ROW=10;
      }
      else if(!previousarmedstatus&&configMode&&(MwRcData[YAWSTICK]<MINSTICK)) // DECREASE
      {
	waitStick = 1;

	if(configPage == 1) {
	  if(ROW >= 1 && ROW <= 5) {
	    if(COL==1) P8[ROW-1]--;
	    if(COL==2) I8[ROW-1]--;
	    if(COL==3) D8[ROW-1]--;
	  }

	  if(ROW == 6) {
	    if(COL==1) P8[7]--;
	    if(COL==2) I8[7]--;
	    if(COL==3) D8[7]--;
	  }

	  if((ROW==7)&&(COL==1)) P8[8]--;
	}

	if(configPage == 2 && COL == 3) {
	  if(ROW==1) rcRate8--;
	  if(ROW==2) rcExpo8--;
	  if(ROW==3) rollPitchRate--;
	  if(ROW==4) yawRate--;
	  if(ROW==5) dynThrPID--;
	}

	if(configPage == 3 && COL == 3) {
	  if(ROW==2) Settings[S_DISPLAYVOLTAGE]=!Settings[S_DISPLAYVOLTAGE];
	  if(ROW==3) Settings[S_VOLTAGEMIN]--;
	  if(ROW==4) Settings[S_DISPLAYTEMPERATURE]=!Settings[S_DISPLAYTEMPERATURE];
	  if(ROW==5) Settings[S_TEMPERATUREMAX]--;
	  if(ROW==6) Settings[S_DISPLAYGPS]=!Settings[S_DISPLAYGPS];
          if(ROW==7) Settings[S_COORDINATES]=!Settings[S_COORDINATES];
	}

	if(configPage == 4 && COL == 3) {
	  if(ROW==3) rssiTimer=15;
	  if(ROW==4) Settings[S_RSSIMAX]=rssiADC;
	  if(ROW==5) Settings[S_DISPLAYRSSI]=!Settings[S_DISPLAYRSSI];
	  if(ROW==6) Settings[S_UNITSYSTEM]=!Settings[S_UNITSYSTEM];
	  if(ROW==7) {
	    Settings[S_VIDEOSIGNALTYPE]=!Settings[S_VIDEOSIGNALTYPE];
	    MAX7456Setup();
	  }
	}

	if(configPage == 5 && COL == 3) {
	  if(ROW==1) accCalibrationTimer=0;
	  if(ROW==5) magCalibrationTimer=0;
	  if(ROW==7) eepromWriteTimer=0;
	}

	if((ROW==10)&&(COL==3)) configPage--;
	if(configPage<MINPAGE) configPage = MAXPAGE;
	if((ROW==10)&&(COL==1)) configExit();
	if((ROW==10)&&(COL==2)) saveExit();
      }
      else if(!previousarmedstatus&&configMode&&(MwRcData[YAWSTICK]>MAXSTICK)) // INCREASE
      {
	waitStick =1;

	if(configPage == 1) {
	  if(ROW >= 1 && ROW <= 5) {
	    if(COL==1) P8[ROW-1]++;
	    if(COL==2) I8[ROW-1]++;
	    if(COL==3) D8[ROW-1]++;
	  }

	  if(ROW == 6) {
	    if(COL==1) P8[7]++;
	    if(COL==2) I8[7]++;
	    if(COL==3) D8[7]++;
	  }

	  if((ROW==7)&&(COL==1)) P8[8]++;
	}

	if(configPage == 2 && COL == 3) {
	  if(ROW==1) rcRate8++;
	  if(ROW==2) rcExpo8++;
	  if(ROW==3) rollPitchRate++;
	  if(ROW==4) yawRate++;
	  if(ROW==5) dynThrPID++;
	}

	if(configPage == 3 && COL == 3) {
	  if(ROW==2) Settings[S_DISPLAYVOLTAGE]=!Settings[S_DISPLAYVOLTAGE];
	  if(ROW==3) Settings[S_VOLTAGEMIN]++;
	  if(ROW==4) Settings[S_DISPLAYTEMPERATURE]=!Settings[S_DISPLAYTEMPERATURE];
	  if(ROW==5) Settings[S_TEMPERATUREMAX]++;
	  if(ROW==6) Settings[S_DISPLAYGPS]=!Settings[S_DISPLAYGPS];
          if(ROW==7) Settings[S_COORDINATES]=!Settings[S_COORDINATES];
	}

	if(configPage == 4 && COL == 3) {
	  if(ROW==3) rssiTimer=15;
	  if(ROW==4) Settings[S_RSSIMAX]=rssiADC;
	  if(ROW==5) Settings[S_DISPLAYRSSI]=!Settings[S_DISPLAYRSSI];
	  if(ROW==6) Settings[S_UNITSYSTEM]=!Settings[S_UNITSYSTEM];
	  if(ROW==7) {
	    Settings[S_VIDEOSIGNALTYPE]=!Settings[S_VIDEOSIGNALTYPE];
	    MAX7456Setup();
	  }
	}

	if(configPage == 5 && COL == 3) {
	  if(ROW==1) accCalibrationTimer=CALIBRATION_DELAY;
	  if(ROW==5) magCalibrationTimer=CALIBRATION_DELAY;
	  if(ROW==7) eepromWriteTimer=EEPROM_WRITE_DELAY;
	}

	if((ROW==10)&&(COL==3)) configPage++;
	if(configPage>MAXPAGE) configPage = MINPAGE;
	if((ROW==10)&&(COL==1)) configExit();
	if((ROW==10)&&(COL==2)) saveExit();
      }
    }

    if(waitStick == 1)
      stickTime = millis();
  }
}

void configExit()
{
  configPage=1;
  ROW=10;
  COL=3;
  configMode=0;
  //waitStick=3;
  previousarmedstatus = 0;
  if (Settings[S_RESETSTATISTICS]){  // NEB added for reset statistics if defined
    trip=0;
    distanceMAX=0;
    altitudeMAX=0;
    speedMAX=0;
    temperMAX = -128;
    flyingTime=0;
  }
  setMspRequests();
}

void saveExit()
{
  if (configPage==1){
    writeReqHeader(30);
    write8(MSP_SET_PID);
    for(uint8_t i=0; i<PIDITEMS; i++) {
      write8(P8[i]);
      write8(I8[i]);
      write8(D8[i]);
    }
    endSerialRequest();
  }

  if (configPage==2){
    writeReqHeader(7);
    write8(MSP_SET_RC_TUNING);
    write8(rcRate8);
    write8(rcExpo8);
    write8(rollPitchRate);
    write8(yawRate);
    write8(dynThrPID);
    write8(thrMid8);
    write8(thrExpo8);
    endSerialRequest();
  }

  if (configPage==3 || configPage==4){
    writeEEPROM();
  }
  configExit();
}

void displayCursor(void)
{
  int cursorpos;

  if(ROW==10) {
    if(COL==3) cursorpos=SAVEP+16-1;    // page
    if(COL==1) cursorpos=SAVEP-1;       // exit
    if(COL==2) cursorpos=SAVEP+6-1;     // save/exit
  }
  else {
    if(configPage==1){
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if(COL==1) cursorpos=(ROW+2)*30+10;
      if(COL==2) cursorpos=(ROW+2)*30+10+6;
      if(COL==3) cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==2){
      COL=3;
      if (ROW==7) ROW=5;
      if (ROW==6) ROW=10;
      if (ROW==9) ROW=5;
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==3){
      COL=3;
      if (ROW==1) ROW=2;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==4){
      COL=3;
      if (ROW==2) ROW=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      if ((ROW==6)||(ROW==7))
        cursorpos=(ROW+2)*30+10+6+6-2;  // Narrow/Imperial strings longer
      else
        cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==5){
      COL=3;
      if (ROW==9) ROW=7;
      if (ROW==8) ROW=10;
      cursorpos=(ROW+2)*30+10+6+6;
      }
    if(configPage==6){
      ROW=10;
      }
  }
  if(Blink10hz)
    screen[cursorpos] = SYM_CURSOR;
}

void displayConfigScreen(void)
{
  MAX7456_WriteString_P(configMsg0, SAVEP);    //EXIT
  if(!previousarmedstatus) {
    MAX7456_WriteString_P(configMsg1, SAVEP+6);  //SaveExit
    MAX7456_WriteString_P(configMsg2, SAVEP+16); //<Page>
  }

  if(configPage==1)
  {
    MAX7456_WriteString_P(configMsg3, 38);
    MAX7456_WriteString_P(configMsg4, ROLLT);
    MAX7456_WriteString(itoa(P8[0],screenBuffer,10),ROLLP);
    MAX7456_WriteString(itoa(I8[0],screenBuffer,10),ROLLI);
    MAX7456_WriteString(itoa(D8[0],screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg5, PITCHT);
    MAX7456_WriteString(itoa(P8[1],screenBuffer,10), PITCHP);
    MAX7456_WriteString(itoa(I8[1],screenBuffer,10), PITCHI);
    MAX7456_WriteString(itoa(D8[1],screenBuffer,10), PITCHD);

    MAX7456_WriteString_P(configMsg6, YAWT);
    MAX7456_WriteString(itoa(P8[2],screenBuffer,10),YAWP);
    MAX7456_WriteString(itoa(I8[2],screenBuffer,10),YAWI);
    MAX7456_WriteString(itoa(D8[2],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg7, ALTT);
    MAX7456_WriteString(itoa(P8[3],screenBuffer,10),ALTP);
    MAX7456_WriteString(itoa(I8[3],screenBuffer,10),ALTI);
    MAX7456_WriteString(itoa(D8[3],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg8, VELT);
    MAX7456_WriteString(itoa(P8[4],screenBuffer,10),VELP);
    MAX7456_WriteString(itoa(I8[4],screenBuffer,10),VELI);
    MAX7456_WriteString(itoa(D8[4],screenBuffer,10),VELD);

    MAX7456_WriteString_P(configMsg9, LEVT);
    MAX7456_WriteString(itoa(P8[7],screenBuffer,10),LEVP);
    MAX7456_WriteString(itoa(I8[7],screenBuffer,10),LEVI);
    MAX7456_WriteString(itoa(D8[7],screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg10, MAGT);
    MAX7456_WriteString(itoa(P8[8],screenBuffer,10),MAGP);

    MAX7456_WriteString("P",71);
    MAX7456_WriteString("I",77);
    MAX7456_WriteString("D",83);
  }

  if(configPage==2)
  {
    MAX7456_WriteString_P(configMsg11, 38);
    MAX7456_WriteString_P(configMsg12, ROLLT);
    MAX7456_WriteString(itoa(rcRate8,screenBuffer,10),ROLLD);
    MAX7456_WriteString_P(configMsg13, PITCHT);
    MAX7456_WriteString(itoa(rcExpo8,screenBuffer,10),PITCHD);
    MAX7456_WriteString_P(configMsg14, YAWT);
    MAX7456_WriteString(itoa(rollPitchRate,screenBuffer,10),YAWD);
    MAX7456_WriteString_P(configMsg15, ALTT);
    MAX7456_WriteString(itoa(yawRate,screenBuffer,10),ALTD);
    MAX7456_WriteString_P(configMsg16, VELT);
    MAX7456_WriteString(itoa(dynThrPID,screenBuffer,10),VELD);
    MAX7456_WriteString_P(configMsg17, LEVT);
    MAX7456_WriteString(itoa(cycleTime,screenBuffer,10),LEVD);
    MAX7456_WriteString_P(configMsg18, MAGT);
    MAX7456_WriteString(itoa(I2CError,screenBuffer,10),MAGD);
  }

  if(configPage==3)
  {
    MAX7456_WriteString_P(configMsg19, 35);
    MAX7456_WriteString_P(configMsg23, PITCHT);
    if(Settings[S_DISPLAYVOLTAGE]){
      MAX7456_WriteString_P(configMsg21, PITCHD);
    }
    else {
      MAX7456_WriteString_P(configMsg22, PITCHD);
    }
    MAX7456_WriteString_P(configMsg24, YAWT);
    MAX7456_WriteString(itoa(Settings[S_VOLTAGEMIN],screenBuffer,10),YAWD);
    MAX7456_WriteString_P(configMsg25, ALTT);

    if(Settings[S_DISPLAYTEMPERATURE] ){
      MAX7456_WriteString_P(configMsg21, ALTD);
    }
    else {
      MAX7456_WriteString_P(configMsg22, ALTD);
    }
    MAX7456_WriteString_P(configMsg26, VELT);
    MAX7456_WriteString(itoa(Settings[S_TEMPERATUREMAX],screenBuffer,10),VELD);
    MAX7456_WriteString_P(configMsg27, LEVT);

    if(Settings[S_DISPLAYGPS]){
      MAX7456_WriteString_P(configMsg21, LEVD);
     }
     else {
      MAX7456_WriteString_P(configMsg22, LEVD);
    }
    MAX7456_WriteString_P(configMsg28, MAGT);
    if(Settings[S_COORDINATES]){
      MAX7456_WriteString_P(configMsg21, MAGD);
     }
     else {
      MAX7456_WriteString_P(configMsg22, MAGD);
    }
  }

  if(configPage==4)
  {
    MAX7456_WriteString_P(configMsg31, 39);

    MAX7456_WriteString_P(configMsg32, ROLLT);
    MAX7456_WriteString(itoa(rssiADC,screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg33, PITCHT);
    MAX7456_WriteString(itoa(rssi,screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg34, YAWT);
    if(rssiTimer>0) MAX7456_WriteString(itoa(rssiTimer,screenBuffer,10),YAWD-5);
    MAX7456_WriteString(itoa(Settings[S_RSSIMIN],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg35, ALTT);
    MAX7456_WriteString(itoa(Settings[S_RSSIMAX],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg36, VELT);
    MAX7456_WriteString_P(Settings[S_DISPLAYRSSI] ? configMsg21 : configMsg22, VELD);

    MAX7456_WriteString_P(configMsg37, LEVT);
    MAX7456_WriteString_P( Settings[S_UNITSYSTEM] ? configMsg38 : configMsg39, LEVD-2);

    MAX7456_WriteString_P(configMsg40, MAGT);
    MAX7456_WriteString_P(Settings[S_VIDEOSIGNALTYPE] ? configMsg42 : configMsg41, MAGD+1);
  }

  if(configPage==5)
  {
    MAX7456_WriteString_P(configMsg43, 37);

    MAX7456_WriteString_P(configMsg44, ROLLT);
    if(accCalibrationTimer>0)
      MAX7456_WriteString(itoa(accCalibrationTimer,screenBuffer,10),ROLLD);
    else
      MAX7456_WriteString("-",ROLLD);

    MAX7456_WriteString_P(configMsg45, PITCHT);
    MAX7456_WriteString(itoa(MwAccSmooth[0],screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg46, YAWT);
    MAX7456_WriteString(itoa(MwAccSmooth[1],screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg47, ALTT);
    MAX7456_WriteString(itoa(MwAccSmooth[2],screenBuffer,10),ALTD);

    MAX7456_WriteString_P(configMsg48, VELT);
    if(magCalibrationTimer>0)
      MAX7456_WriteString(itoa(magCalibrationTimer,screenBuffer,10),VELD);
    else
      MAX7456_WriteString("-",VELD);

    MAX7456_WriteString_P(configMsg49, LEVT);
    MAX7456_WriteString(itoa(MwHeading,screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg50, MAGT);
    if(eepromWriteTimer>0)
      MAX7456_WriteString(itoa(eepromWriteTimer,screenBuffer,10),MAGD);
    else
      MAX7456_WriteString("-",MAGD);
  }

  if(configPage==6)
  {
    int xx;
    MAX7456_WriteString_P(configMsg51, 38);

    MAX7456_WriteString_P(configMsg52, ROLLT);
    MAX7456_WriteString(itoa(MToMf(trip),screenBuffer,10),ROLLD);

    MAX7456_WriteString_P(configMsg53, PITCHT);
    MAX7456_WriteString(itoa(MToM(distanceMAX), screenBuffer,10),PITCHD);

    MAX7456_WriteString_P(configMsg54, YAWT);
    MAX7456_WriteString(itoa(CmToM(altitudeMAX), screenBuffer,10),YAWD);

    MAX7456_WriteString_P(configMsg55, ALTT);
    MAX7456_WriteString(itoa(CMsToKMh(speedMAX), screenBuffer, 10), ALTD);

    MAX7456_WriteString_P(configMsg56, VELT);

    formatTime(flyingTime, screenBuffer, 1);
    MAX7456_WriteString(screenBuffer,VELD-7);

    MAX7456_WriteString_P(configMsg57, LEVT);
    xx= pMeterSum / EST_PMSum;
    MAX7456_WriteString(itoa(xx,screenBuffer,10),LEVD);

    MAX7456_WriteString_P(configMsg58, MAGT);
    MAX7456_WriteString(itoa(TempConverter(temperMAX), screenBuffer, 10), MAGD);
  }
  displayCursor();
}
