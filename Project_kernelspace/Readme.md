Project title:

 GPIO device driver for speed calculation using PIR sensors. 

Summary:

This is a basic implementation of a character device driver which maps the physical address of a GPIO and manipulates the respective bits. Two PIR sensors are positioned at fixed distance (which can be specified when loading the module). When someone (or something) walks in front of any of the two sensors, it is detected as a HIGH output by the PIR sensor connected to the GPIO pins. The time associated with the HIGH output is obtained and speed is then computed dividing the sensors distance by the difference between these two time values.


Working:
When a PIR sensor detects motion, it generates a HIGH signal at the output. The output of the PIR sensor is connected to the GPIO pin. Hence, whenever a motion is detected, a HIGH signal is recieved as an input by the GPIO. This HIGH value is stored in the GPLEV read only register at the corresponding offset which can be read from the kernel space. 

In the project, 2 PIR sensor can be placed at the required distance (in cm) and the value can be given as input by the user. The object should move in the path of the PIR sensors. When the first PIR sensor detects the motion, it sends a HIGH signal at the GPIO. This is noted and the clock time in epoch format (in seconds)is recorded.  The object now moves through the second PIR sensor and the same process is repeated. The speed of the object is calculated using the distance between the PIR sensors and the difference of the time recorded  (in cm/sec). 

Hardware Design:

Components used:
1.	2 PIR sensors
2.	Breadboard 
3.	Connecting wires
4.	2 LEDs
5.	2 220 ohm resistors 

 

The GPIO register information is taken from Broadcom's BCM2835 ARM Peripherals document. There are two read-only registers which can be used to check the level of the GPIOs - GPLEV0 and GPLEV1, the first one starting at offset 0x34 from the GPIO base address.

![schematic](https://user-images.githubusercontent.com/47379504/56853195-fa060500-6941-11e9-888d-5c7f08838211.png)

![WhatsApp Image 2019-04-27 at 11 00 58 PM](https://user-images.githubusercontent.com/47379504/56853207-2c176700-6942-11e9-81d1-839fa59269e9.jpeg)

Firmware and its build process:
1.	The project has been implemented on Raspberry Pi 3 model B V1.2 Raspbian. Communication between the host development system which is the PC and Raspberry Pi is done via Ethernet.  
2.	Download Raspbian OS to initiate a desktop environment for Raspberry Pi on PC.
3.	Insert an SD card and use "win32 disk imager" to write the downloaded OS into SD card (Ensure that the SD card has just one partition).
4.	Select the correct device that is the drive representing the SD card and run the image writer using the write command.
5.	Once the write is complete, eject the SD card and insert it into the Raspberry Pi. Insert the power and ethernet cable.
6.	After booting up, Raspberry Pi comes with a default user name and password and so always use it whenever it is being asked. The credentials are:
Login: pi
Password: raspberry
7.	Once you are on the desktop, open a terminal and enter the following command to update the firmware of the Pi 
 sudo rpi-update
8.	Check the LINUX version of the OS using command
 uname -r
9.	To ensure proper build and loading of module: Download the Respective Header file from the link 
 https://www.niksula.hut.fi/~mhiienka/Rpi/linux-headers-rpi/
10.	Once headers are downloaded install it using command 
               sudo dpkg -i linux-headers-“Depending on ur version”armhf.deb
11.	Now the code can be built and the module can be loaded.

Kernel Space Driver and its build process:
1.	First edit the Makefile specifying the kernel code path. Then do:
make all
2.	If you want to revert the compilation, do:
  make clean
3.	To load the module:
    sudo insmod pir.ko
4.	To remove the module:
    sudo rmmod pir.ko

User space application:
1.	Compile the user space file pir-test.c using command
     gcc -o pir-test pir-test.c
2.	Run it using  command
     sudo ./pir-test
3.	Input the distance between the sensors and move the object from one PIR sensor to another.
4.	The speed will be calculated and displayed.
