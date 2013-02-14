
public boolean toggleRead = false,
        toggleMSP_Data = true,
        toggleReset = false,
        toggleCalibAcc = false,
        toggleCalibMag = false,
        toggleWrite = false,
        toggleSpekBind = false,
        toggleSetSetting = false;
Serial g_serial;      // The serial port
int FilePercent = 0;
float LastPort = 0;
/******************************* Multiwii Serial Protocol **********************/

String boxnames[] = { // names for dynamic generation of config GUI
    "ANGLE;",
    "HORIZON;",
    "BARO;",
    "MAG;",
    "ARM;",
    "LLIGHTS;",
    "GPS HOME;",
    "GPS HOLD;",
    "OSD SW;",
    
  };
String strBoxNames = join(boxnames,""); 
//int modebits = 0;

private static final String MSP_HEADER = "$M<";

private static final int
  MSP_IDENT                =100,
  MSP_STATUS               =101,
  MSP_RAW_IMU              =102,
  MSP_SERVO                =103,
  MSP_MOTOR                =104,
  MSP_RC                   =105,
  MSP_RAW_GPS              =106,
  MSP_COMP_GPS             =107,
  MSP_ATTITUDE             =108,
  MSP_ALTITUDE             =109,
  MSP_ANALOG               =110,
  MSP_RC_TUNING            =111,
  MSP_PID                  =112,
  MSP_BOX                  =113,
  MSP_MISC                 =114,
  MSP_MOTOR_PINS           =115,
  MSP_BOXNAMES             =116,
  MSP_PIDNAMES             =117,
  MSP_BOXIDS               =119,
  MSP_RSSI                 =120,
  MSP_SET_RAW_RC           =200,
  MSP_SET_RAW_GPS          =201,
  MSP_SET_PID              =202,
  MSP_SET_BOX              =203,
  MSP_SET_RC_TUNING        =204,
  MSP_ACC_CALIBRATION      =205,
  MSP_MAG_CALIBRATION      =206,
  MSP_SET_MISC             =207,
  MSP_RESET_CONF           =208,
  MSP_SELECT_SETTING       =210,
  MSP_SPEK_BIND            =240,

  MSP_EEPROM_WRITE         =250,
  
  MSP_DEBUGMSG             =253,
  MSP_DEBUG                =254;

private static final int
  MSP_OSD                  =220;

// Subcommands
private static final int
  OSD_NULL                 =0,
  OSD_READ_CMD             =1,
  OSD_WRITE_CMD            =2;


// initialize the serial port selected in the listBox
void InitSerial(float portValue) {
  if (portValue < commListMax) {
    if(init_com == 0){ 
      String portPos = Serial.list()[int(portValue)];
      txtlblWhichcom.setValue("COM = " + shortifyPortName(portPos, 8));
      g_serial = new Serial(this, portPos, 115200);
      LastPort = portValue;
      init_com=1;
      toggleMSP_Data = true;
      buttonREAD.setColorBackground(green_);
      buttonRESET.setColorBackground(green_);
      commListbox.setColorBackground(green_);
      g_serial.buffer(256);
      System.out.println("Port Turned On " );
      delay(2000);
      
      READ();
      //+((int)(cmd&0xFF))+": "+(checksum&0xFF)+" expected, got "+(int)(c&0xFF));
    }
  }
  else {
    if(init_com == 1){ 
      txtlblWhichcom.setValue("Comm Closed");
      toggleMSP_Data = false;
      init_com=0;
      commListbox.setColorBackground(red_);
      buttonREAD.setColorBackground(red_);
      buttonRESET.setColorBackground(red_);
      buttonWRITE.setColorBackground(red_);
      init_com=0;
      g_serial.clear();
      toggleMSP_Data = false;
      delay(500);
      g_serial.clear();
      g_serial.stop();
      System.out.println("Port Turned Off " );
    }
  }
}

void SetConfigItem(int index, int value) {
  if(index >= CONFIGITEMS)
    return;

  confItem[index].setValue(value);
  if (index == CONFIGITEMS-1)
    buttonWRITE.setColorBackground(green_);
    
  if (value >0){
    toggleConfItem[index].setValue(1);
  }
  else{
    toggleConfItem[index].setValue(0);
  }

  try{
    switch(value) {
    case(0):
      RadioButtonConfItem[index].activate(0);
      break;
    case(1):
      RadioButtonConfItem[index].activate(1);
      break;
    }
  }
  catch(Exception e) {
  }finally {
  }  	
}



public void READ(){
  for(int i = 0; i < CONFIGITEMS; i++)
    SetConfigItem((byte)i, 0);

  p = 0;
  inBuf[0] = OSD_READ_CMD;
  evaluateCommand((byte)MSP_OSD, 1);

}

public void WRITE(){
  p = 0;
  inBuf[0] = OSD_WRITE_CMD;
  evaluateCommand((byte)MSP_OSD, 1);

}

// coded by Eberhard Rensch
// Truncates a long port name for better (readable) display in the GUI
String shortifyPortName(String portName, int maxlen)  {
  String shortName = portName;
  if(shortName.startsWith("/dev/")) shortName = shortName.substring(5);  
  if(shortName.startsWith("tty.")) shortName = shortName.substring(4); // get rid of leading tty. part of device name
  if(portName.length()>maxlen) shortName = shortName.substring(0,(maxlen-1)/2) + "~" +shortName.substring(shortName.length()-(maxlen-(maxlen-1)/2));
  if(shortName.startsWith("cu.")) shortName = "";// only collect the corresponding tty. devices
  return shortName;
}

public static final int
  IDLE = 0,
  HEADER_START = 1,
  HEADER_M = 2,
  HEADER_ARROW = 3,
  HEADER_SIZE = 4,
  HEADER_CMD = 5,
  HEADER_ERR = 6;

private static final String MSP_SIM_HEADER = "$M>";
int c_state = IDLE;
boolean err_rcvd = false;
List<Character> payload;
byte checksum=0;
byte cmd;
int offset=0, dataSize=0;
byte[] inBuf = new byte[256];
int Send_timer = 1;
int p;
int read32() {return (inBuf[p++]&0xff) + ((inBuf[p++]&0xff)<<8) + ((inBuf[p++]&0xff)<<16) + ((inBuf[p++]&0xff)<<24); }
int read16() {return (inBuf[p++]&0xff) + ((inBuf[p++])<<8); }
int read8()  {return inBuf[p++]&0xff;}



int outChecksum;


void serialize8(int val) {
 if ((init_com==1)  && (toggleMSP_Data == true)){
   PortWrite = true;
  
   try {
   g_serial.write(val);
   outChecksum ^= val;
    } catch (Exception e) { // null pointer or serial port dead
        System.out.println("write error " + e);
    }
    
    
     
     
           
   //NullPointerException
      //println("Error from Serialize8");
   
   //catch(NullPointerException e)
   //{
     //System.out.println("Error from serialize8");
   //}
   
  
 } 
}

void serialize16(int a) {
  serialize8((a   ) & 0xFF);
  serialize8((a>>8) & 0xFF);
}

void serialize32(int a) {
  serialize8((a    ) & 0xFF);
  serialize8((a>> 8) & 0xFF);
  serialize8((a>>16) & 0xFF);
  serialize8((a>>24) & 0xFF);
}

void serializeNames(int s) {
  //for (PGM_P c = s; pgm_read_byte(c); c++) {
   // serialize8(pgm_read_byte(c));
  //}
  for (int c = 0; c < strBoxNames.length(); c++) {
    serialize8(strBoxNames.charAt(c));
  }
}

void headSerialResponse(int requestMSP, Boolean err, int s) {
  serialize8('$');
  serialize8('M');
  serialize8(err ? '!' : '>');
  outChecksum = 0; // start calculating a new checksum
  serialize8(s);
  serialize8(requestMSP);
}

void headSerialReply(int requestMSP, int s) {
  headSerialResponse(requestMSP, false, s);
}

void headSerialError(int requestMSP, int s) {
  headSerialResponse(requestMSP, true, s);
}

void tailSerialReply() {
  serialize8(outChecksum);
}

public void evaluateCommand(byte cmd, int dataSize) {
   if (toggleMSP_Data == false) return;
  try{
  int icmd = (int)(cmd&0xFF);
  switch(icmd) {

  case MSP_OSD:
 
  {
    int cmd_internal = read8();
    if(cmd_internal == OSD_NULL) {
      headSerialReply(MSP_OSD, 1);
      serialize8(OSD_NULL);
    }

    if(cmd_internal == OSD_READ_CMD) {
      if(dataSize == 1) {
	// Send a NULL reply
	headSerialReply(MSP_OSD, 1);
	serialize8(OSD_READ_CMD);
      }
      else {
	// Returned result from OSD.
	for(int i = 0; i < CONFIGITEMS; i++)
	  SetConfigItem(i, read8());

	// Send a NULL reply
	headSerialReply(MSP_OSD, 1);
	serialize8(OSD_NULL);
      }
    }

    if(cmd_internal == OSD_WRITE_CMD && dataSize == 1) {
      headSerialReply(MSP_OSD, CONFIGITEMS+1);
      serialize8(OSD_WRITE_CMD);
      for(int i = 0; i < CONFIGITEMS; i++)
        serialize8(int(confItem[i].value()));
    }
    break;
  }
    
  case MSP_IDENT:
  
    headSerialReply(MSP_IDENT, 7);
    serialize8(101);   // multiwii version
    serialize8(0); // type of multicopter
    serialize8(0);         // MultiWii Serial Protocol Version
    serialize32(0);        // "capability"
    break;

  case MSP_STATUS:
   
    Send_timer+=1;
    headSerialReply(MSP_STATUS, 11);
    serialize16(Send_timer);
    serialize16(0);
    serialize16(1|1<<1|1<<2|1<<3|0<<4);
    
    int modebits = 0;
    int BitCounter = 1;
    for (int i=0; i<boxnames.length; i++) {
      if(toggleModeItems[i].getValue() > 0) modebits |= BitCounter;
      BitCounter += BitCounter;
    }
    
    serialize32(modebits);
    serialize8(0);   // current setting
    break;
    
  case MSP_BOXNAMES:
    
     headSerialReply(MSP_BOXNAMES,strBoxNames.length());
     serializeNames(strBoxNames.length());
    break;

  case MSP_ATTITUDE:
   
    headSerialReply(MSP_ATTITUDE, 8);
    serialize16(int(MW_Pitch_Roll.arrayValue()[0])*10);
    serialize16(int(MW_Pitch_Roll.arrayValue()[1])*10);
    serialize16(MwHeading);
    serialize16(0);
    break;

  case MSP_RC:
   
     headSerialReply(MSP_RC, 14);
      //Roll 
     serialize16(int(Pitch_Roll.arrayValue()[0]));
      //pitch
     serialize16(int(Pitch_Roll.arrayValue()[1]));
      //Yaw
     serialize16(int(Throttle_Yaw.arrayValue()[0]));
      //Throttle
     serialize16(int(Throttle_Yaw.arrayValue()[1]));
      for (int i=5; i<8; i++) {
       serialize16(1500);
      }
    break;
  
  
  case MSP_RAW_GPS:
   
   // We have: GPS_fix(0-2), GPS_numSat(0-15), GPS_coord[LAT & LON](signed, in 1/10 000 000 degres), GPS_altitude(signed, in meters) and GPS_speed(in cm/s)  
   //FormatGPSCoord(GPS_latitude,screenBuffer+1,3,'N','S');
    headSerialReply(MSP_RAW_GPS,16);
    serialize8(int(SGPS_FIX.arrayValue()[0]));
    serialize8(int(SGPS_numSat.value()));
    serialize32(430948610);
    serialize32(-718897060);
    serialize16(int(SGPS_altitude.value()/100));
    serialize16(int(SGPS_speed.value()));
    serialize16(355);     
    break;
    
  
  case MSP_COMP_GPS:
   
     headSerialReply(MSP_COMP_GPS,5);
     serialize16(int(SGPS_distanceToHome.value()));
     int GPSheading = int(SGPSHeadHome.value());
     if(GPSheading < 0) GPSheading += 360;
     serialize16(GPSheading);
     serialize8(0);
    break;
  
  case MSP_ALTITUDE:
  
    headSerialReply(MSP_ALTITUDE, 6);
    serialize32(int(sAltitude) *100);
    
    serialize16(int(sVario) *10);     
    break;
  
  case MSP_ANALOG:
  
    headSerialReply(MSP_ANALOG, 5);
    serialize8(int(sVBat * 10));
    serialize16(0);
    serialize16(int(sMRSSI));
    break;

   case MSP_RC_TUNING:
   
     headSerialReply(MSP_RC_TUNING, 7);
     serialize8(80);
     serialize8(80);
     serialize8(80);
     serialize8(80);
     serialize8(80);
     serialize8(80);
     serialize8(80);
     break;

   case MSP_PID:
   
     headSerialReply(MSP_PID, 3*10);
     for(int i=0;i<20;i++) {
       serialize8(40);
       serialize8(20);
       serialize8(70);
     }
     break;

   case MSP_RSSI:
   
     headSerialReply(MSP_RSSI, 2);
     serialize16(700);
     break;


  default:
    System.out.print("Unsupported request = ");
    System.out.println(str(icmd));
    break;
  }
  tailSerialReply();

}
    catch(Exception e) {
      println("error from evaluateCommand");
    }
    finally {
    }   
}

void MWData_Com() {
  if (toggleMSP_Data == false) return;
  List<Character> payload;
  int i,aa;
  float val,inter,a,b,h;
  int c = 0;
  if ((init_com==1)  && (toggleMSP_Data == true)) {
    
      
    while (g_serial.available()>0) {
     try{
      c = (g_serial.read());
     } catch (Exception e) { // null pointer or serial port dead
        System.out.println("write error " + e);
     }


      PortRead = true;
      if (c_state == IDLE) {
        c_state = (c=='$') ? HEADER_START : IDLE;
      }
      else if (c_state == HEADER_START) {
        c_state = (c=='M') ? HEADER_M : IDLE;
      }
      else if (c_state == HEADER_M) {
        if (c == '<') {
          c_state = HEADER_ARROW;
        } else if (c == '!') {
          c_state = HEADER_ERR;
        } else {
          c_state = IDLE;
        }
      }
      else if (c_state == HEADER_ARROW || c_state == HEADER_ERR) {
        /* is this an error message? */
        err_rcvd = (c_state == HEADER_ERR);        /* now we are expecting the payload size */
        dataSize = (c&0xFF);
        /* reset index variables */
        p = 0;
        offset = 0;
        checksum = 0;
        checksum ^= (c&0xFF);
        /* the command is to follow */
        c_state = HEADER_SIZE;
      }
      else if (c_state == HEADER_SIZE) {
        cmd = (byte)(c&0xFF);
        checksum ^= (c&0xFF);
        c_state = HEADER_CMD;
      }
      else if (c_state == HEADER_CMD && offset < dataSize) {
          checksum ^= (c&0xFF);
          inBuf[offset++] = (byte)(c&0xFF);
      } 
      else if (c_state == HEADER_CMD && offset >= dataSize) {
        /* compare calculated and transferred checksum */
        if ((checksum&0xFF) == (c&0xFF)) {
          if (err_rcvd) {
            //System.err.println("Copter did not understand request type "+c);
          } else {
            /* we got a valid response packet, evaluate it */
            if ((init_com==1)  && (toggleMSP_Data == true)) {
            evaluateCommand(cmd, (int)dataSize);
            }
           
          }
        }
        else {
          System.out.println("invalid checksum for command "+((int)(cmd&0xFF))+": "+(checksum&0xFF)+" expected, got "+(int)(c&0xFF));
          System.out.print("<"+(cmd&0xFF)+" "+(dataSize&0xFF)+"> {");
          for (i=0; i<dataSize; i++) {
            if (i!=0) { System.err.print(' '); }
            System.out.print((inBuf[i] & 0xFF));
          }
          System.out.println("} ["+c+"]");
          System.out.println(new String(inBuf, 0, dataSize));
        }
        c_state = IDLE;
        
      }
    }
  }
}





      
