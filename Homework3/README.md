Nikita Michailov INF

## Summary

This is an Arduino barrier that opens automatically when an object approaches
closer than 8 cm to it. An ultrasonic sensor (HC-SR04) measures the distance
every 500 ms and the current distance is shown on a 16x2 LCD. When an object
is within 8 cm, a servo raises the barrier. It closes again once
the object moves further than 18 cm away.

The barrier counts how many times it has been opened and keeps that number in
the EEPROM.

## Pins

+ 2-7 - LCD (RS, E, D4-D7)
+ 8 - Ultrasonic sensor ECHO
+ 9 - Servo
+ 10 - Ultrasonic sensor TRIG

## Direct port access

The ultrasonic sensor pins (8 and 10) are on port B, so they are controlled
directly through the IO registers instead of `pinMode`/`digitalWrite`/
`digitalRead`:

+ `DDRB` - sets the pin direction (input/output)
+ `PORTB` - writes the TRIG pin
+ `PINB` - reads the ECHO pin

## EEPROM layout

+ 0 - Magic byte 0xA5
+ 1-4 - Open counter (unsigned long)

## Serial commands

Baud rate 9600. Commands end with a newline.

+ `count` - show how many times the barrier has been opened
+ `reset` - reset the counter to 0
+ `dist` - show the current distance
+ `help` - list available commands
