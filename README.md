# bitstorm_router_hwtest

Simple test application that exercises the varoius hardware capabilities of the BitStorm Router B.

Currently, this includes usart0 (WAN), usart1 (BLE), and MCU LEDs 1 and 2.

Requires discovery-ble121lr script to be installed on the BLE chip.

MCU lowers BLE CTS then the BLE will begin sending AD messages on usart1.

The MCU will then pass the message out through the WAN usart0.

A very simply 1K databuffer is used for collecting bytes and the buffer is cleared and sent out the WAN port a 1Hz.

If a character is received by the MCU across the WAN port, the letters "CPH" plus the received character are blasted into the buffer.  This will overwrite what ever is there.  At the next 1Hz output, the buffer will be displayed and cleared.