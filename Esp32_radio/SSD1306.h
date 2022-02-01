#include <driver/i2c.h>
#include <string.h>


#define OLED_I2C_ADDRESS   0x3C
// Color definitions for the TFT screen (if used)
// OLED only have 2 colors.
#define BLACK   0
#define BLUE    0xFF
#define RED     0xFF
#define GREEN   0
#define CYAN    GREEN | BLUE
#define MAGENTA RED | BLUE
#define YELLOW  RED | GREEN
#define WHITE   RED | GREEN | BLUE

// Data to display.  There are TFTSECS sections

struct page_struct
{
  uint8_t   page[128] ;               // Buffer for one page (8 pixels heigh)
  bool      dirty ;                   // True if modified
} ;

class SSD1306
{
  public:
    SSD1306 ( uint8_t sda, uint8_t scl ) ;         // Constructor
    void      clear() ;                            // Clear buffer
    void      display() ;                          // Display buffer
    void      print ( char c ) ;                   // Print a character
    void      print ( const char* str ) ;          // Print a string
    void      setCursor ( uint8_t x, uint8_t y ) ; // Position the cursor
    void      fillRect ( uint8_t x, uint8_t y,     // Fill a rectangle
                         uint8_t w, uint8_t h,
                         uint8_t color ) ;
    void      dsp_showPreset( int nr );
    uint8_t   preset = 100;          // lead to zero-width

  private:
    struct page_struct*     ssdbuf = NULL ;
    void                    sendCommand ( uint8_t command ) ;
    const  uint8_t*         font ;                 // Font to use
    uint8_t                 xchar = 0 ;            // Current cursor position (text)
    uint8_t                 ychar = 0 ;            // Current cursor position (text)
    uint8_t					ssd1306_npag = 8;	   // oled 128x64
    
} ;


SSD1306* tft = NULL ;

#define TFTSECS 4
scrseg_struct     tftdata[TFTSECS] =                        // Screen divided in 3 segments + 1 overlay
{                                                           // One text line is 8 pixels
  { false, WHITE,   0,  8, "" },                            // 1 top line
  { false, CYAN,    8, 32, "" },                            // 4 lines in the middle
  { false, YELLOW, 40, 22, "" },                            // 3 lines at the bottom, leave room for indicator
  { false, GREEN,  48, 14, "" }                             // 2 lines at the bottom for rotary encoder
} ;

// Various macro's to mimic the ST7735 version of display functions
#define dsp_setRotation()                                          // Use standard landscape format
#define dsp_setTextSize(a)                                         // Set the text size
#define dsp_setTextColor(a)
#define dsp_print(a)            tft->print ( a )                   // Print a string 
#define dsp_println(a)          tft->print ( a ) ; \
                                tft->print ( '\n' )                // Print string plus newline
#define dsp_setCursor(a,b)      tft->setCursor ( a, b )            // Position the cursor
#define dsp_fillRect(a,b,c,d,e) tft->fillRect ( a, b, c, d, e )    // Fill a rectangle
#define dsp_erase()             tft->clear()                       // Clear the screen
#define dsp_getwidth()          128                                // Get width of screen
#define dsp_update()            tft->display()                     // Updates to the physical screen
#define dsp_usesSPI()           false                              // Does not use SPI

int dsp_getheight()
{
  return oled_128_32 ? 32 : 64;
}

bool dsp_begin()
{
  dbgprint ( "Init SSD1306, I2C pins %d,%d", ini_block.tft_sda_pin,
             ini_block.tft_scl_pin ) ;
  if ( ( ini_block.tft_sda_pin >= 0 ) &&
       ( ini_block.tft_scl_pin >= 0 ) )
  {
    tft = new SSD1306 ( ini_block.tft_sda_pin,
                        ini_block.tft_scl_pin ) ;                  // Create an instant for TFT
  }
  else
  {
    dbgprint ( "Init LCD1306 failed!" ) ;
    
  }
  return ( tft != NULL ) ;
}

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1    
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8    
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

const uint8_t SSD1306FONTWIDTH = 5 ;
const uint8_t SSD1306font[] =
            {
              0x00, 0x00, 0x00, 0x00, 0x00, // SPACE
              0x00, 0x00, 0x5F, 0x00, 0x00, // !
              0x00, 0x03, 0x00, 0x03, 0x00, // "
              0x14, 0x3E, 0x14, 0x3E, 0x14, // #
              0x24, 0x2A, 0x7F, 0x2A, 0x12, // $
              0x43, 0x33, 0x08, 0x66, 0x61, // %
              0x36, 0x49, 0x55, 0x22, 0x50, // &
              0x00, 0x05, 0x03, 0x00, 0x00, // '
              0x00, 0x1C, 0x22, 0x41, 0x00, // (
              0x00, 0x41, 0x22, 0x1C, 0x00, // )
              0x14, 0x08, 0x3E, 0x08, 0x14, // *
              0x08, 0x08, 0x3E, 0x08, 0x08, // +
              0x00, 0xA0, 0x60, 0x00, 0x00, // ,
              0x08, 0x08, 0x08, 0x08, 0x08, // -
              0x00, 0x60, 0x60, 0x00, 0x00, // .
              0x20, 0x10, 0x08, 0x04, 0x02, // /
            
              0x3E, 0x51, 0x49, 0x45, 0x3E, // 0
              0x00, 0x04, 0x02, 0x7F, 0x00, // 1
              0x42, 0x61, 0x51, 0x49, 0x46, // 2
              0x22, 0x41, 0x49, 0x49, 0x36, // 3
              0x18, 0x14, 0x12, 0x7F, 0x10, // 4
              0x27, 0x45, 0x45, 0x45, 0x39, // 5
              0x3E, 0x49, 0x49, 0x49, 0x32, // 6
              0x01, 0x01, 0x71, 0x09, 0x07, // 7
              0x36, 0x49, 0x49, 0x49, 0x36, // 8
              0x26, 0x49, 0x49, 0x49, 0x3E, // 9
              0x00, 0x36, 0x36, 0x00, 0x00, // :
              0x00, 0x56, 0x36, 0x00, 0x00, // ;
              0x08, 0x14, 0x22, 0x41, 0x00, // <
              0x14, 0x14, 0x14, 0x14, 0x14, // =
              0x00, 0x41, 0x22, 0x14, 0x08, // >
              0x02, 0x01, 0x51, 0x09, 0x06, // ?
            
              0x3E, 0x41, 0x59, 0x55, 0x5E, // @
              0x7E, 0x09, 0x09, 0x09, 0x7E, // A
              0x7F, 0x49, 0x49, 0x49, 0x36, // B
              0x3E, 0x41, 0x41, 0x41, 0x22, // C
              0x7F, 0x41, 0x41, 0x41, 0x3E, // D
              0x7F, 0x49, 0x49, 0x49, 0x41, // E
              0x7F, 0x09, 0x09, 0x09, 0x01, // F
              0x3E, 0x41, 0x41, 0x49, 0x3A, // G
              0x7F, 0x08, 0x08, 0x08, 0x7F, // H
              0x00, 0x41, 0x7F, 0x41, 0x00, // I
              0x30, 0x40, 0x40, 0x40, 0x3F, // J
              0x7F, 0x08, 0x14, 0x22, 0x41, // K
              0x7F, 0x40, 0x40, 0x40, 0x40, // L
              0x7F, 0x02, 0x0C, 0x02, 0x7F, // M
              0x7F, 0x02, 0x04, 0x08, 0x7F, // N
              0x3E, 0x41, 0x41, 0x41, 0x3E, // O
            
              0x7F, 0x09, 0x09, 0x09, 0x06, // P
              0x1E, 0x21, 0x21, 0x21, 0x5E, // Q
              0x7F, 0x09, 0x09, 0x09, 0x76, // R
              0x26, 0x49, 0x49, 0x49, 0x32, // S
              0x01, 0x01, 0x7F, 0x01, 0x01, // T
              0x3F, 0x40, 0x40, 0x40, 0x3F, // U
              0x1F, 0x20, 0x40, 0x20, 0x1F, // V
              0x7F, 0x20, 0x10, 0x20, 0x7F, // W
              0x41, 0x22, 0x1C, 0x22, 0x41, // X
              0x07, 0x08, 0x70, 0x08, 0x07, // Y
              0x61, 0x51, 0x49, 0x45, 0x43, // Z
              0x00, 0x7F, 0x41, 0x00, 0x00, // [
              0x02, 0x04, 0x08, 0x10, 0x20, // backslash
              0x00, 0x00, 0x41, 0x7F, 0x00, // ]
              0x04, 0x02, 0x01, 0x02, 0x04, // ^
              0x40, 0x40, 0x40, 0x40, 0x40, // _
            
              0x00, 0x01, 0x02, 0x04, 0x00, // `
              0x20, 0x54, 0x54, 0x54, 0x78, // a
              0x7F, 0x44, 0x44, 0x44, 0x38, // b
              0x38, 0x44, 0x44, 0x44, 0x44, // c
              0x38, 0x44, 0x44, 0x44, 0x7F, // d
              0x38, 0x54, 0x54, 0x54, 0x18, // e
              0x04, 0x04, 0x7E, 0x05, 0x05, // f
              0x18, 0xA4, 0xA4, 0xA4, 0x78, // g
              0x7F, 0x08, 0x04, 0x04, 0x78, // h
              0x00, 0x44, 0x7D, 0x40, 0x00, // i
              0x20, 0x40, 0x44, 0x3D, 0x00, // j
              0x7F, 0x10, 0x28, 0x44, 0x00, // k
              0x00, 0x41, 0x7F, 0x40, 0x00, // l
              0x7C, 0x04, 0x78, 0x04, 0x78, // m
              0x7C, 0x08, 0x04, 0x04, 0x78, // n
              0x38, 0x44, 0x44, 0x44, 0x38, // o
            
              0xFC, 0x24, 0x24, 0x24, 0x18, // p
              0x18, 0x24, 0x24, 0x24, 0xFC, // q
              0x00, 0x7C, 0x08, 0x04, 0x04, // r
              0x48, 0x54, 0x54, 0x54, 0x20, // s
              0x04, 0x04, 0x3F, 0x44, 0x44, // t
              0x3C, 0x40, 0x40, 0x20, 0x7C, // u
              0x1C, 0x20, 0x40, 0x20, 0x1C, // v
              0x3C, 0x40, 0x30, 0x40, 0x3C, // w
              0x44, 0x28, 0x10, 0x28, 0x44, // x
              0x0C, 0x50, 0x50, 0x50, 0x3C, // y
              0x44, 0x64, 0x54, 0x4C, 0x44, // z
              0x00, 0x08, 0x36, 0x41, 0x41, // {
              0x00, 0x00, 0x7F, 0x00, 0x00, // |
              0x41, 0x41, 0x36, 0x08, 0x00, // }
              0x02, 0x01, 0x02, 0x04, 0x02  // ~
            } ;

const uint8_t bigfont1[] =
            {
/*0*/         0x00,0x80,0xe0,0xf0,0xfa,0xfa,0xf9,0x7b,0x33,0x47,0x0f,0x1f,0xfe,0xfe,0xfc,0xf8,0xf0,0x80,0x00,
/*1*/         0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xfe,0xfe,0xfe,0xfc,0x00,
/*2*/         0x00,0x00,0x70,0xf8,0xfc,0xfc,0x7e,0x3e,0x3e,0x3e,0x3e,0xfe,0xfc,0xfc,0xf8,0xe0,0x00,
/*3*/         0x00,0x00,0x3c,0x3e,0x3e,0x3f,0x1f,0x1f,0x1f,0x1f,0xff,0xff,0xfe,0xfe,0xfc,0xf0,0x00,0x00,
/*4*/         0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xc0,0xf0,0xf8,0xfc,0xfc,0x7c,0xfc,0xb8,0x00,0x00,0x00,0x00,
/*5*/         0x00,0xf0,0xfc,0xfe,0x7e,0x7e,0x7e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x1c,0x00,
/*6*/         0x00,0x00,0xc0,0xf0,0xf8,0xf8,0x7c,0x7c,0x7e,0x3e,0x3e,0x3e,0x3e,0x3e,0x1c,0x00,0x00,0x00,
/*7*/         0x00,0x1c,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0xbe,0xbe,0x3e,0x7e,0xfe,0x7c,0x00,
/*8*/         0x00,0xf0,0xfc,0xfe,0xfe,0xff,0xbf,0x1f,0x1f,0x1f,0xbf,0xff,0xfe,0xfe,0xfc,0xf8,0x00,0x00,0x00,
/*9*/         0x00,0xe0,0xf8,0xf8,0xfc,0xfc,0x7e,0x3e,0x3e,0x3e,0x3e,0x7e,0xfe,0xfc,0xfc,0xf8,0xf0,0x00,
			};

const uint8_t bigfont2[] =
            {
/*0*/         0x00,0x7f,0xff,0xff,0xff,0xff,0x80,0x00,0x00,0x00,0x00,0x00,0x80,0xff,0xff,0xff,0xff,0x7f,0x00,
/*1*/         0x01,0x03,0x03,0x03,0x01,0xf9,0xfc,0xfe,0xfe,0xff,0x00,
/*2*/         0x00,0x00,0x00,0x00,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0x7f,0x3f,0x1f,0x0f,0x03,0x00,
/*3*/         0x00,0x00,0x00,0x00,0x00,0x00,0x1e,0x1e,0x3f,0x3f,0x3e,0xfc,0xfd,0xf9,0xfb,0xe0,0x00,0x00,
/*4*/         0x00,0xe0,0xf0,0xf8,0xfe,0xff,0xff,0xef,0xe7,0xe3,0xfd,0xfe,0xff,0xff,0xff,0xe0,0xe0,0x80,0x00,
/*5*/         0x00,0x1f,0x1e,0x1e,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f,0x3f,0xff,0xfe,0xfe,0xfc,0xf0,0x00,
/*6*/         0x00,0xf6,0xfb,0xfd,0xfe,0xfe,0x3f,0x1f,0x1f,0x1f,0x1f,0x3f,0xfe,0xfe,0xfc,0xfc,0xf0,0x00,
/*7*/         0x00,0x00,0x00,0x00,0x00,0x80,0xe0,0xf8,0xff,0xff,0xff,0x7f,0x0e,0x03,0x00,0x00,
/*8*/         0xc0,0xf1,0xf3,0xf7,0xef,0xef,0x2f,0x0f,0x1e,0x1e,0x3e,0xfe,0xfd,0xfd,0xfb,0xf1,0xc0,0x00,0x00,
/*9*/         0x00,0x07,0x1f,0x1f,0x3f,0x3f,0x7e,0x7c,0x7c,0x7c,0x7c,0x7e,0xbf,0xbf,0xdf,0xef,0x37,0x00,
			};

const uint8_t bigfont3[] =
            {
/*0*/         0x00,0x00,0x03,0x07,0x0f,0x1f,0x3f,0x3f,0x3e,0x3e,0x3e,0x3f,0x3f,0x1f,0x0f,0x07,0x03,0x00,0x00,
/*1*/         0x00,0x00,0x00,0x00,0x00,0x1f,0x3f,0x3f,0x3f,0x1f,0x00,
/*2*/         0x00,0x08,0x3e,0x3f,0x3f,0x3f,0x3f,0x3f,0x27,0x3d,0x3e,0x3e,0x3e,0x3e,0x3e,0x1c,0x00,
/*3*/         0x00,0x0c,0x1e,0x1e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3f,0x1f,0x1f,0x0f,0x07,0x00,0x00,
/*4*/         0x00,0x01,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x1f,0x3f,0x3f,0x3f,0x1f,0x03,0x03,0x00,0x00,
/*5*/         0x00,0x0c,0x1e,0x1e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3e,0x3f,0x3f,0x1f,0x1f,0x0f,0x03,0x00,
/*6*/         0x00,0x03,0x0f,0x1f,0x1f,0x3f,0x3f,0x3e,0x3e,0x3e,0x3e,0x3f,0x1f,0x1f,0x0f,0x0f,0x03,0x00,
/*7*/         0x00,0x00,0x00,0x00,0x1c,0x3f,0x3f,0x3f,0x1f,0x07,0x01,0x00,0x00,0x00,0x00,0x00,
/*8*/         0x01,0x07,0x0f,0x1f,0x1f,0x3f,0x3f,0x3e,0x3e,0x3e,0x3f,0x3f,0x1f,0x1f,0x0f,0x07,0x01,0x00,0x00,
/*9*/         0x00,0x00,0x00,0x1c,0x3e,0x3e,0x3e,0x3e,0x3f,0x3f,0x1f,0x1f,0x0f,0x0f,0x07,0x01,0x00,0x00,
			};

const struct {
  int   x;
  int   width;
} spfont[] = {
  { 0,   19 },  /* 0 */
  { 19,  11 },  /* 1 */
  { 30,  17 },  /* 2 */
  { 47,  18 },  /* 3 */
  { 65,  19 },  /* 4 */
  { 84,  17 },  /* 5 */
  { 101, 18 },  /* 6 */
  { 119, 16 },  /* 7 */
  { 135, 19 },  /* 8 */
  { 154, 18 }   /* 9 */
};

int dsp_presetWidth( )
{
  int val;
  if ( !tft )
    return 0;
  if ( tft->preset > 99 )
    return 0;
  val = spfont[tft->preset%10].width;
  if ( tft->preset > 9 )
    val += spfont[tft->preset/10].width;
  return val;
}

//***********************************************************************************************
//                                S S D 1 3 0 6 :: P R I N T                                    *
//***********************************************************************************************
// Put a character in the buffer.                                                               *
//***********************************************************************************************
void SSD1306::print ( char c )
{
  int dw;
  if ( ( c >= ' ' ) && ( c <= '~' ) )                    // Check the legal range
  {
    memcpy ( &ssdbuf[ychar].page[xchar],                 // Copy bytes for 1 character
             &font[(c-32)*SSD1306FONTWIDTH], SSD1306FONTWIDTH ) ;
    ssdbuf[ychar].dirty = true ;
    xchar += SSD1306FONTWIDTH ;                            // Move x cursor
  }
  /* special 20..29 : big numbers , width=19, height=3lines*/
  else if ( ( c >= 20 ) && ( c <= 29 ) )
  {
    int idx=c-20;
	  int max = dsp_getwidth()-xchar;	/* bytes left on this line */
    if ( max > spfont[idx].width )
       max = spfont[idx].width;
    memcpy(&ssdbuf[ychar].page[xchar], bigfont1+spfont[idx].x, max );
    ssdbuf[ychar].dirty = true ;
	  if ( ychar < oled_128_32 ? 3:7 )
    {
		  memcpy(&ssdbuf[ychar+1].page[xchar], bigfont2+spfont[idx].x, max );
		  ssdbuf[ychar+1].dirty = true ;
	  }
	  if ( ychar < oled_128_32 ? 2:6 )
    {
		  memcpy(&ssdbuf[ychar+2].page[xchar], bigfont3+spfont[idx].x, max );
		  ssdbuf[ychar+2].dirty = true ;
	  }
    xchar += max;
  }
  else if ( c == '\n' )                                  // New line?
  {
    xchar = dsp_getwidth() ;                             // Yes, force next line
  }
  dw = dsp_getwidth();
  if ( ychar )
    dw-=dsp_presetWidth( );
  if ( xchar > ( dw - SSD1306FONTWIDTH ) )   // End of line?
  {
    xchar = 0 ;                                          // Yes, mimic CR
    ychar = ( ychar + 1 ) & 7 ;                          // And LF
    if (( ychar == 3 ) && oled_128_32 && (tft->preset < 100))
      ychar = 4;  // invisible space
  }
}

//***********************************************************************************************
//                                S S D 1 3 0 6 :: P R I N T                                    *
//***********************************************************************************************
// Put a character in the buffer.                                                               *
//***********************************************************************************************
void SSD1306::print ( const char* str )
{
  while ( *str )                                // Print until delimeter
  {
    print ( *str ) ;                            // Print next character
    str++ ;
  }
}


//***********************************************************************************************
//                                S S D 1 3 0 6 :: D I S P L A Y                                *
//***********************************************************************************************
// Refresh the display.                                                                         *
//***********************************************************************************************
void SSD1306::display()
{
  i2c_cmd_handle_t cmd ;
  uint8_t          pg ;           // Page number 0..ssd1306_npag - 1

  for ( pg = 0 ; pg < ssd1306_npag ; pg++ )
  {
    if ( ssdbuf[pg].dirty )
    {
      cmd = i2c_cmd_link_create() ;
      i2c_master_start ( cmd ) ;
      i2c_master_write_byte ( cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true ) ;
      i2c_master_write_byte ( cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true ) ;
      i2c_master_write_byte ( cmd, 0xB0 | pg, true ) ;      // Set page address
      i2c_master_write_byte ( cmd, OLED_CONTROL_BYTE_DATA_STREAM, true ) ;
      i2c_master_write ( cmd, ssdbuf[pg].page, 128, true ) ;
      i2c_master_stop ( cmd ) ;
      i2c_master_cmd_begin ( I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS ) ;
      i2c_cmd_link_delete ( cmd ) ;
      ssdbuf[pg].dirty = false ;
    }
  }
}


//***********************************************************************************************
//                              S S D 1 3 0 6 :: C L E A R                                      *
//***********************************************************************************************
// Clear the display buffer and the display.                                                    *
//***********************************************************************************************
void SSD1306::clear()
{
  for ( uint8_t pg = 0 ; pg < ssd1306_npag ; pg++ )
  {
    memset ( ssdbuf[pg].page, BLACK, dsp_getwidth() ) ;  // Clears all pixels of 1 page
    ssdbuf[pg].dirty = true ;                            // Force refresh
  }
  xchar = 0 ;
  ychar = 0 ;
}


//***********************************************************************************************
//                                S S D 1 3 0 6 :: S E T C U R S O R                            *
//***********************************************************************************************
// Position the cursor.  It will be set at a page boundary (line of texts).                     *
//***********************************************************************************************
void SSD1306::setCursor ( uint8_t x, uint8_t y )          // Position the cursor
{
  xchar = x % dsp_getwidth() ;
  ychar = y / 8 % 8 ;
}


//***********************************************************************************************
//                                S S D 1 3 0 6 :: F I L L R E C T                              *
//***********************************************************************************************
// Fill a rectangle.  It will clear a number of pages (lines of texts ).                        *
// Not very fast....                                                                            *
//***********************************************************************************************
void SSD1306::fillRect ( uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color )
{
  uint8_t  pg ;                                           // Page to fill
  uint8_t  pat ;                                          // Fill pattern
  uint8_t  xc, yc ;                                       // Running x and y in rectangle
  uint8_t* p ;                                            // Point into ssdbuf

  if ( !w || !h )
    return;
  
  for ( yc = y ; yc < ( y + h ) ; yc++ )                  // Loop vertically
  {
    pg = ( yc / 8 ) % ssd1306_npag ;                      // Page involved
    pat = 1 << ( yc & 7 ) ;                               // Bit involved
    p = ssdbuf[pg].page + x ;                             // Point to right place
    for ( xc = x ; xc < ( x + w ) ; xc++ )                // Loop horizontally
    {
      if ( color )                                        // Set bit?
      {
        *p |= pat ;                                       // Yes, set bit
      }
      else
      {
        *p &= ~pat ;                                      // No, clear bit
      }
      p++ ;                                               // Next 8 pixels
    }
    ssdbuf[pg].dirty = true ;                             // Page has been changed
  }
}


//***********************************************************************************************
//                                S S D 1 3 0 6                                                 *
//***********************************************************************************************
// Constructor for the display.                                                                 *
//***********************************************************************************************
SSD1306::SSD1306 ( uint8_t sda, uint8_t scl )
{
  i2c_cmd_handle_t cmd ;

  ssdbuf = (page_struct*) malloc ( 8 * sizeof(page_struct) ) ;  // Create buffer for screen
  i2c_config_t i2c_config = {                                   // Set-up of I2C configuration
    I2C_MODE_MASTER,
    (gpio_num_t)sda,
    GPIO_PULLUP_ENABLE,
    (gpio_num_t)scl,
    GPIO_PULLUP_ENABLE,
    1000000
  } ;
  font = SSD1306font ;
  if ( oled_128_32 )
  {
    ssd1306_npag = 4;
    tftdata[1].height = 16;
    tftdata[2].y = 24;
    tftdata[3].y = 24;
    tftdata[2].height = 8;
    tftdata[3].height = 8;
  }
  i2c_param_config ( I2C_NUM_0, &i2c_config ) ;
  i2c_driver_install ( I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0 ) ;
  cmd = i2c_cmd_link_create() ;
  i2c_master_start ( cmd ) ;
  i2c_master_write_byte ( cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true ) ;
  i2c_master_write_byte ( cmd, OLED_CONTROL_BYTE_CMD_STREAM, true ) ;
    i2c_master_write_byte ( cmd, 0xae, true );  // display off
    i2c_master_write_byte ( cmd, 0xd5, true );  // set displayclockdiv
    i2c_master_write_byte ( cmd, 0x80, true );  //
    i2c_master_write_byte ( cmd, 0xa8, true );  // set multiplex
    i2c_master_write_byte ( cmd, oled_128_32 ? 0x1f: 0x3f, true );  //  1/32 , 1/64
    i2c_master_write_byte ( cmd, 0xd3, true );  // display offset
    i2c_master_write_byte ( cmd, 0x00, true );  //
    i2c_master_write_byte ( cmd, 0x40, true );  // set start line to #0
    i2c_master_write_byte ( cmd, 0x8d, true );  // charge-pump
    i2c_master_write_byte ( cmd, 0x14, true );  // charge-pump on
    i2c_master_write_byte ( cmd, 0x20, true );  // memory mode
    i2c_master_write_byte ( cmd, 0x00, true );  // like ks0108
    i2c_master_write_byte ( cmd, 0xda, true );  // set com pins
    i2c_master_write_byte ( cmd, oled_128_32 ? 0x02 : 0x12, true );  //
    i2c_master_write_byte ( cmd, 0x81, true );  // set contrast
    i2c_master_write_byte ( cmd, (dsp_brightness * 32)%255, true );  //
    i2c_master_write_byte ( cmd, 0xd9, true );  // set precharge
    i2c_master_write_byte ( cmd, 0xf1, true );  //
    i2c_master_write_byte ( cmd, 0xdb, true );  // set vcom detect
    i2c_master_write_byte ( cmd, oled_128_32 ? 0x40 : 0x30, true );  //
    i2c_master_write_byte ( cmd, 0x2e, true );  // deactivate scroll
    i2c_master_write_byte ( cmd, 0xa4, true );  // display all on resume
    i2c_master_write_byte ( cmd, 0xa6, true );  // normal display
//  i2c_master_write_byte ( cmd, OLED_CMD_SET_CHARGE_PUMP, true ) ;
//  i2c_master_write_byte ( cmd, 0x14, true ) ;                     // Enable charge pump
//  i2c_master_write_byte ( cmd, OLED_CMD_SET_CONTRAST, true ) ;    // Set contrast
//  i2c_master_write_byte ( cmd, 0x7f, true ) ;                      // to 255
 
  i2c_master_write_byte ( cmd, rotate_screen ? 0xa1 : 0xa0, true );    // a1 = vert.flip
  i2c_master_write_byte ( cmd, rotate_screen ? 0xc8 : 0xc0, true );    // c8 = hor.flip, c0 = normal
 
  i2c_master_write_byte ( cmd, OLED_CMD_DISPLAY_ON, true ) ;
  i2c_master_stop ( cmd ) ;
  i2c_master_cmd_begin ( I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS ) ;
  i2c_cmd_link_delete ( cmd ) ;
  clear() ;                                                       // Clear the display
}


//**************************************************************************************************
//                                      D I S P L A Y B A T T E R Y                                *
//**************************************************************************************************
// Show the current battery charge level on the screen.                                            *
// It will overwrite the top divider.                                                              *
// No action if bat0/bat100 not defined in the preferences.                                        *
//**************************************************************************************************
void displaybattery()
{
  if ( tft && !oled_128_32 )
  {
    if ( ini_block.bat0 < ini_block.bat100 )              // Levels set in preferences?
    {
      static uint16_t oldpos = 0 ;                        // Previous charge level
      uint16_t        ypos ;                              // Position on screen
      uint16_t        v ;                                 // Constrainted ADC value
      uint16_t        newpos ;                            // Current setting

      v = constrain ( adcval, ini_block.bat0,             // Prevent out of scale
                      ini_block.bat100 ) ;
      newpos = map ( v, ini_block.bat0,                   // Compute length of green bar
                     ini_block.bat100,
                     0, dsp_getwidth() ) ;
      if ( newpos != oldpos )                             // Value changed?
      {
        oldpos = newpos ;                                 // Remember for next compare
        ypos = tftdata[1].y - 5 ;                         // Just before 1st divider
        dsp_fillRect ( 0, ypos, newpos, 2, GREEN ) ;      // Paint green part
        dsp_fillRect ( newpos, ypos,
                       dsp_getwidth() - newpos,
                       2, RED ) ;                          // Paint red part
      }
    }
  }
}


//**************************************************************************************************
//                                      D I S P L A Y V O L U M E                                  *
//**************************************************************************************************
// Show the current volume as an indicator on the screen.                                          *
// The indicator is 2 pixels heigh.                                                                *
//**************************************************************************************************
void displayvolume()
{
  if ( tft && !oled_128_32 )
  {
    static uint8_t oldvol = 0 ;                         // Previous volume
    uint8_t        newvol ;                             // Current setting
    uint16_t       pos ;                                // Positon of volume indicator

    newvol = vs1053player->getVolume() ;                // Get current volume setting
    if ( newvol != oldvol )                             // Volume changed?
    {
      oldvol = newvol ;                                 // Remember for next compare
      pos = map ( newvol, 0, 100, 0, dsp_getwidth() ) ; // Compute position on TFT
      dsp_fillRect ( 0, dsp_getheight() - 2,
                     pos, 2, RED ) ;                    // Paint red part
      dsp_fillRect ( pos, dsp_getheight() - 2,
                     dsp_getwidth() - pos, 2, GREEN ) ; // Paint green part
    }
  }
}


//**************************************************************************************************
//                                      D I S P L A Y T I M E                                      *
//**************************************************************************************************
// Show the time on the LCD at a fixed position in a specified color                               *
// To prevent flickering, only the changed part of the timestring is displayed.                    *
// An empty string will force a refresh on next call.                                              *
// A character on the screen is 8 pixels high and 6 pixels wide.                                   *
//**************************************************************************************************
void displaytime ( const char* str, uint16_t color )
{
  static char oldstr[9] = "........" ;             // For compare
  uint8_t     i ;                                  // Index in strings
  uint16_t    pos = dsp_getwidth() + TIMEPOS ;     // X-position of character, TIMEPOS is negative

  if ( str[0] == '\0' )                            // Empty string?
  {
    for ( i = 0 ; i < 8 ; i++ )                    // Set oldstr to dots
    {
      oldstr[i] = '.' ;
    }
    return ;                                       // No actual display yet
  }
  if ( tft )                                       // TFT active?
  {
    dsp_setTextColor ( color ) ;                   // Set the requested color
    for ( i = 0 ; i < 8 ; i++ )                    // Compare old and new
    {
      if ( str[i] != oldstr[i] )                   // Difference?
      {
        dsp_fillRect ( pos, 0, 6, 8, BLACK ) ;     // Clear the space for new character
        dsp_setCursor ( pos, 0 ) ;                 // Prepare to show the info
        dsp_print ( str[i] ) ;                     // Show the character
        oldstr[i] = str[i] ;                       // Remember for next compare
      }
      pos += 6 ;                                   // Next position
    }
  }
}

void dsp_showPreset( int preset )
{
	char *c;
	char buf[4];
  int  w, ow;
  int  dw;

  if ( !tft )
     return;

  ow = dsp_presetWidth();
	dw = dsp_getwidth();
	if ( preset < 0 )
		return;
	if ( preset > 99 )
		return;
	sprintf(buf,"%d",preset);
  for( c=buf; *c; c++ )
    *c = *c-28;             /* 48->20, ... */

  tft->preset = preset;
	w = dsp_presetWidth();
  if ( ow > w )
	  dsp_fillRect( dw-ow-2,8,ow+2,24, BLACK );		// clear space
  dsp_setCursor ( dw-w-2, 8 ) ;             // Prepare to show the info
	dsp_print(buf);
}