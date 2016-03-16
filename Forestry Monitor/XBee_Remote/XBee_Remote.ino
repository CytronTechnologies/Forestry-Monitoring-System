#include <SoftwareSerial.h>
#include <String.h>

#define ManualButton 7  //manual activation button is connected to pin D7
#define SmokeSensor A1 // Smoke detector is connected to pin A1 (analog output)
#define NodeID  1 //node ID

SoftwareSerial XbeeSerial(2,3); //using software serial, make sure the jumpers are at D2 and D3 on Xbee shield
int SmokeValue = 0;
uint8_t message[11] = {0};  //array for message sending to Tower

void setup()
{
  message[0] = 0xFF; //header for the package
  XbeeSerial.begin(9600); // XBee shield baud rate 
  Serial.begin(9600);    //for computer communication
  pinMode(ManualButton, INPUT);
  delay(2000);
}
 
void loop()
{
  SmokeValue = analogRead(SmokeSensor);
//  if (Serial.available()) // if there is incoming serial data
//  {
//   switch(Serial.read()) // read the character
//   {
//     case 't': // if the character is 't'
//       SendXbeeMessage(); // send the text message
//       break;     
//   } 
//  }
 
  if (XbeeSerial.available()){ // if the shield has something to say
    Serial.write(XbeeSerial.read()); // display the output of the shield
  }

  if ((SmokeValue > 150) || (digitalRead(ManualButton) == LOW))
  {
    while(digitalRead(ManualButton) == LOW);
	  SendXbeeMessage(); // send the text message
	  delay(10000); //delay 10 seconds after send to avoid always sending the same message
  }
}
 

void SendXbeeMessage()
{
  Serial.println("Sending Message to Xbee...Detected Fire");
  message[1] = NodeID/256;
  message[2] = NodeID%256;
  message[3] = 0; // 0- smoke sensor trigger, 1 -temp sensor trigger
  AppendADC();
  XbeeSerial.write(message,11); // send message to tower, node 1, smoke, value 300
  delay(100);
  Serial.println("Message Sent.");
}
 
void AppendADC()
{
  uint16_t temp = 0;
  message[4] = SmokeValue/256 ;//(SmokeValue / 1000) + '0';
  message[5] = SmokeValue%256;//((SmokeValue % 1000) / 100) + '0';
  message[6] = 0;//((SmokeValue % 100) / 10) + '0';
  message[7] = 0;//(SmokeValue % 10) + '0';
  temp = getChecksum();
  message[8] = temp/256;
  message[9] = temp%256;
  message[10] = 0xfe; //makesure the last element of string is null
}

uint16_t getChecksum()
{
  uint16_t val = 0;
  for(int i = 1;i<8;i++)
  {
    val += message[i];
  }
  return -val; // complement of checksum
  
}


