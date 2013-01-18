// This is a hacked together version of a RadioBlock sender app for Arduino.
//
// v1: was a frustrating hack
// v2: is at least working to send single characters. It's still a mess. 
// 
// I just kinda hacked whatever I could together to get this. I'll write up more of 
// what I learned when I get a chance.
//
// I'll also probably make a few methods to make parsing the methods a bit easier.
// Hopefully they can get added to the library in the future.
// James Hagerman : jamisnemo : jamis@zenpirate.com : zenpirate.com
// 
// No license yet. Be nice.

#include <SoftwareSerial.h>
#include <RadioBlock.h>

//Pins connected to RadioBlock pins 1/2/3/4

// The RadioBlockSerialInterface inherets from RadioBlock. So, the interface gets 
// all of the methods attached to the RadioBlock class. So we only need an 
// instance of RadioBlockSerialInterface for everything... 
RadioBlockSerialInterface interface = RadioBlockSerialInterface(5,4,3,2);

// Set our known network addresses. How do we deal with unexpected nodes...?
#define OUR_ADDRESS   0x1000
#define THEIR_ADDRESS 0x1001

// Status LED
int led = 13;

void setup()  
{
  // Configure the power lines and setup the SoftwareSerial port
  interface.begin();  

  // Give the RadioBlock some time to init
  delay(500);

  // Tell the world we are alive  
  interface.setLED(true);
  delay(25);
  interface.setLED(false);
  delay(25);
  interface.setLED(true);
  delay(25);
  interface.setLED(false);
  delay(25);
  interface.setLED(true);
  delay(25);
  interface.setLED(false);
  
  // We need to set these values so other RadioBlocks can find us
  interface.setChannel(15);
  interface.setPanID(0xBAAD);
  interface.setAddress(OUR_ADDRESS);
    
  // Setup the hardware serial port (tx/rx lines on the Arduino)
  Serial.begin(9600); 
  Serial.println("Starting to send data..."); // Tell the hardware serial port we're starting
  
  // Setup the status LED
  pinMode(led, OUTPUT);
}

void loop() // run over and over
{
  
  digitalWrite(led, HIGH); // Let the world the arduino is still alive
  
  //==============
  // Sending data:
  
  // Send some arbitrary data to another RadioBlock by building a custom packet:
  interface.setupMessage(THEIR_ADDRESS); // Start a new message to another RadioBlock
  interface.addData(0xf, 'j'); // Add some arbitrary data to the message
  interface.addData(0xe, 0xffff); // Add some arbitrary data to the message
  interface.sendMessage(); // Actually transmit the message over the air.
  
  // Send some arbitrary data to another RadioBlock in one go.
//  interface.sendData(THEIR_ADDRESS, 'j');
  Serial.println("Data sent.");
  
  
  
  //===================
  // Getting data:
  
  // This won't return if the RadioBlock we were trying to communicate with never responds ( If it's
  // powered down, for example). This is a bad thing... but it lets us test to make sure we're 
  // ACTUALLY able to see the other board. You'll need to reset the Arduino if this ever gets hit.
  //interface.readPacketUntilAvailable();
  
  
  // This will tell the RadioBlock to listen for some number of miliseconds for a response.
  // This should return either true or false depending on whether a reponse ever came in from
  // the RadioBlock we were trying to send data to.
  
//  Serial.print("readPacket called with a 1000 millisecond timeout got back: ");
//  Serial.println(interface.readPacket(1000));
  
//  if (interface.readPacket(10)) { 
//    Serial.print("We got a response packet...");
//    
//    if (interface.getResponse().getErrorCode() == APP_STATUS_SUCESS){
//      Serial.println(" it was a good packet!");
//      Serial.print("Len: ");
//      Serial.print(interface.getResponse().getPacketLength(), DEC);
//      Serial.print(", Command: ");
//      Serial.print(interface.getResponse().getCommandId(), HEX);
//      Serial.print(", CRC: ");
//      Serial.print(interface.getResponse().getCrc(), HEX);
//      Serial.println("");
//      
//      Serial.println("Data: ");
//      Serial.println(interface.getResponse().getFrameData()[0]);
//      Serial.println(interface.getResponse().getFrameData()[1]);
//      Serial.println(interface.getResponse().getFrameData()[2]);
//      
//    } else {
//      Serial.println(" it was a bad packet!");
//      Serial.print(" The error code was: ");
//      Serial.println(interface.getResponse().getErrorCode(), HEX);
//    }
//    
//  }
  
  //=====================
  // Still alive notices:
  digitalWrite(led, LOW);
  delay(2000);
}

