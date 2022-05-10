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
const uint8_t digitToSegmentRotated[] = {
 // XGFEDCBA
  0b00111111,    // 0
  0b00110000,    // 1
  0b01011011,    // 2
  0b01111001,    // 3
  0b01110100,    // 4
  0b01101101,    // 5
  0b01101111,    // 6
  0b00111000,    // 7
  0b01111111,    // 8
  0b01111101,    // 9
  0b01111110,    // A
  0b01100111,    // b
  0b00001111,    // C
  0b01110011,    // d
  0b01001111,    // E
  0b01001110     // F
  };

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

typedef struct _timedText
{
   uint8_t   data[4];
   uint32_t  milliDone;
   uint8_t   kind;          /* same kind will replaced */
} timedText;

class TM1637Display
{
  public:
    TM1637Display ( uint8_t clk, uint8_t dio ) ;          // Constructor
    void      clear() ;                            // Clear buffer
    void      setSegments(const uint8_t *data, uint8_t len, uint8_t pos );
    void      addTimed( uint8_t *data, uint16_t ms, uint8_t kind );
    void      checkttFill();
    timedText	m_tText[10];
    int         m_ttFill;

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
  uint8_t data[] = { 0x40, 0x40, 0x40, 0x40 };
  m_pinClk = clk;
  m_pinDIO = dio;
  m_brightness = DEFAULT_TM1637_BRIGHTNESS;
  m_bitDelay = DEFAULT_BIT_DELAY;
  m_value = 0;
  m_ttFill = 0;

  pinMode( m_pinClk, INPUT);
  pinMode( m_pinDIO, INPUT);
  digitalWrite( m_pinClk, LOW );
  digitalWrite( m_pinDIO, LOW );

  setBrightness( dsp_brightness );
  setSegments(data,4,0);
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
void tm1637_displayTime ( const char* str )
{
  static char oldstr[9] = "........" ;             // For compare
  uint8_t     i,t, chg=0 ;  // Index in strings, index data
  uint8_t     showSec=0;
  uint8_t     data[] = { 0,0,0,0 };

  if ( !tm1637 || !dsp_show_time || !str || (strlen(str)<8))
    return;

  if ( tm1637->m_ttFill )
		tm1637->checkttFill();

  if ( tm1637->m_ttFill )
  {
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
         data[tm1637_rotate ? t : t-1] |= 0x80;
     }
     else
     {
       if (( idx >= 48 ) && ( idx <= 57 ))
         data[t] = tm1637_rotate ? digitToSegmentRotated[idx-48] : digitToSegment[idx-48];
       t++;
     }
     if ( oldstr[i] != str[i] )
       chg=1;
     oldstr[i] = str[i];
  }
  if ( chg )
    tm1637->setSegments( data, 4, 0 );
  else
    tm1637->setSegments( data+1+(tm1637_rotate?1:0), 1, 1 );  // double-point only
}

void tm1637_fillData( const char *buf, uint8_t *data )
{
  int i;

  for( i=0; i<4; i++ )
  {
    int idx = buf[i]-48;
    if ( tm1637_rotate )
    {
      if (( idx >= 0 ) && ( idx <= 9 ))
        data[3-i] = digitToSegmentRotated[idx];
      else
        data[3-i] = 0;
      if (( buf[i] == '.' ) && (i<3))
      {
        memset(data,0,3-i);
        break;
      }
    }
    else
    {
      if (( idx >= 0 ) && ( idx <= 9 ))
        data[i] = digitToSegment[idx];
      else
        data[i] = 0;
      if (( buf[i] == '.' ) && (i<3))
      {
        memset(data+i+1,0,3-i);
		    break;
	    }
    }
  }
}

void TM1637Display::addTimed( uint8_t *data, uint16_t ms, uint8_t kind )
{
	uint8_t    idx = m_ttFill;
    uint8_t    replace = 0;

    for( idx=0; idx<m_ttFill; idx++ )
    {
      if ( m_tText[idx].kind == kind )
      {
        replace=1;
		break;
	  }
    }
	if ( idx == 10 )
	  return;
  memcpy(m_tText[idx].data,data,4);
	m_tText[idx].milliDone = ms;
  m_tText[idx].kind = kind;
    if ( !replace )
	  m_ttFill++;

	if ( !idx )	/* 1st one */
	{
	    m_tText[0].milliDone += millis();
  		tm1637->setSegments(data,4,0);
	}
}

void TM1637Display::checkttFill( )
{
  if ( !m_ttFill )
    return;
  if ( m_tText[0].milliDone > millis() )  /* not done */
    return;

  m_ttFill--;
  if ( m_ttFill )
  {
    memcpy(m_tText,m_tText+1,m_ttFill*sizeof(timedText));
  	m_tText[0].milliDone += millis();
    setSegments(m_tText[0].data,4,0);
  }
}

void tm1637_showIP( const char *ip )
{
  const char   *c;
  int    i;
  char   buf[16];
  uint8_t data[] = { 0, 0, 0, 0 };

  if ( !tm1637 )
     return;

  if ( tm1637_rotate )
  {
  data[3] = 0b00000110;     // I
  data[2] = 0b01011110;     // P
  data[1] = 0b10000000;     // :
  }
  else
  {
  data[0] = 0b00110000;     // I
  data[1] = 0b11110011;     // P:
  }

  tm1637->addTimed( data, 2000, 0 );

  tm1637_fillData( ip, data );  /* 1st num */
  tm1637->addTimed( data, 1000, 1 );

  c=ip;
  for( i=0; i<3; i++ )
  {
    c=strchr(c+1,'.');
    if ( !c )
     break;
    tm1637_fillData( c+1, data );  /* next num */
    tm1637->addTimed( data, 1000,i+2 );
  }
}

void tm1637_showPreset( int preset )
{
  char   *c;
  char   buf[16];
  uint8_t data[] = { 0, 0, 0, 0 };

  if ( !tm1637 )
     return;

  if ( preset < 0 )
  	return;
  sprintf(buf,"%4d",preset);

  tm1637_fillData( buf, data );

  if ( tm1637_rotate )
    data[3] = 0b01011110; //  1.letter = P :       .1.1 111.
  else
    data[0] = 0b01110011; //  1.letter = P :       .111 ..11

  tm1637->addTimed( data, 5000, 99 );
}

void tm1637_infoUpdate( void )
{
  uint8_t data[] = { 0, 0, 0, 0 };

  if ( !tm1637 )
    return;

  if ( tm1637_rotate )
  {
    data[3] = 0b00110111; //    letter = U :    ..11 .111
    data[2] = 0b01011110; //    letter = P :    .1.1 111.
    data[1] = 0b01110011; //    letter = d :    .111 ..11
  }
  else
  {
    data[0] = 0b00111110; //    letter = U :    ..11 111.
    data[1] = 0b01110011; //    letter = P :    .111 ..11
    data[2] = 0b01011110; //    letter = d :    .1.1 111.
  }
  tm1637->m_ttFill = 0;   // remove all other text
  tm1637->addTimed( data, 20000, 50 );
}

void tm1637_loop( void )
{
  if ( !tm1637 )
     return;

	tm1637->checkttFill();
}
