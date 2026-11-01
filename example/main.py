from machine import ADC, Pin
import time

led = Pin("LED", Pin.OUT)
sensor = ADC(4)

while True:
    led.toggle()
    volts = sensor.read_u16() * 3.3 / 65535
    temp = 27 - (volts - 0.706) / 0.001721
    print("temp: {:.1f} C".format(temp))
    time.sleep(1)
