# WirelessSecurity

This project is for exam of the course "Pervasive System 2016" - Master of Engineering in Computer Science "La Sapienza"

Other informations on "http://www.slideshare.net/StefanoCoratti/wireless-security-61546049" 

It consists in connecting 4 XBee modules, so configured:
- one XBee direct connected to Gas sensor MQ5
- one XBee direct connected to Flame sensor 
- one XBee direct connected to Passive Motion sensor
- one XBee connected to an Arduino Uno board where WirelessSecurity.ino is loaded

The Arduino works as an Alarm Control Unit, receive data,through XBee modules, wirelessly from sensors
  check if such values are within a threshold value that it means no motion, gas or flames are detected,
  otherwise alarm event is detected and it triggers the Buzzer to communicate such event.
  
I used these three sensors because it's posible to do a simple simulation, 
just a cigarette lighter to produce gas and flame to be detected, and it's sufficent a movement of our hands 
to trigger the motion sensor, but the concept is valid for any sensor.

Stefano
