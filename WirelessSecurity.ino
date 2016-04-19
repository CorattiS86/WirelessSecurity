#include <SoftwareSerial.h> 

// the Arduino Uno has only one Serial Interface that we use to communicate with PC, 
// the way to interactwith such interface is with "Serial"

// but it is possible to use SoftwareSerial library that allow serial communication on other digital pins
// using software to replicate the functionality
// for my project I use pin 10 as RX and pin 11 as TX 
SoftwareSerial xBee(10,11);

float analogValue;
float volt;

float GAS_threshold = 300;
float FLAME_threshold = 100;

int counterGas   = 0;
int counterFlame = 0;
float sumGas = 0;
float sumFlame = 0;

byte SenderAddress[8];

int buzzerPin = 13;

// the XBee modules are configured to communicate with BaudRate = 9600
// then it's important to initially set such value with "xBee.begin(9600);"
void setup() {
  
  Serial.begin(9600);
  Serial.println("Connected !!!");
  
  xBee.begin(9600);
  pinMode(buzzerPin, OUTPUT);  
}

void loop() {
 
    
    //Arduino makes sure that what it receive could be an API packet
    //in according to API protocol the minimum number of bytes for packet is 19 bytes...
    if(xBee.available() > 18)   
    {
        //...and the first byte, the byte [0] is always equal to = 0x7E
        if( xBee.read() == 0x7E )
        {
          // the next two bytes are the lenght of the packet, starting from byte [3]
          xBee.read();
          xBee.read();
          
          //the byte [3] is always 0x92 
          if(xBee.read() == 0x92)
          {
            // now Arduino needs to read the Sender Address, which is 64 bit, then it read the next 8 bytes 
            // and store the Address in an array, successively it will check if the sender is one of expected sender 
            SenderAddress[0] = xBee.read();
            SenderAddress[1] = xBee.read();
            SenderAddress[2] = xBee.read();
            SenderAddress[3] = xBee.read();

            SenderAddress[4] = xBee.read();
            SenderAddress[5] = xBee.read();
            SenderAddress[6] = xBee.read();
            SenderAddress[7] = xBee.read();
            
            Serial.print("ADDRESS: "); Serial.print(SenderAddress[0], HEX); Serial.print(SenderAddress[1], HEX); Serial.print(SenderAddress[2], HEX); Serial.print(SenderAddress[3], HEX);
            Serial.print(" ------- "); Serial.print(SenderAddress[4], HEX); Serial.print(SenderAddress[5], HEX); Serial.print(SenderAddress[6], HEX); Serial.println(SenderAddress[7], HEX);

            // two bytes specify network address
            xBee.read();
            xBee.read();

            // this byte tells you if the message was sent in broadcast 
            xBee.read();

            // number of samples, it's always 1
            xBee.read();

            // the next bytes represent Bit Mask for Digital and Analog pins that have been setup as INPUT on Sender XBee,
            // this can be useful when more sensors are connected to one XBee and we want to distinguish the received values,
            // but for my project i have only one sensor for XBee module, and are all connected to pin 20
            // that XBee sees as the first pin (D0) when reads the values
            
            // two bytes for the Bit Mask of Digital Input
            xBee.read();
            xBee.read();

            // one byte for the Bit Mask of Analog Input
            xBee.read();

            // it reads the next 2 bytes that can be the Digital or Analog sampled data
            // in both cases information is stored with 2 bytes
            byte firstDataByte  = xBee.read();
            byte secondDataByte = xBee.read();

            // the last byte is of the checksum    
            xBee.read(); 
                       
            // at this point it see what sensor correspond the address
            if(( SenderAddress[0] == 0x00) && ( SenderAddress[1] == 0x13 ) && ( SenderAddress[2] == 0xA2 ) && ( SenderAddress[3] == 0x00 ))  
            {
              if(( SenderAddress[4] == 0x40) && ( SenderAddress[5] == 0xA8 ) && ( SenderAddress[6] == 0x9C ) && ( SenderAddress[7] == 0x85 )) 
                       // address of Flame sensor
                       check_Fire( firstDataByte, secondDataByte );     
              else
              if(( SenderAddress[4] == 0x40) && ( SenderAddress[5] == 0xA8 ) && ( SenderAddress[6] == 0x9C ) && ( SenderAddress[7] == 0x6B )) 
                       // address of Gas sensor
                       check_Gas_Leaks( firstDataByte, secondDataByte );     
              else
              if(( SenderAddress[4] == 0x40) && ( SenderAddress[5] == 0xA8 ) && ( SenderAddress[6] == 0x9C ) && ( SenderAddress[7] == 0xA3 )) 
                       // address of Motion sensor
                       check_Motion( firstDataByte, secondDataByte );              
            }
         
          }
        }
    }    
}

void check_Gas_Leaks( byte highValue, byte lowValue )
{
  // for each Analog Data are transmitted with 2 byte, 
  // because the Input voltage is sampled with 10 bit
 
  // after the reading it computes the aggregate value
  analogValue = highValue * 0xFF + lowValue;

  // it computes the equivalent of the voltage present on the pin
  volt = (1.2 * analogValue)/1023; 

  // display vaules received to the PC
  Serial.print("GAS VALUE: "); Serial.print(analogValue); Serial.print("     VOLT: "); Serial.print(volt); Serial.println("V");

  // it checks if the value is above the threshold value
  // and if this happens then triggers the alarm
  if( counterGas < 60)
  {
      counterGas++;
      sumGas += analogValue;
      if (counterGas == 60)
      {
        GAS_threshold = (sumGas/60)+((sumGas/60)*(20/100));
        Serial.print("GAS_THRESOLD:"); Serial.println(GAS_threshold);
      }
  }
  else
    if( analogValue > GAS_threshold )
      alarm(); 
}

void check_Fire( byte highValue, byte lowValue )
{
  // for each Analog Data are transmitted with 2 byte, 
  // because the Input voltage is sampled with 10 bit
 
  // after the reading it computes the aggregate value
  analogValue = highValue * 0xFF + lowValue;

  // it computes the equivalent of the voltage present on the pin
  volt = (1.2 * analogValue)/1023; 

  // display vaules received to the PC
  Serial.print("FLAME VALUE: "); Serial.print(analogValue); Serial.print("     VOLT: "); Serial.print(volt); Serial.println("V");

  // it checks if the value is above the threshold value
  // and if this happens then triggers the alarm
   if( counterFlame < 60)
  {
      counterFlame++;
      sumFlame += analogValue;
      if (counterFlame == 60)
      {
        FLAME_threshold = (sumFlame/60)+((sumFlame/60)*(20/100));
        Serial.print("FLAME_THRESOLD:"); Serial.println(FLAME_threshold);
      }
  }
  else
    if( analogValue > FLAME_threshold )
      alarm(); 
} 
 

void check_Motion( byte highValue, byte lowValue )
{
  // for motion detection we have a digital value then it can be 0 or 1
  // if 1 the triggers the alarm
  Serial.print("MOTION VALUE: "); Serial.println(lowValue);
  if (lowValue == 1)
    alarm();
}

void alarm()
{
  int k=0;
  for(k=0; k<10;k++)
  {
    int i;
    for(i=0; i<50; i++)
    {
      digitalWrite( buzzerPin, HIGH);
      delay( 1);  
      digitalWrite( buzzerPin, LOW);
      delay( 1);
    }
    for(i=0; i<50; i++)
    {
      digitalWrite( buzzerPin, HIGH);
      delay( 2);  
      digitalWrite( buzzerPin, LOW);
      delay( 2);
    }
  }
}
