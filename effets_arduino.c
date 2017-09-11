/*
 * effets_arduino.c
 *
 *  Created on: 8 mar 2014
 *      Author: slawek
 */

/*
 * Na podstawie Adafruit NeoPixel library
 */

#include <avr/io.h>
#include <util/delay.h>
#include "ws2812.h"

static void delay(uint8_t wait)
{
    for(uint8_t d=0; d<wait; d++)
  	  _delay_ms(1);
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<ws2812_num_leds(); i++) {
      ws2812_set_color(i, c);
      ws2812_display();
      delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < ws2812_num_leds(); i=i+3) {
    	  ws2812_set_color(i+q, c);    //turn every third pixel on
      }
      ws2812_display();

      delay(wait);

      for (int i=0; i < ws2812_num_leds(); i=i+3) {
    	  ws2812_set_color(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(uint8_t WheelPos) {
  if(WheelPos < 85) {
   return ws2812_color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return ws2812_color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return ws2812_color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
        for (int i=0; i < ws2812_num_leds(); i=i+3) {
        	ws2812_set_color(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
        }
        ws2812_display();

        delay(wait);

        for (int i=0; i < ws2812_num_leds(); i=i+3) {
        	ws2812_set_color(i+q, 0);        //turn every third pixel off
        }
    }
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<ws2812_num_leds(); i++) {
    	ws2812_set_color(i, Wheel((i+j) & 255));
    }
    ws2812_display();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< ws2812_num_leds(); i++) {
    	ws2812_set_color(i, Wheel(((i * 256 / ws2812_num_leds()) + j) & 255));
    }
    ws2812_display();
    delay(wait);
  }
}

