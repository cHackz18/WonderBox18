// ***************************************************************
// *        Behringer XR18 Arduino MIDI Remote Control           *
// *                   By Vern Graner                            *
// ***************************************************************
// * This code connects to WiFI and sends OSC commands to an     *
// * Behringer XR18. You have to enter an IP address for the     *
// * mixer for this demo to work                                 *
// ***************************************************************
// * Rev Info: V1.0a (VLG) 01-22-2018 Basic proof of concept     *
// *         : V3.2a (VLG) 01-27-2018 CH1&2 mute, FOH/HOSTS PTT  *
// *         : V3.2a (VLG) 01-27-2018 PROTOTYPE1 NO OLED         *
// *         : V3.5a (VLG) 01-28-2018 Use array for OSC commands *
// *         : V3.5b (VLG) 02-06-2018 Cough Box V1               *
// *         : V3.5C (VLG) 03-07-2018 Tasked #4 button for FX    *
// ***************************************************************

// Include the button library
// http://www.michael.net.nz
#include <Button.h>

// Include the WiFi library
// https://www.arduino.cc/en/Reference/WiFi

#include <WiFi.h>    // For WiFi
#include <WiFiUdp.h> // For WiFi

// Include the OSC message handler
// https://github.com/CNMAT/OSC
#include <OSCMessage.h> // For OSC support

// Include the MIDI stuff
// https://github.com/FortySevenEffects/arduino_midi_library
#include <MIDI.h> // MIDI support
#include <midi_Defs.h>
#include <midi_Message.h>
#include <midi_Namespace.h>
#include <midi_Settings.h>

// Assign Button Functions
// ***************************************************

// define button arrays Format is "<OSC COMMAND>","<OSC PARAMETER>",

const char *button1Command[] = {
    "/ch/01/mix/on", "OFF", // Button 1 Payload 0: Ch 1 UN-MUTE
    "/ch/01/mix/on", "ON"   // Button 1 Payload 1: Ch 1 MUTE
};

const char *button2Command[] = {
    "/ch/02/mix/on", "OFF", // Button 2 Payload 0: Ch 2 UN-MUTE
    "/ch/02/mix/on", "ON"   // Button 2 Payload 1: Ch 2 MUTE
};

const char *button3Command[] = {
    "/ch/03/mix/on", "OFF", // Button 3 Payload 0: Ch 3 UN-MUTE
    "/ch/03/mix/on", "ON"   // Button 3 Payload 1: Ch 3 MUTE
};

const char *button4Command[] = {
    //  "/ch/04/mix/on","OFF",       // Button 4 Payload 0: Ch 4 UN-MUTE
    //  "/ch/04/mix/on","ON"         // Button 4 Payload 1: Ch 4 MUTE

    "/ch/02/mix/09/level", "0",      // Button 4 Payload 0: Ch 2 FX3 send ON
    "/ch/02/mix/09/level", "-127", // Button 4 Payload 1: Ch 2 FX3 send OFF
};

// WiFi setup
// char ssid[] = "ACA";                      // your network SSID (name)
// char pass[] = "REDACTED";                 // your network password

char ssid[] = "LOCAL";    // your network SSID (name)
char pass[] = "REDACTED"; // your network password

WiFiUDP Udp; // A UDP instance to let us send and receive packets over UDP
// const IPAddress outIp(192,168,100,6);     // IP of the XR18 in Comma
// Separated Octets, NOT dots!
// IP of the XR18 in Comma Separated Octets, NOT dots!
const IPAddress outIp(192, 168, 1, 15); 
const unsigned int outPort =
    10024; // remote port to receive OSC X-AIR is 10024, X32 is 10023
const unsigned int localPort = 8888; // local port to listen for OSC packets
                                     // (actually not used for sending)

// Pin combos for Cough Drop Box
// Button/LED pin combos Button 0 15/25
// Button/LED pin combos Button 1 27/34
// Button/LED pin combos Button 0 32/39
// Button/LED pin combos Button 3 33/26

// Setup LED pins
int ledPin[] = {15, 27, 32, 33}; // define the pins for the LEDs COUGH DROP
// int ledPin[]={23,22,14,32};      // define the pins for the LEDs on
// BREADBOARD
int ledCount = 4;              // set the number of LEDs in the loop
int ledState[] = {0, 0, 0, 0}; // current state of the button
int led[] = {0, 0, 0, 0};      // current state of the button
#define OFF 0                  // Friendly names for LED states
#define ON 1                   // Friendly names for LED states
#define BLINK 2                // Friendly names for LED states

// Setup button pins
int myButtons[] = {25, 34, 39,
                   26}; // define the pins for the buttons on COUGH DROP
// int myButtons[] = {16,19,18,5,4};   // define the pins for the buttons on
// breadboard
int buttonCount = 5; // set the number of buttons in the loop

// Assign pins for button handler routine COUGH DROP
Button button1(25); // Mic 1
Button button2(34); // Mic 2
Button button3(39); // Mic 3
Button button4(26); // Mic 4
Button button5(36); // LOCK switch

// Assign pins for button handler routine BREADBOARD
// Button button1(16); // Mic 1
// Button button2(19); // Mic 2
// Button button3(18); // Mic 3
// Button button4(5);  // Mic 4
// Button button5(4);  // LOCK switch

// LED handler for ON, OFF and BLINK
#define OFF 0
#define ON 1
#define BLINK 2
unsigned long previousMillis = 0; // for non-blocking LED blink routine
const long interval = 150;        // how fast the LED blinks
int ledBlink = 0;                 // variable for blink routine

// Button Toggle Routine
bool setToToggle = true; // set default toggle state
int toggleFlag1 = 0;     // flag for toggle state
int toggleFlag2 = 0;     // flag for toggle state
int toggleFlag3 = 0;     // flag for toggle state
int toggleFlag4 = 0;     // flag for toggle state

// Button & LED Pair pin numbers:
// Button0 = 15, LED0 = 25
// Button1 = 27, LED0 = 34
// Button2 = 32, LED0 = 39
// Button3 = 33, LED0 = 26

// Setup button type
// "0" is for follow state 1 is for toggle state
int buttonToggle[] = {0, 0, 0, 0}; 

// Define program Variables
int buttonState[] = {0, 0, 0, 0};     // current state of the button
int lastButtonState[] = {0, 0, 0, 0}; // previous state of the button

// MIDI OSC Hex converter
int commandLength = 0;
byte bigMidiCommand[64]; // maximum command length in bytes
byte midiHeader[] = {0xF0, 0x00, 0x20, 0x32, 0x32}; // XR18 OSC preamble
byte midiSpacer[] = {0x20};                         // XR18 OSC post-amble
byte midiFooter[] = {0xF7};                         // XR18 OSC post-amble

// Counter variables
int count;        // Command Counter
int i = 0;        // Button counter
int j = 0;        // bigMIDIcommand counter

//
// Setup MIDI hardware
//
// setup MIDI on TX pin
HardwareSerial Serial1(2);
// create a MIDI object called midiOut
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, midiOut);

void setup() {

  // Start Button Handler
  button1.begin(); // Mic 1
  button2.begin(); // Mic 2
  button3.begin(); // Mic 3
  button4.begin(); // Mic 4
  button5.begin(); // Lock Switch

  // Setup Serial Out
  Serial.begin(115200); // DEBUG window
  Serial1.begin(31250); // setup MIDI output
  midiOut.begin();

  // Setup pin mode for buttons
  for (int ix = 0; ix < buttonCount; ix++) {
    pinMode(myButtons[i], INPUT_PULLUP); // initialize the pin for input
  }

  // Setp pin mode for LEDs
  // step through 0 to ledCount (which is 4 on this box)
  for (int ix = 0; ix < ledCount; ix++) { 
    // initialize the LED as an output
	pinMode(ledPin[i], OUTPUT);
  }

  // Print code version information to debug screen
  Serial.println();
  Serial.println("********************************************************");
  Serial.println("*     WiFi/MIDI Mixer Control V3.5c 2018/03/07         *");
  Serial.println("*   Created by Vern Graner for use with X-AIR Mixers   *");
  Serial.println("********************************************************");
  Serial.println("* Contact: vern@graner.com  http://www.notepad.org/TAE *");
  Serial.println("********************************************************");
  Serial.println();

  // Connect to WiFi network
  Serial.print("Target WiFi SSID: ");
  Serial.println(ssid);
  Serial.print("Connecting");

  WiFi.begin(ssid, pass);
  int WiFiWait = 0;
  for(unsigned int ix = 0; ix != WIFI_TIMEOUT; ++ix)
  while (WiFiWait < 60 &&
         WiFi.status() !=
             WL_CONNECTED) { // wait for either timeout or wifi to connect

    for (int count = 0; count < 4; count++) { // step by 4 cells in array
      digitalWrite(ledPin[count],
                   HIGH); // blink LED to show we are waiting on WiFi
      delay(100);
      digitalWrite(ledPin[count],
                   LOW); // blink LED to show we are waiting on WiFi
      delay(100);
    }

    Serial.print(".");
    WiFiWait++; // increment for timeout waiting for WiFi
  }

  // Check if WiFi connected
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(ledPin[0], LOW); // |							  
	digitalWrite(ledPin[1], LOW); // | Once we are connected, make sure all the LEDs are off
    digitalWrite(ledPin[2], LOW); // |
    digitalWrite(ledPin[3], LOW); // |
    Serial.println("");
    Serial.println("Connected!");
  }
  else {
    digitalWrite(ledPin[0], LOW); // |
    digitalWrite(ledPin[1], LOW); // | Even if not connected, make sure all the LEDs are off
    digitalWrite(ledPin[2], LOW); // |
    digitalWrite(ledPin[3], LOW); // |
    Serial.println("");
    Serial.println("Connection Timed Out!");
    Serial.println("NOTE: Box is MIDI ONLY at this time!");
  }

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(localPort);
  Serial.println("");

  Serial.print("Mixer IP: ");
  Serial.println(outIp);
  Serial.print("Mixer Port: ");
  Serial.println(outPort);
  Serial.println("----------------------------------------------------------");
}

void loop() { // Main Loop

  // ****************************************
  // LOCK switch mode selection             *
  // ***************************************************************************************
  if (button5.pressed()) {             // Read the LOCK switch
    Serial.println("Toggle mode OFF"); // DEBUG output
    setToToggle = false;               // set the mode
    toggleFlag1 = 0;
    toggleFlag2 = 0;
    toggleFlag3 = 0;
    toggleFlag4 = 0;
  }
  if (button5.released()) {           // Read the LOCK switch
    Serial.println("Toggle mode ON"); // DEBUG output
    setToToggle = true;               // set the mode
    toggleFlag1 = 0;
    toggleFlag2 = 0;
    toggleFlag3 = 0;
    toggleFlag4 = 0;
  }
  // ***************************************************************************************

  // ****************************************
  // Button Read Routines                   *
  // ***************************************************************************************
  // ***************************************************************************************
  // ***************************************************************************************
  // ***************************************************************************************

  //********************************************************************
  // BUTTON 1 Handler BEGIN
  //********************************************************************

  // Momentary payload delivery
  //************************************
  if (setToToggle == false && button1.toggled()) {
    if (button1.read() == Button::PRESSED) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button1Command[0], button1Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button1Command[0]); // Send Button Message Command
      msg.add(button1Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[0] = ON;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("Button 1 Payload 0");
      Serial.print("OSC Message: [");
      Serial.print(button1Command[0]);
      Serial.print(" ");
      Serial.print(button1Command[1]);
      Serial.println("]");

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button1Command[2], button1Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button1Command[2]);
      msg2.add(button1Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[0] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("Button 1 Payload 1");
      Serial.print("OSC Message: [");
      Serial.print(button1Command[2]);
      Serial.print(" ");
      Serial.print(button1Command[3]);
      Serial.println("]");
    }
  }
  // Button 1 Momentary END

  // BUTTON 1- Toggled payload delivery
  //************************************
  if (setToToggle == true && button1.pressed()) {
    toggleFlag1 = !toggleFlag1;
    if (toggleFlag1 == 1) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button1Command[0], button1Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button1Command[0]); // Send Button Message Command
      msg.add(button1Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[0] = BLINK;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("Button 1 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button1Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button1Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button1Command[2], button1Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button1Command[2]);
      msg2.add(button1Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[0] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println(""); // to DEBUG screen
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("Button 1 Payload 1"); // to DEBUG screen
      Serial.print("OSC Message: [");       // to DEBUG screen
      Serial.print(button1Command[2]);      // to DEBUG screen
      Serial.print(" ");                    // to DEBUG screen
      Serial.print(button1Command[3]);      // to DEBUG screen
      Serial.println("]");                  // to DEBUG screen

    } // END of ELSE
  }   // End of Button Press Detect
  //********************************************************************
  // BUTTON 1 Handler END
  //********************************************************************

  //********************************************************************
  // button2 Handler BEGIN
  //********************************************************************

  // Momentary payload delivery
  //************************************
  if (setToToggle == false && button2.toggled()) {
    if (button2.read() == Button::PRESSED) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button2Command[0], button2Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button2Command[0]); // Send Button Message Command
      msg.add(button2Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[1] = ON;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button2 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button2Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button2Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button2Command[2], button2Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button2Command[2]);
      msg2.add(button2Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[1] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println(""); // to DEBUG screen
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button2 Payload 1"); // to DEBUG screen
      Serial.print("OSC Message: [");      // to DEBUG screen
      Serial.print(button2Command[2]);     // to DEBUG screen
      Serial.print(" ");                   // to DEBUG screen
      Serial.print(button2Command[3]);     // to DEBUG screen
      Serial.println("]");                 // to DEBUG screen
    }
  }
  // button2 Momentary END

  // button2- Toggled payload delivery
  //************************************
  if (setToToggle == true && button2.pressed()) {
    toggleFlag2 = !toggleFlag2;
    if (toggleFlag2 == 1) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button2Command[0], button2Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button2Command[0]); // Send Button Message Command
      msg.add(button2Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[1] = BLINK;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button2 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button2Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button2Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button2Command[2], button2Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button2Command[2]);
      msg2.add(button2Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[1] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println(""); // to DEBUG screen
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button2 Payload 1"); // to DEBUG screen
      Serial.print("OSC Message: [");      // to DEBUG screen
      Serial.print(button2Command[2]);     // to DEBUG screen
      Serial.print(" ");                   // to DEBUG screen
      Serial.print(button2Command[3]);     // to DEBUG screen
      Serial.println("]");                 // to DEBUG screen

    } // END of ELSE
  }   // End of Button Press Detect
  //********************************************************************
  // button2 Handler END
  //********************************************************************

  //********************************************************************
  // button3 Handler BEGIN
  //********************************************************************

  // Momentary payload delivery
  //************************************
  if (setToToggle == false && button3.toggled()) {
    if (button3.read() == Button::PRESSED) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button3Command[0], button3Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button3Command[0]); // Send Button Message Command
      msg.add(button3Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[2] = ON;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button3 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button3Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button3Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button3Command[2], button3Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button3Command[2]);
      msg2.add(button3Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[2] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println(""); // to DEBUG screen
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button3 Payload 1"); // to DEBUG screen
      Serial.print("OSC Message: [");      // to DEBUG screen
      Serial.print(button3Command[2]);     // to DEBUG screen
      Serial.print(" ");                   // to DEBUG screen
      Serial.print(button3Command[3]);     // to DEBUG screen
      Serial.println("]");                 // to DEBUG screen
    }
  }
  // button3 Momentary END

  // button3- Toggled payload delivery
  //************************************
  if (setToToggle == true && button3.pressed()) {
    toggleFlag3 = !toggleFlag3;
    if (toggleFlag3 == 1) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button3Command[0], button3Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button3Command[0]); // Send Button Message Command
      msg.add(button3Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[2] = BLINK;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button3 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button3Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button3Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button3Command[2], button3Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button3Command[2]);
      msg2.add(button3Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[2] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println(""); // to DEBUG screen
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button3 Payload 1"); // to DEBUG screen
      Serial.print("OSC Message: [");      // to DEBUG screen
      Serial.print(button3Command[2]);     // to DEBUG screen
      Serial.print(" ");                   // to DEBUG screen
      Serial.print(button3Command[3]);     // to DEBUG screen
      Serial.println("]");                 // to DEBUG screen

    } // END of ELSE
  }   // End of Button Press Detect
  //********************************************************************
  // button3 Handler END
  //********************************************************************

  //********************************************************************
  // button4 Handler BEGIN
  //********************************************************************

  // Momentary payload delivery
  //************************************
  if (setToToggle == false && button4.toggled()) {
    if (button4.read() == Button::PRESSED) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button4Command[0], button4Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button4Command[0]); // Send Button Message Command
      msg.add(button4Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[3] = ON;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button4 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button4Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button4Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button4Command[2], button4Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button4Command[2]);
      msg2.add(button4Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[3] = OFF;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button4 Payload 1");
      Serial.print("OSC Message: [");
      Serial.print(button4Command[2]);
      Serial.print(" ");
      Serial.print(button4Command[3]);
      Serial.println("]");
    }
  }
  // button4 Momentary END

  // button4- Toggled payload delivery
  //************************************
  if (setToToggle == true && button4.pressed()) {
    toggleFlag4 = !toggleFlag4;
    if (toggleFlag4 == 1) {

      //*******************************
      //* MIDI OSC Message Send       *
      //***************************************
      midiBuildCommand(button4Command[0], button4Command[1]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      //* WiFi OSC Message Send       *
      //***************************************
      OSCMessage msg(button4Command[0]); // Send Button Message Command
      msg.add(button4Command[1]);        // Send Button Message Argument
      Udp.beginPacket(outIp, outPort);
      msg.send(Udp);
      Udp.endPacket();
      msg.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[3] = BLINK;

      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println("");
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button4 Payload 0");
      Serial.print("OSC Message: [");  // to DEBUG screen
      Serial.print(button4Command[0]); // to DEBUG screen
      Serial.print(" ");               // to DEBUG screen
      Serial.print(button4Command[1]); // to DEBUG screen
      Serial.println("]");             // to DEBUG screen

    } else {

      //*******************************
      // *MIDI OSC Message Send BEGIN *
      //***************************************
      midiBuildCommand(button4Command[2], button4Command[3]);
      midiOut.sendSysEx(commandLength, bigMidiCommand, true); // send SysEx
      //***************************************

      //*******************************
      // *WiFi OSC Message Send BEGIN *
      //***************************************
      OSCMessage msg2(button4Command[2]);
      msg2.add(button4Command[3]);
      Udp.beginPacket(outIp, outPort);
      msg2.send(Udp);
      Udp.endPacket();
      msg2.empty();
      //***************************************

      //*******************************
      //* Set Indicator LED           *
      //***************************************
      led[3] = OFF;
      //*******************************
      //* DEBUG Output                *
      //*******************************
      Serial.println(""); // to DEBUG screen
      Serial.print("led[0]:");
      Serial.print(led[0]);
      Serial.print(" led[1]:");
      Serial.print(led[1]);
      Serial.print(" led[2]:");
      Serial.print(led[2]);
      Serial.print(" led[3]:");
      Serial.println(led[3]);
      Serial.println("button4 Payload 1"); // to DEBUG screen
      Serial.print("OSC Message: [");      // to DEBUG screen
      Serial.print(button4Command[2]);     // to DEBUG screen
      Serial.print(" ");                   // to DEBUG screen
      Serial.print(button4Command[3]);     // to DEBUG screen
      Serial.println("]");                 // to DEBUG screen

    } // END of ELSE
  }   // End of Button Press Detect
  //********************************************************************
  // button4 Handler END
  //********************************************************************

  // *******************************
  // LED HANDLER
  // *******************************
  unsigned long currentMillis = millis(); // fetch the time
  if (currentMillis - previousMillis >=
      interval) // Check to see if we need to change LED state
  {             //
    previousMillis = currentMillis; // save the last time you blinked the LED
    if (ledBlink == 0) // if the LED is off turn it on and vice-versa:
    {                  //
      ledBlink = 1;    //
    } else {           //
      ledBlink = 0;    //
    }                  //
  }

  for (int i = 0; i < ledCount; i++) // Step through all the LEDs
  {
    if (led[i] == BLINK) // Check to see if the mode is blink
    {
      digitalWrite(ledPin[i],
                   ledBlink); // set the LED with the ledState of the variable:
    }

    if (led[i] == ON) // Check to see if the mode is ON
    {
      digitalWrite(ledPin[i], HIGH); // Set the led pin HIGH (on)
    }

    if (led[i] == OFF) //  Check to see if the mode is blink
    {
      digitalWrite(ledPin[i], LOW); // Set the led pin LOW (off)
    }
  }

  // **********************************************************
} // End of main loop

// Subroutines aka functions
// **********************************************************

// ***********************************
// Build the MIDI command from the   *
// two components of the OSC command *
// *************************************************************************
void midiBuildCommand(
    String body1,
    String body2) { // Pass OSC Command and Argument as body1, body2
  commandLength = 0;
  for (int j = 0; j < sizeof(midiHeader); j++) {
    bigMidiCommand[commandLength] =
        midiHeader[j]; // insert header into bigMidiCommand
    commandLength++;
  }

  for (int j = 0; j < body1.length(); j++) {
    bigMidiCommand[commandLength] =
        body1[j]; // insert OSC Command into bigMidiCommand
    commandLength++;
  }

  for (int j = 0; j < sizeof(midiSpacer); j++) {
    bigMidiCommand[commandLength] =
        midiSpacer[j]; // insert a space between OSC command and argument
    commandLength++;
  }

  for (int j = 0; j < body2.length(); j++) {
    bigMidiCommand[commandLength] =
        body2[j]; // insert OSC Argument into bigMidiCommand
    commandLength++;
  }

  for (int j = 0; j < sizeof(midiFooter); j++) {
    bigMidiCommand[commandLength] =
        midiFooter[j]; // insert OSC footer into bigMidiCommand
    commandLength++;
  }

  // DEBUG print the HEX string being sent to the MIDI port
  Serial.println("MIDI Message in HEX:");
  for (int j = 0; j < 29; j++) {
    bigMidiCommand[commandLength] = midiFooter[j];
    Serial.print("0x");
    Serial.print(bigMidiCommand[j], HEX);
    Serial.print(",");
  }
  Serial.println("");
}
