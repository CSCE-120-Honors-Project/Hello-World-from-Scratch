universal asynchronous receiver/transmitter

its a peripheral device that the cpu uses to send and receive data
QEMU initializes it automatically
has a base address, and then offsets for registers
since its already initialized, we only need to deal with these things:
* base: 0x09000000
* data register (what is to be transmitted or what has been received) is offset by 0x000 (its at the start exactly)
* flag register for checking before writing:
	* bit 5: transmit fifo full
		* 1 means its full and we cant send
		* 0 means space available and we can write to it
	* bit 6: receive fifo empty
		* 1 means theres no data to read
		* 0 means theres data to read
	* offset: 0x018

cool trick that this uart module (pl011) does:
* instead of putting both transmit data and receive data in the same data register, it recognizes that you want to either read or write and accesses different "hardware" based on what you want
* so inside the uart chip:
	* a cpu write request will be rerouted to transmit fifo
	* whereas a cpu read request would be rerouted to receive fifo