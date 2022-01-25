// Default preferences in raw data format for PROGMEM
//
#define defaultprefs_version 1808016

#ifndef KA_PCB
// default edzelf ...

const char defprefs_txt[] PROGMEM = R"=====(
# Example configuration
# Programmable input pins:
#gpio_00 = uppreset = 1
#gpio_12 = upvolume = 1
#gpio_13 = downvolume = 1
#gpio_14 = stop
#gpio_17 = resume
#gpio_34 = station = icecast.omroep.nl:80/radio1-bb-mp3
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
volume = 11
toneha = 0
tonehf = 0
tonela = 0
tonelf = 0
#
preset = 0
# Some preset examples
preset_00 = icecast.ndr.de/ndr/ndrschlager/live/mp3/128/stream.mp3 # Ndr Schlager
preset_01 = 138.201.252.6/brf_128    # Brf 91.4
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
pin_enc_clk = 25                                     # GPIO Pin number for rotary encoder "CLK"
pin_enc_dt = 26                                      # GPIO Pin number for rotary encoder "DT"
pin_enc_sw = 27                                      # GPIO Pin number for rotary encoder "SW"
#
pin_tft_scl = -1                                     # GPIO Pin number for TFT "SCL" 22
pin_tft_sda = -1                                     # GPIO Pin number for TFT "SDA" 21
#
pin_sd_cs = -1                                       # GPIO Pin number for SD card "CS"
#
pin_vs_cs = 5                                        # GPIO Pin number for VS1053 "CS"
pin_vs_dcs = 32                                      # GPIO Pin number for VS1053 "DCS"
pin_vs_dreq = 4                                      # GPIO Pin number for VS1053 "DREQ"
#pin_shutdownx = 4     # kaRadio32-pcb
#pin_fixed_12 = 1      # kaRadio32-pcb
#
#touch_04 = uppreset = 1
#touch_05 = downpreset = 1
#
)=====" ;

#else

const char defprefs_txt[] PROGMEM = R"=====(
# Example configuration
# Programmable input pins:
#gpio_00 = uppreset = 1
#gpio_12 = upvolume = 1
#gpio_13 = downvolume = 1
#gpio_14 = stop
#gpio_17 = resume
#gpio_34 = station = icecast.omroep.nl:80/radio1-bb-mp3
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
volume = 11
toneha = 0
tonehf = 0
tonela = 0
tonelf = 0
#
preset = 0
# Some preset examples
preset_00 = icecast.ndr.de/ndr/ndrschlager/live/mp3/128/stream.mp3 # Ndr Schlager
preset_01 = 138.201.252.6/brf_128    # Brf 91.4
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
pin_enc_clk = 16                     # kaRadio32-pcb : GPIO Pin number for rotary encoder "CLK"
pin_enc_dt = 17                      # kaRadio32-pcb : GPIO Pin number for rotary encoder "DT"
pin_enc_sw = 5                       # kaRadio32-pcb : GPIO Pin number for rotary encoder "SW"
#
pin_tft_scl = -1                                      # GPIO Pin number for TFT "SCL" 14
pin_tft_sda = -1                                      # GPIO Pin number for TFT "SDA" 13
#
pin_sd_cs = -1                                       # GPIO Pin number for SD card "CS"
#
pin_vs_cs = 32                       # kaRadio32-pcb : GPIO Pin number for VS1053 "CS"
pin_vs_dcs = 33                      # kaRadio32-pcb : GPIO Pin number for VS1053 "DCS"
pin_vs_dreq = 34                     # kaRadio32-pcb : GPIO Pin number for VS1053 "DREQ"
pin_shutdownx = 4                    # kaRadio32-pcb : LED
pin_fixed_12 = 1                     # kaRadio32-pcb : RST-pin need HIGH
#
#
)=====" ;

#endif
