* computer start
* automatically runs bios/uefi 
	* runs something in some memory (flash i think)
	* that thing will be Danny's bootloader
* bootloader does some stuff
	* sets up stack pointer
	* sets up exception levels
	* idk what else
	* bl os_entry
	* ^ that will run my os
* now my os is in play
	* set my own stack
	* and run some c code
	* [notes on os](OS)



linkerscript takes all the files and does its magic,
* puts my bootloader in storage?
* puts my uart driver somewhere
* 