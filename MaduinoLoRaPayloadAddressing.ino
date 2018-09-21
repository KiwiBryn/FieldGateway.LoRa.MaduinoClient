/*
  Adapted from LoRa Duplex communication with Sync Word

  Sends temperature & humidity data from Seeedstudio 

  https://www.seeedstudio.com/Grove-Temperature-Humidity-Sensor-High-Accuracy-Min-p-1921.html

  To my Windows 10 IoT Core RFM 9X library

  https://blog.devmobile.co.nz/2018/09/03/rfm9x-iotcore-payload-addressing/
  
*/
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <TH02_dev.h>
const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;       // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

// Field gateway configuration
const char FieldGatewayAddress[] = "LoRaIoT1";
const float FieldGatewayFrequency =  915000000.0;
//const float FieldGatewayFrequency =  433000000.0;
const byte FieldGatewaySyncWord = 0x12 ;

// Payload configuration
const int PayloadSizeMaximum = 64 ;
byte payload[PayloadSizeMaximum] = "";
const byte SensorReadingSeperator = ',' ;

// Manual serial number configuration
const char DeviceId[] = {"Maduino1"};

const int LoopSleepDelaySeconds = 300 ;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  
  Serial.println("LoRa Setup");

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(csPin, resetPin, irqPin);// set CS, reset, IRQ pin

  if (!LoRa.begin(FieldGatewayFrequency)) 
  {            
    Serial.println("LoRa init failed. Check your connections.");
    while (true);
  }

  // Need to do this so field gateways pays attention to messsages from this device
  LoRa.enableCrc();
  LoRa.setSyncWord(FieldGatewaySyncWord);  

  //LoRa.dumpRegisters(Serial);
  Serial.println("LoRa Setup done.");
  
  // Configure the Seeedstudio TH02 temperature & humidity sensor
  Serial.println("TH02 setup");
  TH02.begin();
  delay(100);
  Serial.println("TH02 Setup done");  

  Serial.println("Setup done");  
}


void loop() 
{
  int payloadLength = 0 ;  
  float temperature ;
  float humidity ;

  Serial.println("Loop called");
  memset(payload, 0, sizeof(payload));

  // prepare the payload header with "To" Address length (top nibble) and "From" address length (bottom nibble)
  payload[0] = (strlen(FieldGatewayAddress) << 4) | strlen( DeviceId ) ; 
  payloadLength += 1;

  // Copy the "To" address into payload
  memcpy(&payload[payloadLength], FieldGatewayAddress, strlen(FieldGatewayAddress));
  payloadLength += strlen(FieldGatewayAddress) ;

  // Copy the "From" into payload
  memcpy(&payload[payloadLength], DeviceId, strlen(DeviceId));
  payloadLength += strlen(DeviceId) ;
  
  // Read the temperature and humidity values then display nicely
  temperature = TH02.ReadTemperature();
  humidity = TH02.ReadHumidity();

  Serial.print("T:");
  Serial.print( temperature, 1 ) ;
  Serial.print( "C" ) ;

  Serial.print(" H:");
  Serial.print( humidity, 0 ) ;
  Serial.println( "%" ) ;

  // Copy the temperature into the payload
  payload[ payloadLength] = 't';
  payloadLength += 1 ;
  payload[ payloadLength] = ' ';
  payloadLength += 1 ;
  payloadLength += strlen( dtostrf(temperature, -1, 1, (char*)&payload[payloadLength]));  
  payload[ payloadLength] = SensorReadingSeperator;
  payloadLength += sizeof(SensorReadingSeperator) ;

  // Copy the humidity into the payload
  payload[ payloadLength] = 'h';
  payloadLength += 1 ;
  payload[ payloadLength] = ' ';
  payloadLength += 1 ;
  payloadLength += strlen( dtostrf(humidity, -1, 0, (char *)&payload[payloadLength]));  

  // display info about payload then send it (No ACK) with LoRa unlike nRF24L01
  Serial.print( "RFM9X/SX127X Payload length:");
  Serial.print( payloadLength );
  Serial.println( " bytes" );

  LoRa.beginPacket(); 
  LoRa.write( payload, payloadLength ); 
  LoRa.endPacket();      
  
  Serial.println("Loop done");

  delay(LoopSleepDelaySeconds * 1000l);
}

