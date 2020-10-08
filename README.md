# vis-nir_spectrometer_detection_unit
This project contains a high-resolution detection unit for VIS/NIR spectroscopy using the Hamamatsu S1337-1010 photodiode and the Analog AD7768-1 24 Bit ADC.

It was developed as a student project in the summer term of 2020 int the course "Optical Spectroscopy and its Applications" at Cologne University of Applied Sciences.

## Getting Started

![](https://github.com/bazjo/vis-nir_spectrometer_detection_unit/raw/master/resources/images/pcb.jpg)

The PCB is designed to utilize an Arduino shield footprint for use with various breakout boards, such as ST Nucleo STM32 development boards or the avnet MiniZed development platform for Xilinx Zynq.

It features power filtering and an -5V inverter, the AD7768-1 DC to 204 kHz 24-Bit ADC (connected via SPI) with a 4.096 V ADR4540 and a photodiode front end featuring the AD8622 OpAmp. When a tunable clock is not available, i.e. when no FPGA is used, the main clock can be provided by an Si514 Any Frequency Programmable Oscillator (connected via I2C).

The photodiode is attached at the front to be mounted inside the spectrometer.

When assembling the PCB please look out for the bodges/errata. The PCB has been designed using kicad.

Demo applications are provided for the Arduino platform (just for the Si514 as the AD7768-1 requires 16-Bit SPI which is not yet supported by Arduino) and the STM32 platform. This application is designed for the Nucleo F401RE and was developed using the Crossley CrossWorks for ARM IDE. The code can be compiled with the trial version. This demo Application continuously sends data to a connected computer at 300 Sp/s via a serial connection.

## Theory of Operation

![](https://github.com/bazjo/vis-nir_spectrometer_detection_unit/raw/master/resources/images/schematics.png)

### Pinout
[jumpered] Pins must be connected by a designated jumper resistor.

Arduino Pin | STM32F401RE Pin | Connection
---|---|---
D0|PA3|none (!DRDY if not bodged)
D1|PA2|[jumpered] AD7768-1 !RESET
D2|PA10|[jumpered] AD7768-1 GPIO0
D3|PB3|[jumpered] AD7768-1 GPIO1
D4|PB5|[jumpered] AD7768-1 GPIO2
D5|PB4|[jumpered] AD7768-1 GPIO3
D6|PB10|[jumpered] AD7768-1 MCLK
D7|PA8|AD7768-1 !DRDY after bodge
D10|PB6|AD7768-1 !CS
D11|PA7|AD7768-1 MOSI
D12|PA6|AD7768-1 MISO
D13|PA5|AD7768-1 SCK
SDA|PB9|Si514 SDA
SCL|PB8|Si514 SCL

### Bodges/Errata
There are two problems with the pcb which are minor enough not to roll out a new revision
1. The Arduino RX/TX pins are used for GPIO. Unfortunately those Pins are used for the VCP connection on the Nucleo boards as well. The needed connection !DRDY is therefore rerouted to pin D7. This bodge can be performed by drilling the via on !DRDY with 0.8 mm drill bit and soldering a bodge wire between TP6 and D7.
2. When using the ADC on it's own, sync pulses can be generated via SPI. Those need to be send from SYNC_OUT to SYNC_IN. For this, bridge TP4 and TP5 with a 1206 0R resistor.

### PSU
The circuit uses three voltage rails. The 3V3 and 5V rails are used straight from the baseboard and are filtered through a 2-Stage LC PI-filter. The -5V rail is generated locally with TPS63700 inverter and subsequent LM2991 LDO. Power Planes are located on the second inner layer of the PCB. there are different planes for analog and digital ground, whose domains are joint below the high-speed adc interface to prevent loop currents.

### Oscillator
The Si514 Oscillator is attached via I2C and is equipped with pullup resistors. The source for the ADC MCLK can be selected between Si514 and baseboard via means of the resistor jumpers R24/R29. The tap R29 could also be used for MCLK monitoring.

### ADC + Frontend
The AD7768 was chosen for a few different reasons. First of all, it is a 24-Bit ADC suitable for low frequencies and even DC. Further, it features digital signal processing capabilities difficult to implement in a MCU. Most importantly, it allows for a wide range of decimation rates and a highly customizable sample rate so that acquisitions can be matched to the speed of a wavelength selective element. The ADC is connected via SPI. It is used in a pseudo-differential configuration with a 4.096 V reference ADR4540, to measure a voltage range between 0 an 4.096 V. The lowest possible sample rate with the highest decimation of 1024x is 300 Sp/s.

The ADC is paired with a transimpedance photodiode amplifier which is realized upon 1/2 of an AD8622 OpAmp. The values of the feedback network are matched in accordance to the photodiode sensitivity and speed. The photodiodes anode can be connected to ground via R23 or the diode can be reversed biased via R22. Reverse bias is generated through the other 1/2 of the AD8622 buffering an voltage divider R25/R26.

The photodiode is a Hamamatsu S1337-1010. It's spectral response from 190/340-1100 nm enables its use in VIS and certain NIR applications. The large area guarantees a high photosensitivity, and the rise time is fast enough for the desired spectroscopy application.

## Software Examples
The included software provides functions to interface with the hardware on the PCB.

The function `Set_Si514_Frequency()` configures the Si514 and has to be executed after every restart. It expects an array of register values which can be calculated through the Silab Utility provided in /resources/tools.

The functions to read an write from/to the AD7768-1 registers in general feature 8 bit wide addresses and values. An exemption is the result register, which is 24 bit wide an therefore is internally handled as an uint32.

After the ICs are configured, a SYNC pulse is triggered which starts the continuous adc capture. Every time a new sample is ready, a falling edge is presented to the MCU by the adc on !DRDY which is captured through an interrupt, and sets a flag, which enables a readout in the main program loop. Depending upon configuration of the variable `average_mode`, the result is either output through a serial connection right away, or after 256 samples have been averaged.
