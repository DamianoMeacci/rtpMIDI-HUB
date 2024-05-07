/*

Developed for Teensy 4.1 with a switch and a led

version 2024 may

*/

#include <NativeEthernet.h> //Teensy (for Arduino IDE 2.0.4 or later) version 1.59.0
#include <EthernetBonjour.h> // https://github.com/TrippyLighting/EthernetBonjour

#include <USBHost_t36.h> // https://github.com/PaulStoffregen/USBHost_t36
/*
#define SerialMon Serial
#include <AppleMIDI_Debug.h>
*/
#define USE_EXT_CALLBACKS
#include <AppleMIDI.h> //https://github.com/lathoub/Arduino-AppleMidi-Library
/*
MaxNumberOfParticipants set to 5 (default 2) in AppleMIDI_Settings.h
*/

/**********************************************************

Network, Bonjour and pin configuratins

**********************************************************/

//YELLOW A
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xAD }; 
IPAddress staticip(192, 168, 0, 199);
IPAddress autoassignedip(169, 254, 1, 99);
IPAddress autoassignedipDNS(169, 254, 1, 1);
const char BonjourName[] ="YellowBox";
const char BonjourService[] = "YellowBox._apple-midi";

//RED B
/*
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xBC }; 
IPAddress staticip(192, 168, 0, 198);
IPAddress autoassignedip(169,254,10,99);
IPAddress autoassignedipDNS(169, 254, 10, 1);
const char BonjourName[] ="RedBox";
const char BonjourService[] = "RedBox._apple-midi";

*/

const int buttonPin = 28;
const int ledPin = 25;

//////////////////////// Variables /////////////////////////

int buttonState = 0;
int buttonStateOLD;

unsigned long t1 = millis();
unsigned long t_led = 0;
unsigned int blink_length = 200;
unsigned int blink_number = 5;
unsigned int blink_done = 0;
unsigned int led_on = 1;
bool blinking = 0;

USBHost myusb;
USBHub hub1(myusb);
USBHub hub2(myusb);
USBHub hub3(myusb);
USBHub hub4(myusb);
MIDIDevice midi01(myusb);
MIDIDevice midi02(myusb);
MIDIDevice midi03(myusb);
MIDIDevice midi04(myusb);
MIDIDevice midi05(myusb);
MIDIDevice midi06(myusb);
MIDIDevice midi07(myusb);
MIDIDevice midi08(myusb);
MIDIDevice midi09(myusb);
MIDIDevice midi10(myusb);
MIDIDevice * midilist[10] = {
  &midi01, &midi02, &midi03, &midi04, &midi05, &midi06, &midi07, &midi08, &midi09, &midi10
};

//////////////////// UDP communication ////////////////////

int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t&, const APPLEMIDI_NAMESPACE::Exception&, const int32_t);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

void setup()
{
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  
  blink_led(0);

  buttonState = digitalRead(buttonPin);

  if (buttonState) { //pressed:
    Ethernet.begin(mac, staticip);
  } else { //released
    if (Ethernet.begin(mac) == 0) {
      Ethernet.begin(mac, autoassignedip, autoassignedipDNS, autoassignedipDNS, { 255, 255, 0, 0 });
    }
  }
  buttonStateOLD = buttonState;
  
	myusb.begin();
  
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  
  while (Ethernet.linkStatus() == LinkOFF) {
    delay(500);
  }
  
  MIDI.begin(MIDI_CHANNEL_OMNI);

  EthernetBonjour.begin(BonjourName);

  EthernetBonjour.addServiceRecord(BonjourService,
                                   AppleMIDI.getPort(),
                                   MDNSServiceUDP);
  
  // Stay informed on connection status
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
  });


  AppleMIDI.setHandleException(OnAppleMidiException);

  MIDI.setHandleNoteOn(OnNoteOn);
  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
  MIDI.setHandleProgramChange(OnProgramChange);
  MIDI.setHandleAfterTouchChannel(OnAfterTouchChannel);
  MIDI.setHandlePitchBend(OnPitchBend);
  
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

void loop()
{
  MIDI.read();
  EthernetBonjour.run();

  //READ DATA FROM DEVICE AND SEND TO rtp networ device

  for (int port=0; port < 10; port++) {
    if (midilist[port]->read()) {
      uint8_t type =       midilist[port]->getType();
      uint8_t data1 =      midilist[port]->getData1();
      uint8_t data2 =      midilist[port]->getData2();
      uint8_t channel =    midilist[port]->getChannel();
      const uint8_t *sys = midilist[port]->getSysExArray();
      sendToComputer(type, data1, data2, channel, sys);
      blink_led(1);
    }
  }
  blink_led(0);

  buttonState = digitalRead(buttonPin);
  if (buttonState != buttonStateOLD) {
    buttonStateOLD = buttonState;
    if (buttonState) { //pressed:
      Ethernet.begin(mac, staticip);
    } else { //released
      if (Ethernet.begin(mac) == 0) {
        Ethernet.begin(mac, autoassignedip, autoassignedipDNS, autoassignedipDNS, { 255, 255, 0, 0 });
      }
    }
  }
}

// -----------------------------------------------------------------------------
//  Functions
// -----------------------------------------------------------------------------
void sendToComputer(byte type, byte data1, byte data2, byte channel, const uint8_t *sysexarray){
  MIDI.send(type, data1, data2, channel);
}

void blink_led(int i){
   
    t_led = millis();

    if (i && !blinking) {
      t1 = millis();
      blink_done = 0;
      blinking = true;
    } else if (blink_done >= (blink_number * 2)){
      blinking = false;
    }
    
    if (blinking && (t_led > (t1 + blink_length))) {
      if (led_on) {
        led_on = 0;
      } else {
        led_on = 1;
      }
      t1 = millis();
      blink_done++;
    }

    if (led_on) {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
}

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, const APPLEMIDI_NAMESPACE::Exception& e, const int32_t value ) {
  switch (e)
  {
    case APPLEMIDI_NAMESPACE::Exception::BufferFullException:
      //DBG(F("*** BufferFullException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParseException:
      //DBG(F("*** ParseException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::TooManyParticipantsException:
      //DBG(F("*** TooManyParticipantsException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::UnexpectedInviteException:
      //DBG(F("*** UnexpectedInviteException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParticipantNotFoundException:
      //DBG(F("*** ParticipantNotFoundException"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::ComputerNotInDirectory:
      //DBG(F("*** ComputerNotInDirectory"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::NotAcceptingAnyone:
      //DBG(F("*** NotAcceptingAnyone"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::ListenerTimeOutException:
      //DBG(F("*** ListenerTimeOutException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::MaxAttemptsException:
      //DBG(F("*** MaxAttemptsException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::NoResponseFromConnectionRequestException:
      //DBG(F("***:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::SendPacketsDropped:
      //DBG(F("*** SendPacketsDropped"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::ReceivedPacketsDropped:
      //DBG(F("*** ReceivedPacketsDropped"), value);
      break;
  }
}

static void OnNoteOn(byte channel, byte note, byte velocity) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendNoteOn(note, velocity, channel);
  }
  blink_led(1);
}

static void OnNoteOff(byte channel, byte note, byte velocity) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendNoteOff(note, velocity, channel);
  }
  blink_led(1);
}

static void OnAfterTouchPoly(byte channel, byte note, byte pressure) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendAfterTouchPoly(note, pressure, channel);
  }
  blink_led(1);
}

static void OnControlChange(byte channel, byte number, byte value) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendControlChange(number, value, channel);
  }
  blink_led(1);
}

static void OnProgramChange(byte channel, byte number) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendProgramChange(number, channel);
  }
  blink_led(1);
}

static void OnAfterTouchChannel(byte channel, byte pressure) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendAfterTouch(pressure, channel);
  }
  blink_led(1);
}

static void OnPitchBend(byte channel, int bend) {
  for (int port=0; port < 10; port++) {
    midilist[port]->sendPitchBend(bend, channel);
  }
  blink_led(1);
}


////////////////////// FOR FUTURE UPDATES ////////////////////

/*

static void OnSystemExclusive(byte * array, unsigned size) {
  Serial.println(F("SystemExclusive"));
}

static void OnTimeCodeQuarterFrame(byte data) {
  Serial.print(F("TimeCodeQuarterFrame: "));
  Serial.println(data, HEX);
}

static void OnSongPosition(unsigned beats) {
  Serial.print(F("SongPosition: "));
  Serial.println(beats);
}

static void OnSongSelect(byte songnumber) {
  Serial.print(F("SongSelect: "));
  Serial.println(songnumber);
}

static void OnTuneRequest() {
  Serial.println(F("TuneRequest"));
}

static void OnClock() {
  Serial.println(F("Clock"));
}

static void OnStart() {
  Serial.println(F("Start"));
}

static void OnContinue() {
  Serial.println(F("Continue"));
}

static void OnStop() {
  Serial.println(F("Stop"));
}

static void OnActiveSensing() {
  Serial.println(F("ActiveSensing"));
}

static void OnSystemReset() {
  Serial.println(F("SystemReset"));
}
*/