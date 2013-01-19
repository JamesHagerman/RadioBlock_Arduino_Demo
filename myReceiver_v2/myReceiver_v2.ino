// This is a hacked together version of a RadioBlock receiver app for Arduino.
//
// v1: was a frustrating hack
// v2: is at least working to receive data. It's still a mess and can't really 
//      parse out useful information quite yet.
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
#define OUR_ADDRESS   0x1001
#define THEIR_ADDRESS 0x1000

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
  Serial.println("Starting to receive packets..."); // Tell the hardware serial port we're starting
  
  // Setup the status LED
  pinMode(led, OUTPUT);
}

void loop() // run over and over
{
  
  digitalWrite(led, HIGH); // Let the world the arduino is still alive
  
  //==============
  // Sending data:
  
  // Send some arbitrary data to another RadioBlock by building a custom packet:
//  interface.setupMessage(THEIR_ADDRESS); // Start a new message to another RadioBlock
//  interface.addData(4, 200); // Add some arbitrary data to the message
//  interface.sendMessage(); // Actually transmit the message over the air.
  
  // Send some arbitrary data to another RadioBlock in one go.
//  interface.sendData(THEIR_ADDRESS, 'j');
//  Serial.println("Data sent.");
  
  
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
  
  if (interface.readPacket(10)) { 
    Serial.print("We got a packet...");
    
    if (interface.getResponse().getErrorCode() == APP_STATUS_SUCESS){
      Serial.println(" it was a good packet!");
      Serial.print("Length of full packet: ");
      Serial.print(interface.getResponse().getPacketLength(), DEC);
      Serial.print(", Command: ");
      Serial.print(interface.getResponse().getCommandId(), HEX);
      Serial.print(", CRC: ");
      Serial.print(interface.getResponse().getCrc(), HEX);
      Serial.println("");
      
      // Parse the data!
      parseFrameData(interface.getResponse());
      
    } else {
      Serial.println(" it was a bad packet!");
      Serial.print(" The error code was: ");
      Serial.println(interface.getResponse().getErrorCode(), HEX);
    }
    
    Serial.println("");
  }
  
  //=====================
  // Still alive notices:
  digitalWrite(led, LOW);
  delay(10);
}

void parseFrameData(RadioBlockResponse thePacket) {
	
	int frame_data_length = 0;
	// Detected send method:
	// 0 = unknown
	// 1 = sendData()
	// 2 = sendMessage()
	int send_method = -1; 
	
	int command_id = -1;
	unsigned int code_and_type = 0;
	unsigned int payload_code = 0;
	unsigned int payload_data_type = 0;
	
  	// We can use this to determine which commands the sending unit used to build the packet:
	// If the length == 6, the sender used sendData()
	// If the length > 6, the sender used setupMessage(), addData(), and sendMessage() 
	//
	// If the sender used the second method, we need to do more parsing of the payload to pull out
	// the sent data. See "Data or start of payload" below at array offset of 5.
	frame_data_length = thePacket.getFrameDataLength();
	Serial.print("Length of Frame Data: ");
	Serial.println(frame_data_length, DEC);

	if (frame_data_length == 6) {
		send_method = 0;
	} else if (frame_data_length > 6) {
		send_method = 1;
	}

	// The following "meanings" for these bytes are from page 15 of the 
	// SimpleMesh_Serial_Protocol.pdf from Colorado Micro Devices.
	Serial.println("Frame Data: "); 

	//command_id = thePacket.getFrameData()[0]; // This isn't always right. A bug?
	command_id = thePacket.getCommandId();
	Serial.print(" Command ID: ");
	Serial.print(thePacket.getFrameData()[0], HEX); // Command ID (err... this is always a zero!?)
	Serial.print(" actually, that may be wrong. It's actually: ");
	Serial.println(command_id, HEX);
	
	// We should probably switch on Command ID here. Only parse data if we got command 0x22...
	if (command_id == 0x22) {
		Serial.print(" Source address: ");
		Serial.println(thePacket.getFrameData()[1], HEX); // Source address

		Serial.print(" Frame options: ");
		// 0x00 None
		// 0x01 Acknowledgment was requested
		// 0x02 Security was used
		Serial.println(thePacket.getFrameData()[2], HEX); // Frame options

		Serial.print(" Link Quality Indicator: ");
		Serial.println(thePacket.getFrameData()[3], HEX); // link quality indicator

		Serial.print(" Received Signal Strength Indicator: ");
		Serial.println(thePacket.getFrameData()[4], HEX); // Received Signal Strength Indicator

		// Parse Data or Payload:
		// First a note on the libraries treatment of payloads: It doesn't do any work to pull them out 
		// of the packet. Maybe in the future? Maybe this code should be merged into the library?
		//
		// The meaning of the byte at offset 5 depend on how the sender built the packet. We can use
		// the getFrameDataLength() command to determine how the packet was built (see comments above).
		
		if (send_method == 0) {
			// If the sender used sendData(), this byte is the data the sender was meaning to send. 
			// All other payload bytes can be ignored. You have your data!
			Serial.print(" Sent Data: ");
			Serial.println(thePacket.getFrameData()[5], HEX); // The actual data
			
		} else if (send_method == 1) {
			// If the sender used sendMessage(), this byte is a combination of of the code and the data type of the 
			// variable the sender was sending. It seems like the code is arbitrary and can be set by the programmer.
			// The data type is determined by addData()'s overloaded second paramater.
			//
			// The lower bits are the data type.
			// The upper bits are the code.
			//
			// So, this byte is ((code << 4) | (type))
			//
			// The data types are defined in RadioBlock.cpp in the library.
			
			code_and_type = thePacket.getFrameData()[5];
			Serial.print(" Encoded send code and original data type: ");
			Serial.println(code_and_type, HEX); // The actual data
			
			payload_data_type = code_and_type & 0xf;
			payload_code = (code_and_type >> 4) & 0xf;
			
			Serial.print("  The sent code was (in hex): ");
			Serial.println(payload_code, HEX);
			Serial.print("  The original data type was: ");
			Serial.println(payload_data_type);
			
			
			if (payload_data_type == 1) {
				Serial.println("   Data type is TYPE_UINT8. Data:");
				Serial.print("    The data: ");
				Serial.println(thePacket.getFrameData()[6]); 
			} else if (payload_data_type == 2) {
				Serial.println("   Data type is TYPE_INT8. High and low bytes:");
				Serial.print("    High part: ");
				Serial.println(thePacket.getFrameData()[6]); 
				Serial.print("    Low part: ");
				Serial.println(thePacket.getFrameData()[7]);
			} else if (payload_data_type == 3) {
				Serial.println("   Data type is TYPE_UINT16. High and low bytes:");
				Serial.print("    High part: ");
				Serial.println(thePacket.getFrameData()[6]); 
				Serial.print("    Low part: ");
				Serial.println(thePacket.getFrameData()[7]);
			} else if (payload_data_type == 4) {
				Serial.println("   Data type is TYPE_INT16. High and low bytes:");
				Serial.print("    High part: ");
				Serial.println(thePacket.getFrameData()[6]); 
				Serial.print("    Low part: ");
				Serial.println(thePacket.getFrameData()[7]);
			} else if (payload_data_type == 5) {
				Serial.println("   Data type is TYPE_UINT32. Four bytes:");
				Serial.print("    MSB: ");
				Serial.println(thePacket.getFrameData()[6]); 
				Serial.print("    : ");
				Serial.println(thePacket.getFrameData()[7]);
				Serial.print("    :");
				Serial.println(thePacket.getFrameData()[8]);
				Serial.print("    LSB:");
				Serial.println(thePacket.getFrameData()[9]);
			} else {
				Serial.println("   Data type is not coded for yet...");
				// Debugging: 
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[6]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[7]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[8]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[9]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[10]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[11]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[12]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[13]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[14]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[15]);
				Serial.print("   Raw byte:");
				Serial.println(thePacket.getFrameData()[16]);
				// End debugging
			}
			
		} else {
			// Unkown send method!
			Serial.println("We got a packet smaller then expected... What happened?");
		}
	} else {
		Serial.println("The sender used a non-data command. We can't deal with that yet.");
		Serial.print("The Command ID (in hex) was: ");
		Serial.println(command_id, HEX);
	}
	
	
}

