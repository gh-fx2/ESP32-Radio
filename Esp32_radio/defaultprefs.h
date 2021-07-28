// Default preferences in raw data format for PROGMEM
//
#define defaultprefs_version 1808016
const char defprefs_txt[] PROGMEM = R"=====(
# Example configuration
# Programmable input pins:
gpio_00 = uppreset = 1
gpio_12 = upvolume = 1
gpio_13 = downvolume = 1
gpio_14 = stop
gpio_17 = resume
gpio_34 = station = icecast.omroep.nl:80/radio1-bb-mp3
#
# MQTT settings
mqttbroker = none
mqttport = 1883
mqttuser = none
mqttpasswd = none
mqqprefix = none
# Enter your WiFi network specs here:
wifi_01 = Sprite/WwidK25aiFR
wifi_02 = Sprite2G/WwidK25aiFR
wifi_03 = marconet/p0tzb11tz
#
volume = 72
toneha = 0
tonehf = 0
tonela = 0
tonelf = 0
#
preset = 0
# Some preset examples
preset_00 = br-br1-obb.cast.addradio.de/br/br1/obb/mp3/128/stream.mp3
preset_01 = pool.radiopaloma.de/RADIOPALOMA.mp3?context=fHA6LTF=
preset_02 = br-brschlager-live.cast.addradio.de/br/brschlager/live/mp3/mid
preset_03 = antenneunna-ais-edge-3007-fra-eco-cdn.cast.addradio.de/antenneunna/live/mp3/high
preset_04 = ndr-ndr1radiomv-schwerin.cast.addradio.de/ndr/ndr1radiomv/schwerin/mp3/128/stream.mp3
preset_05 = mdr-284290-0.cast.mdr.de/mdr/284290/0/mp3/high/stream.mp3
preset_06 = 138.201.252.6/brf_128
preset_07 = wdr-wdr4-live.icecast.wdr.de/wdr/wdr4/live/mp3/128/stream.mp3
preset_08 = stream.absolutradio.de/relax/mp3-128/surfmusik/
#
# Clock offset and daylight saving time
clk_server = pool.ntp.org                            # Time server to be used
clk_offset = 1                                       # Offset with respect to UTC in hours
clk_dst = 1                                          # Offset during daylight saving time (hours)
# Some IR codes
ir_40BF = upvolume = 1
ir_C03F = downvolume = 1
# GPIO pinnings
pin_ir = -1                                          # GPIO Pin number for IR receiver VS1838B
pin_enc_clk = 25                                     # GPIO Pin number for rotary encoder "CLK" 25
pin_enc_dt = 26                                      # GPIO Pin number for rotary encoder "DT" 26
pin_enc_sw = 27                                      # GPIO Pin number for rotary encoder "SW" 27
#
pin_tft_cs = -1                                      # GPIO Pin number for TFT "CS"
pin_tft_dc = -1                                       # GPIO Pin number for TFT "DC"
#
pin_sd_cs = -1                                       # GPIO Pin number for SD card "CS"
#
pin_vs_cs = 5                                        # GPIO Pin number for VS1053 "CS"
pin_vs_dcs = 32                                      # GPIO Pin number for VS1053 "DCS"
pin_vs_dreq = 4                                      # GPIO Pin number for VS1053 "DREQ"
#
touch_04 = uppreset = 1
touch_05 = downpreset = 1
#
)=====" ;
