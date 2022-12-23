/*******************************************************************************

*******************************************************************************/

#ifndef GRUNDIG_BAY100_H_
#define GRUNDIG_BAY100_H_

typedef struct _grundigBay100TimedText
{
  uint8_t   data[9];
  uint8_t   nbytes;
  uint32_t  milliDone;
  uint8_t   kind;
} grundigBay100TimedText;

typedef struct _grundigBay100Timed
{
  uint8_t        tFill;
  grundigBay100TimedText tText[10];
} grundigBay100Timed;

void HT1621Display::grundigBay100_addTimed( uint8_t *data, uint32_t ms, uint8_t kind, uint8_t nbytes )
{
  grundigBay100Timed *t = (grundigBay100Timed*)_timed;
  uint8_t  rpl = 0;
  uint8_t  idx;
  if ( !t )
  {
    t = (grundigBay100Timed*)malloc(sizeof(grundigBay100Timed));
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
  memcpy(t->tText[idx].data,data,nbytes);
  t->tText[idx].nbytes = nbytes;
  t->tText[idx].milliDone = ms;
  t->tText[idx].kind = kind;
  if ( !rpl )
    t->tFill++;

  if ( !idx )
  {
    t->tText[0].milliDone += millis();
    update(data,nbytes);
  }
}

static const uint16_t  g_nummap[] =
{   0xca60 ,   // 0
    0x0060 ,   // 1
    0x8e20 ,   // 2
    0x0e60 ,   // 3
    0x4460 ,   // 4
    0x4e40 ,   // 5
    0xce40 ,   // 6
    0x0260 ,   // 7
    0xce60 ,   // 8
    0x4e60     // 9
};

void grundigBay100_putTimeNum( int idx, int cpos, uint8_t *data )
{
  uint16_t v = g_nummap[idx];
  uint8_t *p = (uint8_t*)&v;

  switch( cpos )
  {
    case 0 :
      if ( idx == 1 )
        data[1] = 0x0C;
      if ( idx == 2 )
        data[1] = 0x84;
      break;
    case 1 :
      data[2] |= p[1];
      data[3] |= p[0];
      break;
    case 2 :
      v >>= 4;   // 6e60  -> 06e6
      data[3] |= p[1];
      data[4] |= p[0];
      break;
    case 3 :
      data[5] |= p[1];
      data[6] |= p[0];
      break;
  }
}

void grundigBay100_putPosNum( int idx, int cpos, uint8_t *data )
{
  uint16_t v = g_nummap[idx];
  uint8_t *p = (uint8_t*)&v;

  switch( cpos )
  {
    case 0 :
      if ( idx == 1 )
        data[0] = 0x20;
      break;
    case 1 :
      data[0] |= p[1];
      data[1] |= p[0];
      break;
  }
}

void grundigBay100_fillTimeData( const char *buf, uint8_t *data )
{
  int i;
  memset(data,0,7);
  for( i=0; i<4; i++ )
  {
    int idx = buf[i]-48;
    if (( idx >= 0 ) && ( idx <= 9 ))
      grundigBay100_putTimeNum( idx, i, data );
    else if (( buf[i] == '.' ) && ( i == 3 ))
    {
      data[4] |= 8;
      break;
    }
    else if ( buf[i] == '.' )
    {
      data[3] |= 0x80;
      break;
    }
  }
}

void grundigBay100_fillPosData( const char *buf, uint8_t *data )
{
  int i;
  memset(data,0,2);
  for( i=0; i<2; i++ )
  {
    int idx = buf[i]-48;
    if (( idx >= 0 ) && ( idx <= 9 ))
      grundigBay100_putPosNum( idx, i, data );
  }
}

void HT1621Display::grundigBay100_showIP( const char *ip )
{
  uint8_t  data[9];
  const char     *c;
  int      i;

  memset(data,0,9);
// IP:
  data[1] = 0x0E;
  data[2] = 0xC6;
  data[3] = 0x20;
  grundigBay100_addTimed( data, 2000, 0, 7 );

  grundigBay100_fillTimeData( ip, data );     /* 1st. num */
  grundigBay100_addTimed( data, 1000, 1, 7 );
  c=ip;
  for( i=0; i<3; i++ )
  {
    c=strchr(c+1,'.');
    if ( !c )
      break;
    grundigBay100_fillTimeData( c+1, data );
    if ( i==2 )
      data[6] |= 2;
    grundigBay100_addTimed( data, 1000, i+2, 7 );
  }
}

void HT1621Display::grundigBay100_showPreset( int preset )
{
  uint8_t  data[9];
  char     *c;
  char     buf[6];

  sprintf(buf,"%2d",preset);

  grundigBay100_fillPosData( buf, data );
  update( data, 7*8, 2 );
}

void HT1621Display::grundigBay100_displayTime ( const char* str )
{
  static char oldstr[9] = "........" ;             // For compare
  uint8_t     i,n, chg=0 ;  // Index in strings, index data
  uint8_t     showSec=0;
  uint8_t     data[] = { 0,0,0,0,0,0,0 };

  if ( !_timed )
    return;
  grundigBay100Timed *t = (grundigBay100Timed*)_timed;
  if ( t->tFill )
    grundigBay100_loop();

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
         data[1] |= 0x02;
     }
     else
     {
       if (( idx >= 48 ) && ( idx <= 57 ))
       {
         grundigBay100_putTimeNum( idx-48, n, data );
       }
       n++;
     }
     if ( oldstr[i] != str[i] )
       chg=1;
     oldstr[i] = str[i];
  }
  data[6] |=2;
  update( data, 7 );
}

void HT1621Display::grundigBay100_infoUpdate( void )
{
  uint8_t data[] = { 0, 0, 0xC8, 0x6C, 0x62, 0x8c, 0xe0, 0,0 };  // UPd
  grundigBay100Timed *t = (grundigBay100Timed*)_timed;

  if ( t )
    t->tFill = 0;
  grundigBay100_addTimed( data, 20000, 50, 9 );
}

void HT1621Display::grundigBay100_loop( void )
{
  if ( !_timed )
    return;
  grundigBay100Timed *t = (grundigBay100Timed*)_timed;
  if ( !t->tFill )
    return;
 if ( t->tText[0].milliDone > millis() )  // wait
   return;
 t->tFill--;
 if ( t->tFill )
 {
   memcpy(t->tText,t->tText+1,t->tFill*sizeof(grundigBay100TimedText));
   t->tText[0].milliDone += millis();
   update(t->tText[0].data+1,8,t->tText[0].nbytes-1);
 }
}
#endif
