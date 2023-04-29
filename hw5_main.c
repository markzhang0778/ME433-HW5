#include "nu32dip.h" // constants, functions for startup and UART
#include <stdio.h>
#include <math.h>
#include "spi.h"

#define CS LATAbits.LATA3
#define baud 24000

void send_spi(unsigned char ch, unsigned int v);        // sends the data

int main(void) {
  NU32DIP_Startup();
  initSPI();
  _CP0_SET_COUNT(0);
  int ticks = 0;
  double t = 0.0;
  
  //CP0 core timer counts up 24000000 times per second
  while (1) {
      if(_CP0_GET_COUNT() > 24000000){_CP0_SET_COUNT(0);}
      ticks = _CP0_GET_COUNT();
      t = (double)ticks / 24000000;                     //0 < t < 1 sec
      
      //Calculate voltage for sine wave
      double V_s = 1.65 * sin(4 * M_PI * t) + 1.65;     
      unsigned int s = (V_s / 3.3) * 1023;            //convert V to 10 bit
      //write UART to debug
      //char msg[20];
      //sprintf(msg, "%i %f %f, %i\r\n",ticks, t, V_s, s);
      //NU32DIP_WriteUART1(msg);
      
      //get voltage for triangle wave
      double V_t = 0;
      if(t <= 0.5){
          //ascending
          V_t = 2 * 3.3 * t;
      }
      else{
          V_t = 6.6 - (2 * 3.3 * t);
      }
      unsigned int tri = (V_t/3.3)*1023;
      
      //send the voltages to DAC using SPI function
      send_spi(0, s);           //ch = 0 for channel A
      send_spi(1, tri);
      
      while((_CP0_GET_COUNT() - ticks) < 24000){
          //delay, send data 1000 times per second
          //wait 24000 ticks to send next value
          ;
      }
      
        
  }
}
		
void send_spi(unsigned char ch, unsigned int v){
    unsigned short t = 0;
    t = 0b111 << 12;
    t = t | (ch << 15);
    t = t | (v << 2);
    CS = 0; //latch CS (A3) low to start

    //send the first(?) byte
    spi_io(t >> 8);

    //send the remaining
    spi_io(t);

    CS = 1; //latch CS (A3) high to end
}