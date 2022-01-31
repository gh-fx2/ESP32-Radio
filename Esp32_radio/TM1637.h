#include <driver/i2c.h>
#include <string.h>

#define DEFAULT_BIT_DELAY 			2000
#define DEFAULT_TM1637_BRIGHTNESS	0x0b  /* 0x00=off ( 0x80..0x87 = on ) */

#define SEG_A   0b00000001
#define SEG_B   0b00000010
#define SEG_C   0b00000100
#define SEG_D   0b00001000
#define SEG_E   0b00010000
#define SEG_F   0b00100000
#define SEG_G   0b01000000
#define SEG_DP  0b10000000

#define TM1637_I2C_COMM1    0x40
#define TM1637_I2C_COMM2    0xC0
#define TM1637_I2C_COMM3    0x80

//
//      A
//     ---
//  F |   | B
//     -G-
//  E |   | C
//     ---
//      D
const uint8_t digitToSegment[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00000110,    // 1
  0b01011011,    // 2
  0b01001111,    // 3
  0b01100110,    // 4
  0b01101101,    // 5
  0b01111101,    // 6
  0b00000111,    // 7
  0b01111111,    // 8
  0b01101111,    // 9
  0b01110111,    // A
  0b01111100,    // b
  0b00111001,    // C
  0b01011110,    // d
  0b01111001,    // E
  0b01110001     // F
  };

class TM1637Display
{
  public:
    TM1637Display ( uint8_t clk, uint8_t dio ) ;          // Constructor
    void      clear() ;                            // Clear buffer
    void      setSegments(const uint8_t *data, uint8_t len, uint8_t pos );
    uint16_t   m_timeBlocked;

  protected:
  void          bitDelay () ;
	void					start();
	void					stop();
	bool					writeByte( uint8_t b );
	void					showDots(uint8_t dots, uint8_t* digits);
  void          setBrightness( uint8_t bright ); // 0 = off, 1..8 = value
	void					showNumberBaseEx(int8_t base, uint16_t num, uint8_t dots = 0, bool leading_zero = false, uint8_t length = 4, uint8_t pos = 0);
  
  void          showNumberDec(int num, bool leading_zero, uint8_t length, uint8_t pos);
  void          showNumberDecEx(int num, uint8_t dots, bool leading_zero, uint8_t length, uint8_t pos);
  uint8_t       encodeDigit(uint8_t digit);

  private:
	uint8_t				m_pinClk;
	uint8_t				m_pinDIO;
	uint8_t				m_brightness;
	uint8_t				m_bitDelay;
	uint32_t			m_value;
	

  
} ;


TM1637Display* tm1637 = NULL ;

void tm1637_begin()
{
  dbgprint ( "Init TM1637, CLK,DIO pins %d,%d", ini_block.tm1637_clk_pin, ini_block.tm1637_dio_pin ) ;
  if ( ( ini_block.tm1637_clk_pin >= 0 ) &&
       ( ini_block.tm1637_dio_pin >= 0 ) )
  {
    tm1637 = new TM1637Display ( ini_block.tm1637_clk_pin, ini_block.tm1637_dio_pin ) ;
  }
}

//***********************************************************************************************
//                                  T M 1 6 3 7                                                 *
//***********************************************************************************************
// Constructor for the display.                                                                 *
//***********************************************************************************************
TM1637Display::TM1637Display ( uint8_t clk, uint8_t dio )
{
  uint8_t data[] = { 0, 0, 0, 0};
  m_pinClk = clk;
  m_pinDIO = dio;
  m_brightness = DEFAULT_TM1637_BRIGHTNESS;
  m_bitDelay = DEFAULT_BIT_DELAY;
  m_value = 0;
  m_timeBlocked = 0;

  pinMode( m_pinClk, INPUT);
  pinMode( m_pinDIO, INPUT);
  digitalWrite( m_pinClk, LOW );
  digitalWrite( m_pinDIO, LOW );

  setBrightness( dsp_brightness );
  clear();
}

void TM1637Display::setBrightness(uint8_t brightness )
{
  m_brightness = brightness ? (((brightness-1) & 0x7) | 0x08): 0;
}

void TM1637Display::setSegments(const uint8_t segments[], uint8_t length, uint8_t pos)
{
    // Write COMM1
    start();
    writeByte(TM1637_I2C_COMM1);
    stop();

    // Write COMM2 + first digit address
    start();
    writeByte(TM1637_I2C_COMM2 + (pos & 0x03));

    // Write the data bytes
    for (uint8_t k=0; k < length; k++)
      writeByte(segments[k]);

    stop();

    // Write COMM3 + brightness
    start();
    writeByte(TM1637_I2C_COMM3 + (m_brightness & 0x0f));
    stop();
}

void TM1637Display::clear()
{
    uint8_t data[] = { 0, 0, 0, 0 };
    setSegments(data,4,0);
}

void TM1637Display::bitDelay()
{
    delayMicroseconds(m_bitDelay);
}

void TM1637Display::start()
{
  pinMode(m_pinDIO, OUTPUT);
  bitDelay();
}

void TM1637Display::stop()
{
    pinMode(m_pinDIO, OUTPUT);
    bitDelay();
    pinMode(m_pinClk, INPUT);
    bitDelay();
    pinMode(m_pinDIO, INPUT);
    bitDelay();
}

bool TM1637Display::writeByte(uint8_t b)
{
  uint8_t data = b;

  // 8 Data Bits
  for(uint8_t i = 0; i < 8; i++) {
    // CLK low
    pinMode(m_pinClk, OUTPUT);
    bitDelay();

    // Set data bit
    if (data & 0x01)
      pinMode(m_pinDIO, INPUT);
    else
      pinMode(m_pinDIO, OUTPUT);

    bitDelay();

    // CLK high
    pinMode(m_pinClk, INPUT);
    bitDelay();
    data = data >> 1;
  }

  // Wait for acknowledge
  // CLK to zero
  pinMode(m_pinClk, OUTPUT);
  pinMode(m_pinDIO, INPUT);
  bitDelay();

  // CLK to high
  pinMode(m_pinClk, INPUT);
  bitDelay();
  uint8_t ack = digitalRead(m_pinDIO);
  if (ack == 0)
    pinMode(m_pinDIO, OUTPUT);


  bitDelay();
  pinMode(m_pinClk, OUTPUT);
  bitDelay();

  return ack;
}

uint8_t TM1637Display::encodeDigit(uint8_t digit)
{
    return digitToSegment[digit & 0x0f];
}

//**************************************************************************************************
//                                      D I S P L A Y T I M E                                      *
//**************************************************************************************************
// Show the time on the LCD at a fixed position in a specified color                               *
// To prevent flickering, only the changed part of the timestring is displayed.                    *
// An empty string will force a refresh on next call.                                              *
// A character on the screen is 8 pixels high and 6 pixels wide.                                   *
//**************************************************************************************************
void tm1637_displaytime ( const char* str )
{
  static char oldstr[9] = "........" ;             // For compare
  uint8_t     i,t, chg=0 ;  // Index in strings, index data
  uint8_t     showSec=0;
  uint8_t     data[] = { 0,0,0,0 };

  if ( !tm1637 || !dsp_show_time || !str || (strlen(str)<8))
    return;

  if ( tm1637->m_timeBlocked )
  {
    tm1637->m_timeBlocked--;
    strcpy(oldstr,"........");                     // display complete changed
    return;
  }

  if ( str[0] == '\0' )                            // Empty string?
  {
    for ( i = 0 ; i < 8 ; i++ )                    // Set oldstr to dots
    {
      oldstr[i] = '.' ;
    }
    return ;                                       // No actual display yet
  }

  showSec = str[7]&1;    // seconds are equal ?

  for ( i = 0, t=0 ; (i < 5) && (t< 4) ; i++ )                    // 1st 5 letters Compare old and new
  {
     uint8_t idx = str[i];
     if ( t && (idx == ':' ))
     {
       if ( showSec )
         data[t-1] |= 0x80;
     }
     else
     {
       if (( idx >= 48 ) && ( idx <= 57 ))
         data[t] = digitToSegment[idx-48];
       t++;
     }
     if ( oldstr[i] != str[i] )
       chg=1;
     oldstr[i] = str[i];
  }
  if ( chg )
    tm1637->setSegments( data, 4, 0 );
  else
    tm1637->setSegments( data+1, 1, 1 );  // double-point only
}

void tm1637_showPreset( int preset )
{
  char   *c;
  char   buf[16];
  int    i;
  uint8_t data[] = { 0, 0, 0, 0 };

  if ( !tm1637 )
     return;

  if ( preset < 0 )
  	return;
  if ( preset > 999 )  /* maybe IP */
    sprintf(buf,"%d___",(preset-1000));
  else
    sprintf(buf,"%4d",preset);
  for( i=0; i<4; i++ )
  {
    int idx = buf[i]-48;
    if (( idx >= 0 ) && ( idx <= 9 ))
      data[i] = digitToSegment[idx];
    else
      data[i] = 0;
  }
  if ( preset < 1000 )
    data[0] = 0x73; //         .111 ..11
  tm1637->setSegments(data,4,0);
  tm1637->m_timeBlocked = 10;  // show station for 10 seconds
}
