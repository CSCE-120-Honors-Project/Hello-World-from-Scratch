#include "../uart/uart.h"

void main(void) {
    uart_init(); // literally does nothing because qemu pre-initializes it, but have this line for good practice
    uart_puts("Hello World!\nHowdy World!, this is the OS!");
    
    while(1); //infinite loop so we don't leave the OS
}