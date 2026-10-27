Nikita Michailov INF 

## Summary
This is a light-level monitor with an adjustable alarm. 
The Arduino reads ambient light level from the photoresistor every 100 ms and 
shows it as a 5-LED bar graph. It sounds the buzzer when the light goes 
above a threshold, and you can set that threshold with the button. 
It also records the darkest and brightest readings it has seen and keeps them in
the EEPROM. 

## EEPROM layout

+ 0 - Magic byte 0x42
+ 1-2 Alarm threshold
+ 3-4 Minimum light seen
+ 5-6 Maximum light seen

## Timer configuration

TimerOne library is used to implement a timer interrupt with 100 ms. The timer
interupt is used to notify when a new sample from the photoresistor is ready

## Interrupt configuration

External interrupt on pin 2, triggered on the falling edge. Used to notify
when the button is pressed.
