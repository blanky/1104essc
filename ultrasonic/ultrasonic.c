/**
 * \file
 *         Ultrasonic test process
 * \author
 *         s4292844
 */

#include "contiki.h"
#include <stdio.h>
#include "sys/etimer.h"

#include "sensortag/board-peripherals.h"
#include "sensortag/cc2650/board.h"
#include "lib/cc26xxware/driverlib/gpio.h"
#include "ti-lib.h"
#include "dev/leds.h"

//-----------------------USEFUL STUFF ------------------//

#define SENSORTAG_GROVE2_DP2	1 << BOARD_IOID_DP2
#define SENSORTAG_GROVE2_DP3	1 << BOARD_IOID_DP3

#define ULTRA_PIN SENSORTAG_GROVE2_DP3
//timeout = ~24ms because
//speed of sound 340m/s
//max range = 4m
#define PIN_TIMEOUT 24000

int32_t ultrasonic_pulse(void) {
  int32_t tick_count = 0;
  GPIODirModeSet(ULTRA_PIN, GPIO_DIR_MODE_OUT);
  GPIOPinWrite(ULTRA_PIN, 0);
  clock_delay_usec(2);
  GPIOPinWrite(ULTRA_PIN, ULTRA_PIN);
  clock_delay_usec(5);
  GPIOPinWrite(ULTRA_PIN, 0);
  GPIODirModeSet(ULTRA_PIN, GPIO_DIR_MODE_IN);
  while(!GPIOPinRead(ULTRA_PIN)) {
    tick_count++;
    if(tick_count > PIN_TIMEOUT) {
      return -1;
    }
    clock_delay_usec(1);
  }
  tick_count = 0;
  while(GPIOPinRead(ULTRA_PIN)) {
    tick_count++;
    if(tick_count > PIN_TIMEOUT) {
      tick_count = -1;
      break;
    }
    clock_delay_usec(1);
  }

  return tick_count;
}

//---------------------END USEFUL STUFF---------------------

/*---------------------------------------------------------------------------*/
PROCESS(gpio_process, "GPIO process");
AUTOSTART_PROCESSES(&gpio_process);
/*---------------------------------------------------------------------------*/
//Main Thread
PROCESS_THREAD(gpio_process, ev, data) {
  static struct etimer et;
  int latest_tick = 0, distance;
  PROCESS_BEGIN();
  etimer_set(&et, CLOCK_SECOND);

	
	while(1) {
    PROCESS_WAIT_UNTIL(etimer_expired(&et));
    latest_tick = (int) ultrasonic_pulse();
    //formula for conversion
    distance = (latest_tick + 31) / 10;
    if(distance > 84) {
      leds_off(LEDS_ALL);
      leds_on(LEDS_GREEN);
    } else if(distance < 10) {
      leds_off(LEDS_ALL);
      leds_on(LEDS_RED);
    } else {
      leds_off(LEDS_ALL);
    }
    printf("Tick: %d, Distance: %d+-3 cm\n\r", latest_tick, distance);
    etimer_reset(&et);
	}

  PROCESS_END();
}
