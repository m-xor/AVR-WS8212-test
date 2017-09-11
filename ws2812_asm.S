/*
 *	ws2812_asm.S
 *
 *	Wysyła bufor na wyjście wg protokołu układu WS2811
 *	Wielkość kodu dla częstotliwości taktowania (w bajtach)
 *	(m168p):
 *	8MHz	128
 *	10MHz	156 (+28)
 *	16MHz	210 (+54)
 *	20MHz	236 (+26)
 *	24MHz	278 (+42)
 *
 *	Czas wykonania:
 *	30µs * ilość diod (albo 10µs na bajt bufora) + 15 (ostatni bit ==0) lub 18 taktów (ostatni bit ==1)
 */

#define __SFR_OFFSET 0	//pomniejsza adresy I/O o 0x20

#include <avr/io.h>
#include "ws2812_syms.h"


/*
 * Dane do instrukcji opóźniających. Wyjściowo timingi są dla częstotl. 8MHz,
 * dostosowanie do innych częstotliwości odbywa się przez wstawienie
 * odpowiedniej ilości intstrukcji opóźniających (nop, rjmp)
 * Procedura z OUT przetestowana dla m168 przy 8,10,20,24 MHz.
 * Procedura z SBI/CBI przetest. dla m168/8MHz
 */
#if F_CPU == 8000000UL
		/* 8MHz */
	#define H0_DELAY	0
	#define L0_DELAY	0
	#define H1_DELAY	0
	#define L1_DELAY	0
#elif F_CPU == 10000000UL
		/* 10MHz */
	#define H0_DELAY	1
	#define L0_DELAY	2
	#define H1_DELAY	1
	#define L1_DELAY	2
#elif F_CPU == 11059200UL
	#define H0_DELAY	1
	#define L0_DELAY	3
	#define H1_DELAY	2
	#define L1_DELAY	2
#elif F_CPU == 12000000UL
	#define H0_DELAY	1
	#define L0_DELAY	4
	#define H1_DELAY	2
	#define L1_DELAY	3
#elif F_CPU == 14745600UL
	#define H0_DELAY	2
	#define L0_DELAY	6
	#define H1_DELAY	4
	#define L1_DELAY	4
#elif F_CPU == 16000000UL
		/* 16MHz */
	#define H0_DELAY 	3
	#define L0_DELAY	7
	#define H1_DELAY	5
	#define L1_DELAY	5
#elif F_CPU == 18432000UL
	#define H0_DELAY	3
	#define L0_DELAY	10
	#define H1_DELAY	7
	#define L1_DELAY	6
#elif F_CPU == 20000000UL
		/* 20MHz */
	#define H0_DELAY	4
	#define L0_DELAY	11
	#define H1_DELAY	8
	#define L1_DELAY	7
#elif F_CPU == 24000000UL
		/* 24MHz */
	#define H0_DELAY	5
	#define L0_DELAY	15
	#define H1_DELAY	11
	#define L1_DELAY	9
#endif
//-----------------------

/*
 * definicje nazw symbolicznych wykorzystywanych rejestrów
 */
//r30, r31 (Z) - indeks bufora ramki
#define ridxhi	r25 //licznik elementów do przetworzenia - starszy bajt
#define ridxlo	r24	//...młodszy bajt
#define routpin	r23		//pin wyjściowy (maska bitowa)
#define routport	PIN(WS2812_PIN_LETTER)		//port wyjściowy
#define rbyte	r22		//przetwarzany bajt
#define ridxbit	r21		//indeks bitu
#define rtmp	r20		//zachowanie rejestru SREG
//------------------------------------

;makra pomocnicze wstawiające instrukcje opóźniające
.macro delay_jumps jmps
.rept \jmps
rjmp .+0
.endr
.endm

.macro delay_nops nops
.rept \nops
nop
.endr
.endm

.macro delay_instr cnt=0
.if \cnt / 2
delay_jumps (\cnt/2)
.endif
.if \cnt % 2
delay_nops (\cnt%2)
.endif
.endm
;------------------

	.section .text

	.global ws2812_display

//wersja na µC obługujących PINx = mask jako PORTx ^= mask
#if !(\
		defined (__AVR_ATmega8__) || defined (__AVR_ATmega8A__) ||\
		defined (__AVR_ATmega16__) || defined (__AVR_ATmega16A__) ||\
		defined (__AVR_ATmega32__) || defined (__AVR_ATmega32A__) ||\
		defined (__AVR_ATmega64__) || defined (__AVR_ATmega64A__) ||\
		defined (__AVR_ATmega128__) || defined (__AVR_ATmega128A__)\
	)

	/*defined (__AVR_ATmega328P__) || defined (__AVR_ATmega328__) ||\
	defined (__AVR_ATmega168__)  || defined (__AVR_ATmega168A__) ||\
	defined (__AVR_ATmega168P__) || defined (__AVR_ATmega168PA__) ||\
	defined (__AVR_ATmega88__)   || defined (__AVR_ATmega88A__) ||\
	defined (__AVR_ATmega88P__)  ||\
	defined (__AVR_ATmega48A__)  || defined (__AVR_ATmega48__) ||\
	defined (__AVR_ATmega48PA__) || defined (__AVR_ATmega48P__)*/

//********* OUT **************
ws2812_display:
; inicjalizacja stałych i zmiennych
	ldi r30, lo8( ws2812_fbuf )	;adres bufora ramki do rejestru Z
	ldi r31, hi8( ws2812_fbuf )	;
	ldi ridxlo, lo8( WS2812_LED_NUM * 3 )	;długość bufora ramki do licznika
	ldi ridxhi, hi8( WS2812_LED_NUM * 3 )
	ldi routpin, _BV(WS2812_PIN_NR)
	ldi ridxbit, 6	;tylko bajty 0-5 w wewnętrznej pętli

; sprawdzenie czy są dane do przetwarzania
	cp ridxlo,r1		;jeżeli WS2812_LED_NUM == 0 ?
	cpc ridxhi,r1		;...
	brne cont
	rjmp exitproc		;...wyjdz z procedury
cont:

	ld rbyte, Z+	;pobierz bajt z bufora ramki

; przerwania
	in rtmp,SREG	;zachowanie globalnej flagi przerwań
	cli				;wyłącz przerwania, procedura jest krytyczna czasowo

;---- początek nadawania -----
;bity 0-5 są takie same - w czasie ich nadawania nie są wykonywane żadne inne obliczenia
;bit 6 - sprawdzanie osiągnięcia końca bufora i ewentualne skierowanie programu w stronę końca
;bit 7 wersja zwykła - wczytanie nowego bajtu z bufora
;bit 7 wersja końcowa - wyjście z pętli
loop:
;bity 0-5
	out routport, routpin	;1	[1]	ustaw stan wysoki na wyjściu
	lsl rbyte				;1	[2]
	brcs bit_0_5_one		;1/2[3] jeżeli bit==1 skocz
	DELAY_INSTR H0_DELAY
	out routport, routpin	;1	[4]
	nop						;1	[5]
	nop						;1	[6]
	DELAY_INSTR L0_DELAY
	dec ridxbit				;1	[7] licznik wewnętrznej pętli
	;kolejne dwie instrukcje muszą być na końcu, ewentualne nopy wyżej
	breq bit_6_zero			;1/2[8]
	rjmp loop				;2	[10]

bit_0_5_one:				;po skoku	[4]
	nop						;1	[5]
	DELAY_INSTR H1_DELAY
	dec ridxbit				;1	[6] licznik wewnętrznej pętli
	out routport, routpin	;1	[7]
	DELAY_INSTR L1_DELAY
	;kolejne dwie instrukcje muszą być na końcu, ewentualne nopy wyżej
	breq bit_6_zero			;1/2[8]
	rjmp loop				;2	[10]

;....................
;bit 6
bit_6_zero:					;po skoku [9]
	nop						;1	[10] - końcówka 5. bitu
	out routport, routpin	;1	[1]
	lsl rbyte				;1	[2]
	brcs bit_6_one			;1/2[3]
	DELAY_INSTR H0_DELAY
	out routport, routpin	;1	[4]
	nop						;1	[5]
	DELAY_INSTR L0_DELAY
;następne instrukcje muszą być na końcu, ewentualne nopy wyżej
	sbiw ridxlo,1			;2	[7] zmniejsz licznik
	breq bit_7_zero_end		;1/2[8]	jeżeli koniec bufora ramki wyjdź
	rjmp bit_7_zero			;2	[10]

bit_6_one:					;po skoku [4]
	DELAY_INSTR H1_DELAY
	sbiw ridxlo, 1			;2	[6]
	out routport, routpin	;1	[7]
	DELAY_INSTR L1_DELAY
;następne dwie instrukcje muszą być na końcu, ewentualne nopy wyżej
	breq bit_7_zero_end		;1/2[8]
	rjmp bit_7_zero			;2	[10]

;.................
;ostatni bit całej transmisji jest inny
;koniec bufora ramki, wyślij ostatni bit i zakończ pracę
bit_7_zero_end:				;po skoku [9]
	nop						;1	[10] - końcówka przedostatniego bitu
	out routport, routpin	;1	[1]
	lsl rbyte				;1	[2]
	brcs bit_7_one_end		;1/2[3]
	DELAY_INSTR H0_DELAY
	out routport, routpin	;1	[4]
	rjmp endproc			;2	[6]

bit_7_one_end:				;po skoku [4]
	nop						;1	[5]
	nop						;1	[6]
	DELAY_INSTR H1_DELAY
	out routport, routpin	;1	[7]
	rjmp endproc			;2	[9]
;ostatni wysłany bit jest krótszy

;.....................
;bit 7
bit_7_zero: 					;ostatni bit 0
	out routport, routpin	;1	[1]
	lsl rbyte				;1	[2]
	brcs bit_7_one			;1/2[3]
	DELAY_INSTR H0_DELAY
	out routport, routpin	;1	[4]
	DELAY_INSTR L0_DELAY
	ld rbyte, Z+			;2	[6]	wczytaj następny bajt z bufora ramki
	ldi ridxbit, 6			;1	[7]	licznik wewnętrznej pętli
	nop						;1	[8]
	rjmp loop				;2 [10]

bit_7_one:					;po skoku [4]
	DELAY_INSTR H1_DELAY
	ld rbyte, Z+			;2	[6] następny bajt z bufora ramki
	out routport, routpin	;1	[7]
	DELAY_INSTR L1_DELAY
	ldi ridxbit, 6			;1	[8] licznik wewnętrznej pętli
	rjmp loop				;2	[10]

;----- koniec nadawania -----

endproc:
	out SREG,rtmp	;odtwórz globalną flagę przerwania
exitproc:
	ret

//********* OUT **************

#else	//**** wersja dla starych µC

//******** SBI/CBI *****************************************
//  ---  redefinicja stalych  ---
;w starych µC uzywamy PORTx
#ifdef routport
	#undef routport
#endif
#define routport	PORT(WS2812_PIN_LETTER)		//port wyjściowy

#ifdef routpin
	#undef routpin
#endif

/*
 * Dla tej wersji minimalnym taktowaniem jest 10MHz. Przy 8MHz prawdopodobnie bedzie
 * dzialać, chociaż na granicy tolerancji. Wyliczenie ilości instrukcji opóźniajacych
 * przez modyfikacje wczesniejszych definicji dla 8MHz
 */
 #if F_CPU < 10000000UL
 	#warning Procedura przygowowana dla czestotliwości min 10MHz
	#define H0_DELAY_M	0
	#define L0_DELAY_M	0
	#define H1_DELAY_M	0
	#define L1_DELAY_M	0
#else
	#define H0_DELAY_M	H0_DELAY-1
	#define L0_DELAY_M	L0_DELAY-1
	#define H1_DELAY_M	H1_DELAY-1
	#define L1_DELAY_M	L1_DELAY-1
#endif


// ---- poczatek procedury -----
ws2812_display:
; inicjalizacja stałych i zmiennych
	ldi r30, lo8( ws2812_fbuf )	;adres bufora ramki do rejestru Z
	ldi r31, hi8( ws2812_fbuf )	;
	ldi ridxlo, lo8( WS2812_LED_NUM * 3 )	;długość bufora ramki do licznika
	ldi ridxhi, hi8( WS2812_LED_NUM * 3 )
	ldi ridxbit, 6	;tylko bajty 0-5 w wewnętrznej pętli

; sprawdzenie czy są dane do przetwarzania
	cp ridxlo,r1		;jeżeli WS2812_LED_NUM == 0 ?
	cpc ridxhi,r1		;...
	brne cont
	rjmp exitproc		;...wyjdz z procedury
cont:

	ld rbyte, Z+	;pobierz bajt z bufora ramki

; przerwania
	in rtmp,SREG	;zachowanie globalnej flagi przerwań
	cli				;wyłącz przerwania, procedura jest krytyczna czasowo

;---- początek nadawania -----
;bity 0-5 są takie same - w czasie ich nadawania nie są wykonywane żadne inne obliczenia
;bit 6 - sprawdzanie osiągnięcia końca bufora i ewentualne skierowanie programu w stronę końca
;bit 7 wersja zwykła - wczytanie nowego bajtu z bufora
;bit 7 wersja końcowa - wyjście z pętli
loop:
;bity 0-5
	sbi routport, WS2812_PIN_NR	;2	[2]	ustaw stan wysoki na wyjściu
	lsl rbyte				;1	[3]
	brcs bit_0_5_one		;1/2[4] jeżeli bit==1 skocz
	DELAY_INSTR H0_DELAY_M
	cbi routport, WS2812_PIN_NR	;2	[6]	ustaw stan niski na wyjściu
	nop						;1	[7]
	nop						;1	[8]
	DELAY_INSTR L0_DELAY_M
	dec ridxbit				;1	[9] licznik wewnętrznej pętli
	;kolejne dwie instrukcje muszą być na końcu, ewentualne nopy wyżej
	breq bit_6_zero			;1/2[10]
	rjmp loop				;2	[12]

bit_0_5_one:				;po skoku	[5]
	nop						;1	[6]
	DELAY_INSTR H1_DELAY_M
	dec ridxbit				;1	[7] licznik wewnętrznej pętli
	cbi routport, WS2812_PIN_NR	;2	[9]	ustaw stan niski na wyjściu
	DELAY_INSTR L1_DELAY_M
	;kolejne dwie instrukcje muszą być na końcu, ewentualne nopy wyżej
	breq bit_6_zero			;1/2[10]
	rjmp loop				;2	[12]

;....................
;bit 6
bit_6_zero:					;po skoku [11]
	nop						;1	[12] - końcówka 5. bitu
	sbi routport, WS2812_PIN_NR	;2	[2]	ustaw stan wysoki na wyjściu
	lsl rbyte				;1	[3]
	brcs bit_6_one			;1/2[4]
	DELAY_INSTR H0_DELAY_M
	cbi routport, WS2812_PIN_NR	;2	[6]	ustaw stan niski na wyjściu
	nop						;1	[7]
	DELAY_INSTR L0_DELAY_M
;następne instrukcje muszą być na końcu, ewentualne nopy wyżej
	sbiw ridxlo,1			;2	[9] zmniejsz licznik
	breq bit_7_zero_end		;1/2[10]	jeżeli koniec bufora ramki wyjdź
	rjmp bit_7_zero			;2	[12]

bit_6_one:					;po skoku [5]
	DELAY_INSTR H1_DELAY_M
	sbiw ridxlo, 1			;2	[7]
	cbi routport, WS2812_PIN_NR	;2	[9]	ustaw stan niski na wyjściu
	DELAY_INSTR L1_DELAY_M
;następne dwie instrukcje muszą być na końcu, ewentualne nopy wyżej
	breq bit_7_zero_end		;1/2[10]
	rjmp bit_7_zero			;2	[12]

;.................
;ostatni bit całej transmisji jest inny
;koniec bufora ramki, wyślij ostatni bit i zakończ pracę
bit_7_zero_end:				;po skoku [11]
	nop						;1	[12] - końcówka przedostatniego bitu
	sbi routport, WS2812_PIN_NR	;2	[2]	ustaw stan wysoki na wyjściu
	lsl rbyte				;1	[3]
	brcs bit_7_one_end		;1/2[4]
	DELAY_INSTR H0_DELAY_M
	cbi routport, WS2812_PIN_NR	;2	[6]	ustaw stan niski na wyjściu
	rjmp endproc			;2	[8]

bit_7_one_end:				;po skoku [5]
	nop						;1	[6]
	nop						;1	[7]
	DELAY_INSTR H1_DELAY_M
	cbi routport, WS2812_PIN_NR	;2	[9]	ustaw stan niski na wyjściu
	rjmp endproc			;2	[11]
;ostatni wysłany bit jest krótszy

;.....................
;bit 7
bit_7_zero: 					;ostatni bit 0
	sbi routport, WS2812_PIN_NR	;2	[2]	ustaw stan wysoki na wyjściu
	lsl rbyte				;1	[3]
	brcs bit_7_one			;1/2[4]
	DELAY_INSTR H0_DELAY_M
	cbi routport, WS2812_PIN_NR	;2	[6]	ustaw stan niski na wyjściu
	DELAY_INSTR L0_DELAY_M
	ld rbyte, Z+			;2	[8]	wczytaj następny bajt z bufora ramki
	ldi ridxbit, 6			;1	[9]	licznik wewnętrznej pętli
	nop						;1	[10]
	rjmp loop				;2 [12]

bit_7_one:					;po skoku [5]
	DELAY_INSTR H1_DELAY_M
	ld rbyte, Z+			;2	[7] następny bajt z bufora ramki
	cbi routport, WS2812_PIN_NR	;2	[9]	ustaw stan niski na wyjściu
	DELAY_INSTR L1_DELAY_M
	ldi ridxbit, 6			;1	[10] licznik wewnętrznej pętli
	rjmp loop				;2	[12]

;----- koniec nadawania -----

endproc:
	out SREG,rtmp	;odtwórz globalną flagę przerwania
exitproc:
	ret

//******** SBI/CBI *****************************************

#endif

