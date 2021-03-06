#define DATAOUT 11              // MOSI
#define DATAIN  12              // MISO
#define SPICLOCK  13            // sck




#define VSYNC 2                 // INT0

//MAX7456 opcodes
#define DMM_reg   0x04
#define DMAH_reg  0x05
#define DMAL_reg  0x06
#define DMDI_reg  0x07
#define VM0_reg   0x00
#define VM1_reg   0x01

// video mode register 0 bits
#define VIDEO_BUFFER_DISABLE 0x01
//#define MAX7456_RESET 0x02
#define VERTICAL_SYNC_NEXT_VSYNC 0x04
#define OSD_ENABLE 0x08
#define SYNC_MODE_AUTO 0x00
#define SYNC_MODE_INTERNAL 0x30
#define SYNC_MODE_EXTERNAL 0x20
#define VIDEO_MODE_PAL 0x40
#define VIDEO_MODE_NTSC 0x00

// video mode register 1 bits


// duty cycle is on_off
#define BLINK_DUTY_CYCLE_50_50 0x00
#define BLINK_DUTY_CYCLE_33_66 0x01
#define BLINK_DUTY_CYCLE_25_75 0x02
#define BLINK_DUTY_CYCLE_75_25 0x03

// blinking time
#define BLINK_TIME_0 0x00
#define BLINK_TIME_1 0x04
#define BLINK_TIME_2 0x08
#define BLINK_TIME_3 0x0C

// background mode brightness (percent)
#define BACKGROUND_BRIGHTNESS_0 0x00
#define BACKGROUND_BRIGHTNESS_7 0x01
#define BACKGROUND_BRIGHTNESS_14 0x02
#define BACKGROUND_BRIGHTNESS_21 0x03
#define BACKGROUND_BRIGHTNESS_28 0x04
#define BACKGROUND_BRIGHTNESS_35 0x05
#define BACKGROUND_BRIGHTNESS_42 0x06
#define BACKGROUND_BRIGHTNESS_49 0x07

#define BACKGROUND_MODE_GRAY 0x40

//MAX7456 commands
#define CLEAR_display 0x04
#define CLEAR_display_vert 0x06
#define END_string 0xff

#define WHITE_level_80 0x03
#define WHITE_level_90 0x02
#define WHITE_level_100 0x01
#define WHITE_level_120 0x00

#define MAX7456ADD_VM0          0x00  //0b0011100// 00 // 00             ,0011100
#define MAX7456ADD_VM1          0x01
#define MAX7456ADD_HOS          0x02
#define MAX7456ADD_VOS          0x03
#define MAX7456ADD_DMM          0x04
#define MAX7456ADD_DMAH         0x05
#define MAX7456ADD_DMAL         0x06
#define MAX7456ADD_DMDI         0x07
#define MAX7456ADD_CMM          0x08
#define MAX7456ADD_CMAH         0x09
#define MAX7456ADD_CMAL         0x0a
#define MAX7456ADD_CMDI         0x0b
#define MAX7456ADD_OSDM         0x0c
#define MAX7456ADD_RB0          0x10
#define MAX7456ADD_RB1          0x11
#define MAX7456ADD_RB2          0x12
#define MAX7456ADD_RB3          0x13
#define MAX7456ADD_RB4          0x14
#define MAX7456ADD_RB5          0x15
#define MAX7456ADD_RB6          0x16
#define MAX7456ADD_RB7          0x17
#define MAX7456ADD_RB8          0x18
#define MAX7456ADD_RB9          0x19
#define MAX7456ADD_RB10         0x1a
#define MAX7456ADD_RB11         0x1b
#define MAX7456ADD_RB12         0x1c
#define MAX7456ADD_RB13         0x1d
#define MAX7456ADD_RB14         0x1e
#define MAX7456ADD_RB15         0x1f
#define MAX7456ADD_OSDBL        0x6c
#define MAX7456ADD_STAT         0xA0

// Selectable by board type
uint8_t MAX7456SELECT;		// output pin
uint8_t MAX7456RESET;		// output pin

// Selectable by video mode
//uint8_t ENABLE_display;
//uint8_t ENABLE_display_vert;
//uint8_t DISABLE_display;
uint16_t MAX_screen_size;
uint8_t videomode;

//////////////////////////////////////////////////////////////
uint8_t spi_transfer(uint8_t data)
{
  SPDR = data;                    // Start the transmission
  while (!(SPSR & (1<<SPIF)))     // Wait the end of the transmission
    ;
  return SPDR;                    // return the received byte
}

void MAX7456_Send(uint8_t add, uint8_t data)
{
  spi_transfer(add);
  spi_transfer(data);
}

// ============================================================   WRITE TO SCREEN

void MAX7456Setup(void)
{
  uint8_t MAX7456_reset;
  uint8_t MAX_screen_rows;

  
  MAX7456Configure();

  if(Settings[S_VIDEOSIGNALTYPE]) {    // PAL
    MAX7456_reset = 0x42;
    MAX_screen_size = 480;
    MAX_screen_rows = 16;
    videomode = VIDEO_MODE_PAL;
  }
  else {                                // NTSC
    MAX7456_reset = 0x02;
    MAX_screen_size = 390;
    MAX_screen_rows = 13;
    videomode = VIDEO_MODE_NTSC;
  }

  pinMode(MAX7456RESET,OUTPUT);
  digitalWrite(MAX7456RESET,HIGH); //hard enable

  delay(250);

  pinMode(MAX7456SELECT,OUTPUT);
  digitalWrite(MAX7456SELECT,HIGH); //disable device

  pinMode(DATAOUT, OUTPUT);
  pinMode(DATAIN, INPUT);
  pinMode(SPICLOCK,OUTPUT);
  pinMode(VSYNC, INPUT);

  // SPCR = 01010000
  //interrupt disabled,spi enabled,msb 1st,master,clk low when idle,
  //sample on leading edge of clk,system clock/4 rate (4 meg)

  SPCR = (1<<SPE)|(1<<MSTR);
  uint8_t spi_junk;
  spi_junk=SPSR;
  spi_junk=SPDR;
  delay(250);

  // force soft reset on Max7456
  digitalWrite(MAX7456SELECT,LOW);
  MAX7456_Send(VM0_reg, MAX7456_reset);
  digitalWrite(MAX7456SELECT,HIGH);
  delay(500);

  // set all rows to same charactor white level, 90%
  digitalWrite(MAX7456SELECT,LOW);
  
  uint8_t x;
  for(x = 0; x < MAX_screen_rows; x++) {
    MAX7456_Send(MAX7456ADD_RB0+x, WHITE_level_120);
  }

  // make sure the Max7456 is enabled
  MAX7456_Send(VM0_reg, videomode|OSD_ENABLE);

  digitalWrite(MAX7456SELECT,HIGH);
  delay(100);
}

// Copy string from ram into screen buffer
void MAX7456_WriteString(const char *string, int Adresse)
{
  uint8_t xx = 0;
  char c;
  while((c = string[xx++]) != 0)
  {
    screen[Adresse++] = c;
  }
}

// Copy string from progmem into the screen buffer
void MAX7456_WriteString_P(const char *string, int Adresse)
{
  uint8_t xx = 0;
  char c;
  while((c = (char)pgm_read_byte(&string[xx++])) != 0)
  {
    screen[Adresse++] = c;
  }
}

void MAX7456_DrawScreen()
{
  int xx;

  digitalWrite(MAX7456SELECT,LOW);
  for(xx=0;xx<MAX_screen_size;++xx)
  {
    MAX7456_Send(MAX7456ADD_DMAH, xx>>8);
    MAX7456_Send(MAX7456ADD_DMAL, xx);
    MAX7456_Send(MAX7456ADD_DMDI, screen[xx]);
    screen[xx] = ' ';
  }
  digitalWrite(MAX7456SELECT,HIGH);
}

void MAX7456Configure() {
  // todo - automatically recognising card.
  if(Settings[S_BOARDTYPE] == 0) {	// Minim
    MAX7456SELECT = 10;      // ss 
    MAX7456RESET  = 9;       // RESET
  }

  if(Settings[S_BOARDTYPE] == 1) {	// Rush
    MAX7456SELECT = 6;       // ss
    MAX7456RESET = 10;       // RESET
  }
}

#define WRITE_TO_MAX7456
#define NVM_ram_size 54
#define WRITE_nvr 0xa0
#define STATUS_reg_nvr_busy 0x20

void write_NVM(uint8_t char_address)
{
#ifdef WRITE_TO_MAX7456
  // disable display
  digitalWrite(MAX7456SELECT,LOW);
  MAX7456_Send(VM0_reg, videomode);

  MAX7456_Send(MAX7456ADD_CMAH, char_address); // set start address high

  for(uint8_t x = 0; x < NVM_ram_size; x++) // write out 54 bytes of character to shadow ram
  {
    MAX7456_Send(MAX7456ADD_CMAL, x); // set start address low
    MAX7456_Send(MAX7456ADD_CMDI, fontData[x]);
  }

  // Transfer the 54 bytes from shadow ram to NVM
  MAX7456_Send(MAX7456ADD_CMM, WRITE_nvr);
  
  // wait until bit 5 in the status register returns to 0 (12ms)
  while ((spi_transfer(MAX7456ADD_STAT) & STATUS_reg_nvr_busy) != 0x00)
    ;
    
  //MAX7456_Send(VM0_reg, videomode|OSD_ENABLE); // turn on screen
  digitalWrite(MAX7456SELECT,HIGH);
#else
  delay(12);
#endif
}
