# Arduino-Ring-Buffer

A small library designed to implement an effective ring buffer on microcontroller boards.

This project is a work-in-progress and will be updated as new features become required, or I learn to work with other
architectures besides AVR. As of release, I'm only able to test this library on the 8-bit AVR microcontrollers. Though
it should be able to work on all AVR boards, it's currently only optimized for 8-bit boards. 

I was considering adding an additional level of control to this project by allowing a sort of configuration through
defines and other preprocessor directives. Either through adding a separate configuration file, which I would rather 
not use, or by adding a large section of preprocessor directives just before the actual code of the RingBuffer.h. I
find this a better option if something like this configuration is necessary. 

Some side notes:
   This is largely a learning project for myself. Trying to learn about how this data structure works, as well as ways
   to learn how to optimize code. As such, the code likely will recieve a lot of changes as I receive input from the 
   community, which I'm looking forward to!