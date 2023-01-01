/*******************************************************************************

*******************************************************************************/

#ifndef HT1621_H_
#define HT1621_H_

#define  BIAS43     0x52           //0b1000 0101 0010  1/3duty 4com
#define  BIAS33     0x5A           //0b1000 0101 0010  1/3duty 3com
#define  BIAS32     0x58           //0b1000 0101 0010  1/2duty 3com
#define  BIAS     0x52           //0b1000 0101 0010  1/3duty 4com
#define  SYSDIS   0X00             //0b1000 0000 0000  
#define  SYSEN    0X02             //0b1000 0000 0010 
#define  LCDOFF   0X04             //0b1000 0000 0100
#define  LCDON    0X06             //0b1000 0000 0110
#define  XTAL     0x28             //0b1000 0010 1000
#define  RC256    0X30             //0b1000 0011 0000
#define  TONEON   0X12             //0b1000 0001 0010
#define  TONEOFF  0X10             //0b1000 0001 0000
#define  WDTDIS1  0X0A             //0b1000 0000 1010

#define  BUFFERSIZE 16

#define HT1621_T_VIM878		1
#define HT1621_T_DENVER_TR36		2
#define HT1621_T_CHECK    3
#define HT1621_T_GRUNDIG_BOY100		4

typedef void (*t1Proc)( char *p1 );

class  HT1621Display
{
public:
	HT1621Display( int8_t cs_p, int8_t wr_p, int8_t data_p, uint8_t t );
  void loop();
  int _type;

  void vim878_showIP( const char *in );
  void vim878_loop( void );
  void denverTr36_showIP( const char *in );
  void denverTr36_showPreset( int preset );
  void denverTr36_displayTime ( const char* str );
  void denverTr36_loop( void );
  void denverTr36_addTimed( uint8_t *data, uint32_t ms, uint8_t kind );
  void denverTr36_infoUpdate( void );
  void grundigBoy100_showIP( const char *in );
  void grundigBoy100_showPreset( int preset );
  void grundigBoy100_displayTime ( const char* str );
  void grundigBoy100_loop( void );
  void grundigBoy100_addTimed( uint8_t *data, uint32_t ms, uint8_t kind, uint8_t nbytes );
  void grundigBoy100_infoUpdate( void );

private:
	int _cs_p;
	int _wr_p;
	int _data_p;
  uint8_t _is_alpha;
	void wrDATA(unsigned char data, unsigned char cnt);
	void wrCMD(unsigned char CMD);
	void config(); // legacy: why not in begin func
	void update( uint8_t *buffer, int num_bytes );
  void update( uint8_t *buffer, int off, int num_bytes );
  void *_timed;
};


HT1621Display* ht1621 = NULL;

void ht1621_begin( int8_t cs, int8_t wr, int8_t data, uint8_t type )
{
  if (( cs >= 0 ) && ( wr >= 0 ) && ( data >= 0 ) && type )
  {
    ht1621 = new HT1621Display( cs, wr, data, type );
  }
}

void HT1621Display::wrCMD(unsigned char CMD) {  //100
	digitalWrite(_cs_p, LOW);
	wrDATA(0x80, 4);
	wrDATA(CMD, 8);
	digitalWrite(_cs_p, HIGH);
}

void HT1621Display::config()
{
	wrCMD(BIAS);
	wrCMD(RC256);
	wrCMD(SYSDIS);
	wrCMD(WDTDIS1);
	wrCMD(SYSEN);
	wrCMD(LCDON);
}

HT1621Display::HT1621Display(int8_t cs_p, int8_t wr_p, int8_t data_p, uint8_t t)
{
  uint8_t data[] = { 0,0,0,0,0,0,0,0
                      ,0,0,0,0,0,0,0,0
                   };
	pinMode(cs_p, OUTPUT);
	pinMode(wr_p, OUTPUT);
	pinMode(data_p, OUTPUT);
	_cs_p=cs_p;
	_wr_p=wr_p;
	_data_p=data_p;
	_type = t;
  _is_alpha = 0;
  _timed = 0;
  if ( t == HT1621_T_VIM878 )
    _is_alpha = 1;
	config();
 
 if ( t == HT1621_T_DENVER_TR36 )
 {
   data[0] = 0x04;
   data[2] = 0x40;
   data[3] = 0x04;
   data[5] = 0x40;
   update(data,8);
 }
 else if ( t == HT1621_T_GRUNDIG_BOY100 )
 {
   data[2] = 0x04;
   data[4] = 0x40;    // 0x84 = 2,  0x0C = 1
   data[5] = 0x04;
   update(data,16);
 }
 else if ( t == HT1621_T_CHECK )
 {
   memset(data,255,16);
   update(data,16);
 }
}

void HT1621Display::wrDATA(unsigned char data, unsigned char cnt) {
	unsigned char i;
	for (i = 0; i < cnt; i++) {
		digitalWrite(_wr_p, LOW);
		delayMicroseconds(4);
		if (data & 0x80) {
			digitalWrite(_data_p, HIGH);
		}
		else {
			digitalWrite(_data_p, LOW);
		}
		digitalWrite(_wr_p, HIGH);
		delayMicroseconds(4);
		data <<= 1;
	}
}

void HT1621Display::update( uint8_t *buffer, int num_bytes )
{
  int i;
  unsigned char addr = 0;
  digitalWrite(_cs_p, LOW);
  wrDATA(0xa0, 3);
  wrDATA(addr, 6);
  for( i=0; i<num_bytes; i++ )
     wrDATA(buffer[i], 8 );
  digitalWrite(_cs_p, HIGH);
}

void HT1621Display::update( uint8_t *buffer, int off, int num_bytes )
{
  int i;
  unsigned char addr = off*8;
  digitalWrite(_cs_p, LOW);
  wrDATA(0xa0, 3);
  wrDATA(addr, 6);
  for( i=0; i<num_bytes; i++ )
     wrDATA(buffer[i], 8 );
  digitalWrite(_cs_p, HIGH);
}

void ht1621_loop( void )
{
  if ( !ht1621 )
    return;
  switch( ht1621->_type )
  {
  case HT1621_T_VIM878 :
      ht1621->vim878_loop( );
      break;
  case HT1621_T_DENVER_TR36 :
      ht1621->denverTr36_loop( );
      break;
  case HT1621_T_GRUNDIG_BOY100 :
      ht1621->grundigBoy100_loop( );
      break;
  }
}

void ht1621_showIP( const char *ip )
{
  if ( !ht1621 )
    return;
  switch( ht1621->_type )
  {
  case HT1621_T_VIM878 :
      ht1621->vim878_showIP( ip );
      break;
  case HT1621_T_DENVER_TR36 :
      ht1621->denverTr36_showIP( ip );
      break;
  case HT1621_T_GRUNDIG_BOY100 :
      ht1621->grundigBoy100_showIP( ip );
      break;
  }
}

void ht1621_displayTime( const char *str )
{
  if ( !ht1621 || !dsp_show_time || !str || (strlen(str)<8))
    return;
  if ( ht1621->_type == HT1621_T_DENVER_TR36 )
    ht1621->denverTr36_displayTime( str );
  else if ( ht1621->_type == HT1621_T_GRUNDIG_BOY100 )
    ht1621->grundigBoy100_displayTime( str );
}

void ht1621_showPreset( int preset )
{
  if ( !ht1621 )
    return;
  if ( ht1621->_type == HT1621_T_DENVER_TR36 )
    ht1621->denverTr36_showPreset( preset );
  else if ( ht1621->_type == HT1621_T_GRUNDIG_BOY100 )
    ht1621->grundigBoy100_showPreset( preset );
}

void ht1621_infoUpdate( void )
{
  if ( !ht1621 )
    return;
  if ( ht1621->_type == HT1621_T_DENVER_TR36 )
    ht1621->denverTr36_infoUpdate( );
  else if ( ht1621->_type == HT1621_T_GRUNDIG_BOY100 )
    ht1621->grundigBoy100_infoUpdate( );
}

#include "VIM878.h"
#include "DENVER_TR36.h"
#include "GRUNDIG_BOY100.h"

#endif
