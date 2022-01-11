# Publish and compile sequence

 - Copy code to the PI
 ```bash
scp -r ./src/c pi@192.168.0.67:/home/pi/projects/pills-reminder
 ```

 - leds service
 ```bash
gcc leds-service.c pins-mapping.c file-access.c leds.c utils.c -o leds-service -l pigpio
 ```