# Tardis Bookshelf Code

This is code for an Arduino project (made in Arduino IDE v1.0)

# Hardware

The Arduino board is embedded in a bookshelf made to look like a Tardis. The supporting hardware is:

 * a Sparkfun SD card board with prototyping area
 * a 74HC595 SIPO register for sound output. 8 bit data is fed into 8-bit DAC (2R-R ladder), and then
   to a small amp driving a loudspeaker
 * a cluster of white LEDs arranged in three rings, driven by 3 PWM pins
 * a bluetooth serial adaptor board

# Software

The tardis is controlled from an Android phone via bluetooth serial commands. There are a variety of
commands that perform pre-defined actions which include control of the LED cluster and playing sounds.

Sounds are stored in WAV files on the SDCARD in 8ksps mono format. Software reads these values and sends
them to the amp via the 74HC595. This includes some "custom" code to work around certain Arduino library calls
that are not fast enough for high rates (e.g. digitalWrite and serialOut)

Sound samples on the SD-card include:

 * dr who theme tune
 * tardis sound
 * dalek exterminate
 * a couple of 10th doctor quotes
 * double rainbow clip
 * nyan cat music

