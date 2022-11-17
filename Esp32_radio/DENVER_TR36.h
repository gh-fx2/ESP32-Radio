/*******************************************************************************

*******************************************************************************/

#ifndef DENVER_TR36_H_
#define DENVER_TR36_H_

typedef struct _denverTr36TimedText
{
  uint8_t   data[6];
  uint32_t  milliDone;
  uint8_t   kind;
} denverTr36TimedText;

typedef struct _denverTr36Timed
{
  uint8_t        tFill;
  denverTr36TimedText tText[10];
} denverTr36Timed;

void HT1621Display::denverTr36_addTimed( uint8_t *data, uint32_t ms, uint8_t kind )
{
  denverTr36Timed *t = (denverTr36Timed*)_timed;
  uint8_t  rpl = 0;
  uint8_t  idx;
  if ( !t )
  {
    t = (denverTr36Timed*)malloc(sizeof(denverTr36Timed));
    t->tFill = 0;
    _timed = t;
  }
  for( idx=0; idx < t->tFill; idx++ )
  {
    if ( t->tText[idx].kind == kind )
    {
      rpl=1;
      break;
    }
  }
  if ( idx == 10 )
    return;
  memcpy(t->tText[idx].data,data,6);
  t->tText[idx].milliDone = ms;
  t->tText[idx].kind = kind;
  if ( !rpl )
    t->tFill++;

  if ( !idx )
  {
    t->tText[0].milliDone += millis();
    update(data,6);
  }
}

static const uint16_t  nummap[] =
{   0x06a6 ,   // 0
    0x0006 ,   // 1
    0x04e2 ,   // 2
    0x00e6 ,   // 3
    0x0246 ,   // 4
    0x02e4 ,   // 5
    0x06e4 ,   // 6
    0x0026 ,   // 7
    0x06e6 ,   // 8
    0x02e6     // 9
};

void denverTr36_putNum( int idx, int cpos, uint8_t *data )
{
  uint16_t v = nummap[idx];
  uint8_t *p = (uint8_t*)&v;

  switch( cpos )
  {
    case 0 :
      v <<= 4;
      data[0] |= p[1];
      data[1] |= p[0];
      break;
    case 1 :
      data[1] |= p[1];
      data[2] |= p[0];
      break;
    case 2 :
      v <<= 4;   // 06e6  -> 6e60
      data[3] |= p[1];
      data[4] |= p[0];
      break;
    case 3 :
      data[4] |= p[1];
      data[5] |= p[0];
      break;
  }
}

void denverTr36_fillData( const char *buf, uint8_t *data )
{
  int i;
  memset(data,0,6);
  for( i=0; i<4; i++ )
  {
    int idx = buf[i]-48;
    if (( idx >= 0 ) && ( idx <= 9 ))
      denverTr36_putNum( idx, i, data );
    else if (( buf[i] == '.' ) && ( i == 3 ))
    {
      data[4] |= 8;
      break;
    }
    else if ( buf[i] == '.' )
    {
      data[4] |= 128;
      break;
    }
  }
}

void HT1621Display::denverTr36_showIP( const char *ip )
{
  uint8_t  data[6];
  const char     *c;
  int      i;

// IP:
  data[0] = 0x60;
  data[1] = 0x06;
  data[2] = 0x62;
  data[3] = 0x80;
  data[4] = 0;
  data[5] = 0;
  denverTr36_addTimed( data, 2000, 0 );

  denverTr36_fillData( ip, data );     /* 1st. num */
  denverTr36_addTimed( data, 1000, 1 );
  c=ip;
  for( i=0; i<3; i++ )
  {
    c=strchr(c+1,'.');
    if ( !c )
      break;
    denverTr36_fillData( c+1, data );
    denverTr36_addTimed( data, 1000, i+2 );
  }
}

void HT1621Display::denverTr36_showPreset( int preset )
{
  uint8_t  data[6];
  char     *c;
  char     buf[6];

  sprintf(buf,"%4d",preset);

  denverTr36_fillData( buf, data );

  data[0] = 0x66;
  data[1] = 0x20;

  denverTr36_addTimed( data, 5000, 99 );
}

void HT1621Display::denverTr36_displayTime ( const char* str )
{
  static char oldstr[9] = "........" ;             // For compare
  uint8_t     i,n, chg=0 ;  // Index in strings, index data
  uint8_t     showSec=0;
  uint8_t     data[] = { 0,0,0,0,0,0 };

  if ( !_timed )
    return;
  denverTr36Timed *t = (denverTr36Timed*)_timed;
  if ( t->tFill )
    denverTr36_loop();

  if ( t->tFill )
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

  for ( i = 0, n=0 ; (i < 5) && (n< 4) ; i++ )                    // 1st 5 letters Compare old and new
  {
     uint8_t idx = str[i];
     if ( n && (idx == ':' ))
     {
       if ( showSec )
         data[3] |= 0x80;
     }
     else
     {
       if (( idx >= 48 ) && ( idx <= 57 ))
         denverTr36_putNum( idx-48, n, data );

       n++;
     }
     if ( oldstr[i] != str[i] )
       chg=1;
     oldstr[i] = str[i];
  }
  if ( chg )
    update( data, 6 );
  else
    update( data, 4 );
}

void HT1621Display::denverTr36_infoUpdate( void )
{
  uint8_t data[] = { 0b01101000, 0b01100110, 0b01100010, 0b01001100, 0b01101000, 0b00000000 };  // UPd
  denverTr36Timed *t = (denverTr36Timed*)_timed;
  if ( t )
    t->tFill = 0;
  denverTr36_addTimed( data, 20000, 50 );
}

void HT1621Display::denverTr36_loop( void )
{
  if ( !_timed )
    return;
  denverTr36Timed *t = (denverTr36Timed*)_timed;
  if ( !t->tFill )
    return;
 if ( t->tText[0].milliDone > millis() )  // wait
   return;
 t->tFill--;
 if ( t->tFill )
 {
   memcpy(t->tText,t->tText+1,t->tFill*sizeof(denverTr36TimedText));
   t->tText[0].milliDone += millis();
   update(t->tText[0].data,6);
 }
}
#endif
