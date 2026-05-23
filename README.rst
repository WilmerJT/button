.. zephyr:code-sample:: led-pwm
   :name: LED PWM
   :relevant-api: led_interface

   Control PWM LEDs using the LED API.

Overview
********

This sample allows to test the led-pwm driver. The first "pwm-leds" compatible
device instance found in DT is used. For each LEDs attached to this device
(child nodes) the same test pattern (described below) is executed. The LED API
functions are used to control the LEDs.

Test pattern
============

For each PWM LEDs (one after the other):

- Turning on
- Turning off
- Increasing brightness gradually
- Blinking on: 0.1 sec, off: 0.1 sec
- Blinking on: 1 sec, off: 1 sec
- Turning off

Building and Running
********************

This sample can be built and executed on all the boards with PWM LEDs connected.
The LEDs must be correctly described in the DTS: the compatible property of the
device node must match "pwm-leds". And for each LED, a child node must be
defined and the PWM configuration must be provided through a "pwms" phandle's
node.


fix_audio.py

PS C:\Users\COMPUMAX\Documents\led_pwm.worktrees\agents-mp3-pwm-sending-lm386n> python fix_audio.py
Leyendo WAV...
Canales: 1
Sample width: 2
Framerate: 22050 Hz
Frames: 21494
Muestras mono: 21494
Rango original: -24315 a 17137
Sin resample necesario
Rango 8-bit: 0 a 255
Duración final: 0.975 s