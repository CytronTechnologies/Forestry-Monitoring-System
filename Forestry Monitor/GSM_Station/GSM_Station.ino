#include <SoftwareSerial.h>
#include <String.h>
 
SoftwareSerial gprsSerial(7,8); //using software serial, make sure the jumpers are at Software (near to SIM900)
String SMSMessage = "This is from Forest Monitoring system, Node #node# has detected #type# with value of #val#.";  //array for SMSmessage sending to phone number
String toSend = "";
String event[2]= {"smoke", "temperature"};
int ReceiveAlarm = 0; // status flag
const char SMSnumber1[] = "AT+CMGS= \"+60198212201\"";
const char SMSnumber2[] = "AT+CMGS= \"+60124073597\"";
uint8_t recv[10] = {0}; // receive buffer from nodes

void setup()
{
  gprsSerial.begin(19200); // GPRS shield baud rate 
  Serial.begin(9600);    //to communicate with XBee
  delay(500);
}
 
void loop()
{
 
  if (Serial.available()>0) // if there is incoming serial data from XBee
  {
     toSend = "";
     memset(recv, '\0', sizeof(recv));
	 
	 /* valid data packets from node (11 bytes): 
	 0xff, 
	 Hbyte of node ID, 
	 LByte of node ID, 
	 type,
	 Hbyte of _smokeVal, 
	 LByte of _smokeVal, 
	 Hbyte of _tempVal, 
	 LByte of _tempVal, 
	 Hbyte of checksum,
	 LByte of checksum,
	 0xfe
	 */
     if(Serial.read()== 0xFF) // check if it is header of the package
     {
        //if header is correct, get the rest 10 data and store them into receive buffer
		    Serial.readBytes(recv, sizeof(recv));
        
        //footer verification  
        if(recv[9]!=0xfe) return;
        
        uint16_t _node = recv[0]*256 + recv[1];
        uint8_t _type = recv[2];
        uint16_t _smokeVal = recv[3]*256 + recv[4];
        uint16_t _tempVal = recv[5]*256 + recv[6];
        uint16_t _checksum = recv[7]*256 + recv[8];

        //checksum verification
        if(getChecksum()!=_checksum) return;

        //pass verification, can start process and send data
        toSend = SMSMessage;
        toSend.replace("#node#", String(_node));
        toSend.replace("#type#", event[_type]);
        toSend.replace("#val#", String(_smokeVal));
       
        ReceiveAlarm = 1; //indicate need to send SMS   
     }  
  }
  
  if(ReceiveAlarm == 1)
  {
    SendTextMessage();
    ReceiveAlarm = 0; //reset the status flag after send SMS
  }
  
  if (gprsSerial.available()){ // if the shield has something to say
    Serial.write(gprsSerial.read()); // display the output of the shield to the node
  }
}

uint16_t getChecksum()
{
  uint16_t val = 0;
  for(int i = 0;i<7;i++)
  {
    val += recv[i];
  }
  return -val;
}
 
/*
* Name: SendTextMessage
* Description: Send a text message to a number
*/
void SendTextMessage()
{
  // Serial.println("Sending Text...");
  
  gprsSerial.print("AT+CMGF=1\r"); // Set the shield to SMS mode
  delay(100);
  // send sms message, the phone number needs to include the country code e.g. if a U.S. phone number such as (540) 898-5543 then the string must be:
  // +15408985543
  //gprsSerial.println(SMSnumber2); //ober phone
  gprsSerial.println(SMSnumber1); //Julius phone
  delay(100);
  gprsSerial.println(toSend); //the content of the message
  delay(1000);
  gprsSerial.print((char)26);//the ASCII code of the ctrl+z is 26 (required according to the datasheet)
  delay(100);
  gprsSerial.println();  
}
 
/*
* Name: DialVoiceCall()
* Description: Can call/dial a phone number
*/
void DialVoiceCall()
{
  gprsSerial.println("ATD+60124073597;");//dial the number, must include country code
  delay(100);
  gprsSerial.println();
}
