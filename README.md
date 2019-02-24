# Insper Filament Vending Machine 

This is the repository of the Insper filament vending machine.

![](https://github.com/Insper/VendingMachine/blob/master/doc/endv0-p1.jpeg)
![](https://github.com/Insper/VendingMachine/blob/master/doc/endv0-p2.jpeg)

The first go of this machine was to demonstrate to undergraduate students that are attending the embedded computer course (Insper Computer Engineering) an demo of what they should do on the first (of two) projects of this course. On this project they should specify and develop an autonomous machine that sells something.

Looking for some real need of a vending machine on Insper, we decided together if Insper-Fablab to "sell"  filaments for the 3D printers. 

Insper Fablab was response for all the mechanical part (project and fabrication). Insper Computer Architecture Lab for the specification, assemble and firmware development.

Now that we have a first prototype we detect several improvements to the machine on several areas: Usability, Mechanical and Firmware. We start to specify a second version that shall be put under real test.

# Technical

This machine is controlled by a Microchip Cortex M7 uC (SAME70) and runs a freertos to control all peripherals: 

- LCD
- Keypad
- bluetooth communication
- two step motors
- RGB LED strips

---------------------------
- UNDER CONSTRUCTION 
---------------------------

```


#############################################################
#############################################################
##               --  EXT1 LCD maXTouch Xplained --         ##
##                                                         ##
##                        \/                               ##
##                        ID    GND                        ##
##               (**NC**) PC31  PA19 (**NC**)              ##
##                 (GPIO) PB3   PB2  (**NC**)              ##
##                  (PWM) PA0   PC30 (**NC**)              ##
##             (GPIO/IRQ) PD28  PC17 (GPIO)                ##
##            (I²C - SDA) PA3   PA4  (I²C - SCL)           ##
##               (**NC**) PB0   PB1  (**NC**)              ##
##             (SPI MISO) PD25  PD21 (SPI SCK)             ##
##        (Botão Direita) PD20  PD22 (Botão Baixo)         ##
##                        GND   VCC                        ##
##                                                         ##
#############################################################
#############################################################


#############################################################
#############################################################
##              --  J500 - Motor Driver 2  --              ##
##                                                         ##
##               \/                                        ##
##                                                         ##
##               AREF (AREF) - (**NC**)                    ##
##               GND   (GND) - (GND)                       ##
##               D13  (PD22) - (EN_B - Motor 2)            ##
##               D12  (PD20) - (EN_A - Motor 2)            ##
##               D11  (PD21) - (IN2_B - Motor 2)           ##
##               D10  (PD25) - (IN1_B - Motor 2)           ##
##               D9    (PC9) - (IN2_A - Motor 2)           ##
##               D8   (PA17) - (IN1_A - Motor 2)           ##
##                                                         ##
##                                                         ##
#############################################################
#############################################################


#############################################################
#############################################################
##              --  J503 - Motor Driver 1  --              ##
##                                                         ##
##               \/                                        ##
##                                                         ##
##               D7  (PA2) - (EN_B - Motor 1)              ##
##               D6 (PD19) - (EN_A - Motor 1)              ##
##               D5 (PD11) - (IN2_B - Motor 1)             ##
##               D4 (PD27) - (IN1_B - Motor 1)             ##
##               D3  (PA6) - (IN2_A - Motor 1)             ##
##               D2  (PA5) - (IN1_A - Motor 1)             ##
##               D1 (PD30) - (**NC**)                      ##
##               D0 (PD28) - (**NC**)                      ##
##                                                         ##
##                                                         ##
#############################################################
#############################################################


```
