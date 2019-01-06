# Optical Tachometer

This code can be used to measure rotational speed ([RPM](https://en.wikipedia.org/wiki/Revolutions_per_minute)) of a shaft using an optical sensor.  This was originally based on the [Measure RPM - Optical Tachometer](https://www.instructables.com/id/Measure-RPM-DIY-Portable-Digital-Tachometer/) instructatble.  The actual local build uses a [QRD1114](https://cdn.solarbotics.com/products/datasheets/qrd1114.pdf) Reflective Object Sensor, instead of the separate IR LED and photodiode.  The same functionality, but resistor values have been tweaked in our circuit.

The code from that instructuable has been majorly refactored, and mostly rewritten using the base concepts.

The initial (and current) code used Arduino pins 2, 3 and 4 to connect the sensor.  Those were chosen to allow a simple header to be plugged into an Arduino board, with the requirements being to connect to an interrupt pin, and supply power to the led and photo diode.  The senor only needs to use a single data pin (pin 2 for the interrupt), plus 5 volts and ground.  Pins 3 and 4 are set to LOW and HIGH, to supply ground and 5V respectively.  The digital pins are capable of supplying (and sinking) enough power for that.

TODO:
frtizing diagram and link
