# Fan-Control-Deep-Sleep
A simple arduino project to read the rpm of a fan and sending it over the net via mqtt.

The current consumption during the deep sleep is very low (lower than 0.1mA).
When sending data 70mA are absorbed for up to a second, this is very useful for a small and remote unit.
Intially designed to work with a d1 mini board and then converted to work with an esp32.
