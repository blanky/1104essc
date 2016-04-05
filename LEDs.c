/**
 * \file
 *         A very simple Contiki application to write and read the sensortag's gpio pins (Grove2 connector)
 * \author
 *         mds
 */

#include "contiki.h"
#include <stdio.h>
#include "dev/leds.h"
#include "sensortag/board-peripherals.h"
#include "sensortag/cc2650/board.h"
#include "lib/cc26xxware/driverlib/gpio.h"
#include "ti-lib.h"
#include "random.h"

//create masks for Grove2 connector (debug devpack)
#define SENSORTAG_GROVE2_DP2	1 << BOARD_IOID_DP2
#define SENSORTAG_GROVE2_DP3	1 << BOARD_IOID_DP3

#define LED_CLK SENSORTAG_GROVE2_DP3
#define LED_DATA SENSORTAG_GROVE2_DP2

#define NUM_LEDS 2

#define RANDOM_RAND_MAX = 128;

 uint8_t led_state[NUM_LEDS*3];

 

 void led_clk(){
 	GPIOPinWrite(LED_CLK, 0);
 	clock_delay_usec(20);
 	GPIOPinWrite(LED_CLK, 1);
 	clock_delay_usec(20);
 }

 void led_send_byte(uint8_t b){
 	for (int i=0; i<8; i++){
 		if ((b & 0x80) != 0){
 			GPIOPinWrite(LED_DATA, 1);
 		} else {
 			GPIOPinWrite(LED_DATA, 0);
 		}
 		led_clk();
 		b <<= 1;
 	}
 }

 void led_send_colour(uint8_t red, uint8_t green, uint8_t blue){
 	uint8_t prefix = 0b11000000;
 	if ((blue & 0x80) == 0)   prefix |= 0b00100000;
 	if ((blue & 0x40) == 0)   prefix |= 0b00010000;
 	if ((green & 0x80) == 0)  prefix |= 0b00001000;
 	if ((green & 0x40) == 0)  prefix |= 0b00000100;
 	if ((red & 0x80) == 0)    prefix |= 0b00000010;
 	if ((red & 0x80) == 0)    prefix |= 0b00000001;
 	led_send_byte(prefix);

 	led_send_byte(blue);
 	led_send_byte(green);
 	led_send_byte(red);
 }

 void led_set_colour_rgb(uint8_t led, uint8_t red, uint8_t green, uint8_t blue){

 	for(int i = 0; i < 4; i++){
 		led_send_byte(0x00);
 	}

 	for(int i = 0; i < NUM_LEDS; i++){
 		if (i == led){
 			led_state[i*3 + 0] = red;
 			led_state[i*3 + 1] = green;
 			led_state[i*3 + 2] = blue;
 		}

 		led_send_colour(led_state[i*3 + 0], //Red
 						led_state[i*3 + 1], //Green
 						led_state[i*3 + 2]);//Blue
 	}

 	for(int i = 0; i < 4; i++){
 		led_send_byte(0x00);
 	}
 }

void led_init(){
 	// set pin DP2 as output, DP3 as input
    GPIODirModeSet(SENSORTAG_GROVE2_DP2, GPIO_DIR_MODE_OUT);
    GPIODirModeSet(SENSORTAG_GROVE2_DP3, GPIO_DIR_MODE_OUT);

    //led_state = (uint8_t*) calloc(NUM_LEDS*3, sizeof(uint8_t));

    for(uint8_t i=0; i<NUM_LEDS; i++){
    	led_set_colour_rgb(i, 0, 0, 0);
    }
 }

/*---------------------------------------------------------------------------*/
PROCESS(gpio_interface_process, "GPIO process");
AUTOSTART_PROCESSES(&gpio_interface_process);
/*---------------------------------------------------------------------------*/
//GPIO Interface Thread
PROCESS_THREAD(gpio_interface_process, ev, data) {


    PROCESS_BEGIN();
    
    
	int red = 0;
	int green = 0;
	int blue = 0;
	int rssi = 0;
	led_init();

	random_init(5674);
	
	while(1) {

		

		

		// uint8_t rand_red = (0x00FF & random_rand());
		// uint8_t rand_green = (0x00FF & random_rand());
		// uint8_t rand_blue = (0x00FF & random_rand());
		// red = (0x00FF & random_rand());
		// green = (0x00FF & random_rand());
		// blue = (0x00FF & random_rand());

		// led_set_colour_rgb(0, rand_red, rand_green, rand_blue);
		// led_set_colour_rgb(1, red, green, blue);

		// clock_wait(CLOCK_SECOND/20);

		// while((rand_red + rand_blue + rand_green + red + blue + green) > 0){
		// 	if(rand_red > 0) {
		// 		rand_red -= 10;
		// 	} else rand_red = 0;
		// 	if(rand_green > 0) {
		// 		rand_green -= 10;
		// 	} else rand_green = 0;
		// 	if(rand_blue > 0) {
		// 		rand_blue -= 10;
		// 	} else rand_blue = 0;

		// 	if(red > 0) {
		// 		red -= 10;
		// 	} else red = 0;
		// 	if(green > 0) {
		// 		green -= 10;
		// 	} else green = 0;
		// 	if(blue > 0) {
		// 		blue -= 10;
		// 	} else blue = 0;


		// 	led_set_colour_rgb(0, rand_red, rand_green, rand_blue);
		// 	led_set_colour_rgb(1, red, green, blue);

		// 	clock_wait(CLOCK_SECOND/20);
		// }


		for (rssi= 0; rssi > -100; rssi -= 1){

			red = rssi*-2;
			green = rssi*2+200;

			led_set_colour_rgb(0, red, green, 0);
			led_set_colour_rgb(1, red, green, 0);
			clock_wait(CLOCK_SECOND/50);
		
		}
	}

    PROCESS_END();
}

