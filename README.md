# rtpMIDI-HUB
A project developed using the Arduino IDE to utilize a Teensy 4.1 as an interface capable of connecting USB MIDI peripherals with the rptMIDI (AppleMIDI) protocol.

## Description:
The project harnesses the power of USB Host and leverages the network interface of the Teensy 4.1 from PRJC (https://www.pjrc.com/store/teensy41.html) as a conduit for linking USB MIDI devices to the Ethernet network. Crafted within the confines of the Arduino IDE (https://www.arduino.cc/), this endeavor is augmented with an LED and a switch, offering the flexibility of toggling between two distinct Ethernet configurations.

This system boasts the remarkable capability of interfacing with a plethora of USB peripherals, accommodating up to 10 simultaneous connections, and seamlessly communicating with any device that supports the rtpMIDI protocol.

## For Mac OS users:
Configuring the "MIDI Network Setup" is effortlessly achieved through the dedicated window nestled within the system application "Audio MIDI Setup." This user-friendly interface facilitates the establishment of MIDI connections with unparalleled ease and efficiency.

## For Windows OS users:
Embarking on the Windows journey requires the installation of Tobias Erichsen's rtpMIDI application (https://www.tobias-erichsen.de/software/rtpmidi.html) alongside Apple Bonjour on your system. These tools form the backbone of MIDI communication, enabling seamless integration with your device. With these software components in place, you're primed to delve into the rich world of MIDI connectivity and unleash the full potential of your setup.

## Using more than two computers connected to rtpMIDI-HUB
The AppleMIDI library, in its initial configuration, allows for a maximum of two computers connected to the device.
If you want to use more than two computers, you need to modify the MaxNumberOfParticipants variable in AppleMIDI_Settings.h and optionally the MaxNumberOfComputersInDirectory variable in the same file.
