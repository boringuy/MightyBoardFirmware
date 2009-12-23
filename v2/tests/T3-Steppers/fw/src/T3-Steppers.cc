/*
 * T3-Steppers.cc
 *
 *  Created on: Dec 10, 2009
 *      Author: phooky
 */

#include "UART.hh"
#include "PSU.hh"
#include "DebugPacketProcessor.hh"
#include "CommandPacketProcessor.hh"
#include <avr/interrupt.h>
#include "Timeout.hh"
#include "DebugPin.hh"
#include "Steppers.hh"

#if defined(__AVR_ATmega644P__)
#define HAS_PASSTHRU 1
#elif defined(__AVR_ATmega168__)
#define HAS_PASSTHRU 0
#endif

int main() {
	initPsu();
	TimeoutManager::init();
	uart[0].enable(true);
#if HAS_PASSTHRU
	uart[1].enable(true);
#endif // HAS_PASSTHRU
	sei();
	steppers.setTarget(Point(500,0,0),500);
	while (1) {
		if (uart[0].in_.hasError() && uart[0].in_.getErrorCode() == PacketError::PACKET_TIMEOUT) {
			// nop for now
			uart[0].in_.reset();
		}
		if (!steppers.isRunning()) {
			setDebugLED(true);
			steppers.setTarget(Point(0,500,0),500);
		}
		if (uart[0].in_.isFinished()) {
			if (processDebugPacket(uart[0].in_, uart[0].out_)) {
				uart[0].in_.reset();
				uart[0].beginSend();
			} else if (processCommandPacket(uart[0].in_, uart[0].out_)) {
				uart[0].in_.reset();
				uart[0].beginSend();				
			} else {
				uart[0].in_.reset();
			}
			while (!uart[0].out_.isFinished()) {
			}
			uart[0].out_.reset();
		}
	}
	return 0;
}
