# AVR-WS8212-test

Simple program to test AVR driver for WS8212 LEDs. Run on Atmega328/168 and probably on any ~~newer~~ atmega chips. ~~Doesn't run on atmega8/16/32 etc.~~
One should modify ws2812_syms.h file to acomodate one's hardware configuration. If compiled under Eclipse AVR plugin it's necessary to define proper F_CPU in ws2812_syms.h file.
Only one of files main.c, main_0.c and main_1.c is to be included into build.
Comments aren't in english :-/. Sorry.
