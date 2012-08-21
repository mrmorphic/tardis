/**
 * Tardis
 * 
 * + Hardware:
 * + Components
 * - SD Card
 * - sound output
 * - LED output
 * - bluetooth input
 * + Pin assignments:
 * - Pin 11 SPI MOSI   - SD card
 * - Pin 12 SPI MISO   - SD card
 * - Pin 13 SPI CLK    - SD card
 * - Pin 8 SPI CS      - SD card
 * - Pin 10            - SD card (not connected but reserved)
 * - Pin 0 RxD         - bluetooth adapter
 * - Pin 1 TxD         - bluetooth adapter
 * - Pin 2             - audio serial data output 74HC595 DS
 * - Pin 3             - audio shift data 74HC595 (shift data in)
 * - Pin 4             - audio store data 74HC595
 * - Pin 5 PWM         - LED inner ring
 * - Pin 6 PWM         - LED mid ring
 * - Pin 9 PWM         - LED outer ring
 */

/**
 * These are the functions implemented
 *   Code    Sound         Lighting effect
 *   ----    -----         ---------------
 *   A       tardis.wav    pulsing light
 *   B       exterm.wav    with flickering light
 *   C       biodamp.wav
 *   D       bingle.wav
 *   E       rainbow.wav   pulsing light
 *   F       stuff.wav
 *   G       theme.wav     pulsing light
 *   H       nyancat.wav   ring cycle
 *   I                     light on full
 *   J                     light off
 *   K                     light increase
 *   L                     light decrease
 */
#include <SD.h>

/* pin map */
const int pin_sdcard_chip_select = 8;
const int pin_sdcard_mosi = 11;
const int pin_sdcard_miso = 12;
const int pin_sdcard_clk = 13;
const int pin_sdcard_chip_select_alt= 10;

const int pin_serial_rxd = 0;
const int pin_serial_txd = 1;

const int pin_led_inner = 5;
const int pin_led_mid = 6;
const int pin_led_outer = 9;

const int pin_sound_data = 2;
const int pin_sound_clock = 3;
const int pin_sound_store = 4;

// prototypes
void sound_setup();
void sound_play_file(char *);
void sound_play_file_ping(long);
void light_setup();
void light_ping(long);
void bluetooth_setup();
void bluetooth_ping(long);
void light_set_mode(int, long);
int light_get_mode();

const int light_mode_none = 0;
const int light_mode_flashing = 1;
const int light_mode_flickering = 2;
const int light_mode_rings = 3;
const int light_mode_reading = 4;

void setup() {
  light_setup();
  sound_setup();
  bluetooth_setup();

  light_set_mode(light_mode_none, 0L);
  sound_play_file("startup.wav", 1, -1);
}


long time;

void loop() {
  time = micros();

  bluetooth_ping(time);

  light_ping(time);
  sound_play_file_ping(time);
}


/// Bluetooth

void bluetooth_setup() {
  Serial.begin(9600);
}

void bluetooth_ping(long time) {
  int prior_mode = light_get_mode();

  if (Serial.available() >= 1) {
    int ch = Serial.read();
    switch (ch) {
    case 'A':
      light_set_mode(light_mode_flashing, 1300L);
      sound_play_file("tardis.wav", 1, prior_mode);
      break;
    case 'B':
      light_set_mode(light_mode_flickering, 0L);
      sound_play_file("exterm.wav", 2, prior_mode);
      break;
    case 'C':
      light_set_mode(light_mode_none, 0L);
      sound_play_file("biodamp.wav", 1, -1);
      break;
    case 'D':
      light_set_mode(light_mode_none, 0L);
      sound_play_file("bingle.wav", 1, -1);
      break;
    case 'E':
      light_set_mode(light_mode_flashing, 2400L);
      sound_play_file("rainbow.wav", 1, prior_mode);
      break;
    case 'F':
      light_set_mode(light_mode_none, 0L);
      sound_play_file("stuff.wav", 1, prior_mode);
      break;
    case 'G':
      light_set_mode(light_mode_flashing, 900L);
      sound_play_file("theme.wav", 1, prior_mode);
      break;
    case 'H':
      light_set_mode(light_mode_rings, 120000L);
      sound_play_file("nyancat.wav", 1000, prior_mode);
      break;
    case 'I':
      light_set_mode(light_mode_reading, 0L);
      break;
    case 'J':
      light_set_mode(light_mode_none, 0L);
      break;
    case 'K':
      light_increase();
      break;
    case 'L':
      light_decrease();
      break;
    }
  }
}

/**
 * These are the functions implemented
 *   Code    Sound         Lighting effect
 *   A       tardis.wav    pulsing light
 *   B       exterm.wav    with flickering light
 *   C       biodamp.wav
 *   D       bingle.wav
 *   E       rainbow.wav   pulsing light
 *   F       stuff.wav
 *   G       theme.wav     pulsing light
 *   H       nyancat.wav   ring cycle
 *   I                     light on full
 *   J                     light off
 *   K                     light increase
 *   L                     light decrease
 */


/// LED lights


void light_setup() {
  pinMode(pin_led_inner, OUTPUT);
  pinMode(pin_led_mid, OUTPUT);
  pinMode(pin_led_outer, OUTPUT);
}


// ring is pin number. brightness is 0-255
void light_set_ring_brightness (int ring, int brightness) {
  analogWrite (ring, brightness);
}

long light_flash_time = 0L;
long light_flash_period;
int light_brightness = 0;
int light_flash_direction;
int light_mode = light_mode_none;
long light_flicker_next_time = 0L;
long light_ring_time = 0L;
long light_ring_period;
int light_ring_selected;
const int light_flicker_min_time = 1000;
const int light_flicker_max_time = 3000;
long light_reading_time = 0L;
const int light_reading_period = 5000;    // polling time for reading mode, so we're not setting pwm too frequently

int light_get_mode() {
  return light_mode;
}

void light_set_mode (int mode, long param) {
  light_mode = mode;
  switch (mode) {
  case light_mode_none:
    light_set_ring_brightness(pin_led_outer, 0);
    light_set_ring_brightness(pin_led_mid, 0);
    light_set_ring_brightness(pin_led_inner, 0);
    break;

  case light_mode_flashing:
    light_flash_period = param;
    light_brightness = 0;
    light_flash_direction = 1;
    break;

  case light_mode_flickering:
    light_flicker_next_time = micros() + random(light_flicker_min_time, light_flicker_max_time);
    break;

  case light_mode_rings:
    light_ring_time = millis();
    light_ring_period = param;
    light_ring_selected = 0;
    break;

  case light_mode_reading:
    light_brightness = 255;
    light_reading_time = micros();
    break;

  }
}

void light_increase() {
  if (light_brightness < 235) light_brightness += 25;
}

void light_decrease() {
  if (light_brightness > 25) light_brightness -= 25;
}

void light_ping(long time)
{
  int br;

  switch (light_mode) {
  case light_mode_none:
    // don't need to do anything
    break;

  case light_mode_flashing:
    if (light_flash_period == 0L) return;
    if (time > light_flash_time + light_flash_period) {
      light_set_ring_brightness(pin_led_outer, light_brightness);
      light_set_ring_brightness(pin_led_mid, light_brightness);
      light_set_ring_brightness(pin_led_inner, light_brightness);
      if (light_flash_direction > 0) {
        if (light_brightness >= 255) light_flash_direction = -1;
      }
      else {
        if (light_brightness <= 0) light_flash_direction = 1;
      }
      light_brightness += light_flash_direction;
      light_flash_time = time;
    }
    break;

  case light_mode_flickering:
    if (time < light_flicker_next_time) return;
    br = random(0,2) ? 255 : 0;
    light_set_ring_brightness(pin_led_outer, br);
    light_set_ring_brightness(pin_led_mid, br);
    light_set_ring_brightness(pin_led_inner, br);
    light_flicker_next_time = micros() + random(light_flicker_min_time, light_flicker_max_time);
    break;

  case light_mode_rings:
    if (time > light_ring_time + light_ring_period) {
      light_ring_selected++;
      if (light_ring_selected > 3) light_ring_selected = 1;

      switch (light_ring_selected) {
        case 1:
          light_set_ring_brightness(pin_led_outer, 255);
          light_set_ring_brightness(pin_led_mid, 0);
          light_set_ring_brightness(pin_led_inner, 0);
          break;
        case 2:
          light_set_ring_brightness(pin_led_outer, 0);
          light_set_ring_brightness(pin_led_mid, 255);
          light_set_ring_brightness(pin_led_inner, 0);
          break;
        case 3:
          light_set_ring_brightness(pin_led_outer, 0);
          light_set_ring_brightness(pin_led_mid, 0);
          light_set_ring_brightness(pin_led_inner, 255);
          break;
      }
      light_ring_time = time;
    }
    break;

  case light_mode_reading:
    if (time > light_reading_time + light_reading_period) {
      light_set_ring_brightness(pin_led_outer, light_brightness);
      light_set_ring_brightness(pin_led_mid, light_brightness);
      light_set_ring_brightness(pin_led_inner, light_brightness);
      light_reading_time = time;
    }
    break;
  }
}


/// Sound

int sound_playing;
File sound_file;
int sound_play_count;
int sound_final_light_mode;
int sound_played_times;

long play_sound_time = 0L;  // last ping time
long play_sound_period = 113L;  // number of microseconds between byte sends to audio, 8000 samples per second

int sound_sdcard_ok;

void sound_setup() {
  pinMode(pin_sound_data, OUTPUT);
  pinMode(pin_sound_clock, OUTPUT);
  pinMode(pin_sound_store, OUTPUT);

  // make sure that the default chip select pin is set to
  // output, even if though we don't use it. This is a bit
  // retarded but is a limitation of the SD card library.
  pinMode(pin_sdcard_chip_select_alt, OUTPUT);

  // Initialise SD card
  sound_sdcard_ok = true;
  if (!SD.begin(pin_sdcard_chip_select)) {
    sound_sdcard_ok = false;
  }
}

/* Given a file, read that file and send the bytes through to
 * the sound output using PWM. This actually initialises
 * the reading.
 * @param filename          name of file
 * @param count             number of times to play the sound
 * @param final_light_mode  the light mode to set when the sound sequence has finished playing. If -1, no
 *                          change in light mode.
 */
void sound_play_file(char *filename, int count, int final_light_mode) {
  sound_playing = false;
  if (!sound_sdcard_ok) return;

  sound_file = SD.open(filename);
  if (!sound_file) {
    Serial.println("Could not open file");
    return;
  }
  //  sound_byte_offset = 0L;
  sound_playing = true;
  sound_play_count = count;
  sound_played_times = 0;
  sound_final_light_mode = final_light_mode;
}

#define CLEAR_PIN(port,pin) (port&=(~(1<<pin)))
#define SET_PIN(port,pin) (port|=(1<<pin))

// Write a byte out to the 74HC595. We could use shiftOut, but it is not fast enough, only giving
// a maximum sample rate of around 4ksps. This routine, although it only works on port D of the CPU,
// allows a maximum rate of about 16ksps. The sound samples are 8ksps, so this is fast enough to give
// enough spare CPU capacity for other functions.
// pin 2 == data  == port D, pin 2
// pin 3 == clk   == port D, pin 3
// pin 4 == store == port D, pin 4
void sound_write_byte(int data) {
  CLEAR_PIN(PORTD, 4);
  CLEAR_PIN(PORTD, 3);

  for (int i=0; i<8; i++) {
    if (data & 0x80) {
      SET_PIN(PORTD,2);
    }
    else {
      CLEAR_PIN(PORTD,2);
    }
    data = data << 1;
    SET_PIN(PORTD,3);
    CLEAR_PIN(PORTD,3);
  }
  SET_PIN(PORTD,4);
}

void sound_play_file_ping(long time) {
  if (!sound_playing) return;

  if (time < (play_sound_time + play_sound_period)) return;

  play_sound_time = time;

  if (!sound_file.available()) {
    sound_played_times++;
    if (sound_played_times == sound_play_count) {
      // no more data in file, so stop.
      sound_playing = false;
      sound_write_byte(0);
      if (sound_final_light_mode >= 0) light_set_mode(sound_final_light_mode, 0L);

      return;
    }
    sound_file.seek(0L);
  }

  byte data = sound_file.read();

  sound_write_byte(data);
}



