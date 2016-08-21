#include "sti_gnss_lib.h"
#include "GNSS.h"

/*
  NOTE: To initialize the UART console port with baud rate 19,200
*/
void setup() {
  // put your setup code here, to run once:
  GnssConf.setNavMode(STGNSS_NAV_MODE_AUTO);
  GnssConf.setUpdateRate(STGNSS_POSITION_UPDATE_RATE_2HZ);
  GnssConf.setDopMaskMode(STGNSS_DOP_MASK_AUTO);
  GnssConf.setPdopMask(30.0);
  GnssConf.setHdopMask(30.0);
  GnssConf.setGdopMask(30.0);
  GnssConf.init();
  
  
   Serial.config(STGNSS_UART_8BITS_WORD_LENGTH, STGNSS_UART_1STOP_BITS, STGNSS_UART_NOPARITY);
   Serial.begin(9600);/* do initialization for GNSS */
}

void loop() {
  // put your main code here, to run repeatedly:

}
