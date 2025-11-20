* privilege model
	* hierarchy of security/privilege
	* higher number = higher power

EL0: unprivileged- weakest one for user applications
cant use system registers, hardware directly, MMU config
has to use system calls to request stuff

EL1: privileged (OS kernel)
can use most registers, mmu config, interrupts
for controlling memory management, processes, device drivers, etc.
THIS IS WHAT I PLAY WITH

EL2: Hypervisor (Virtualization)
for virtual machine creation / vm context switching.
controls EL1 and EL0

EL3: Secure monitor
full control over everything
for security operation's and secure monitor firmware
controls: security state transition, trusted boot, power management

STEPS TO TAKE
* dont need to do any of this bc this program so simple
* but i owul dcreate a table iwht 16 entries at some 2kb aligned address bc aarch64 requires that
* then write handlers for each type i want to handle
* tell the cpu about it by writing to VBAR_EL1, or same with 2 , 3