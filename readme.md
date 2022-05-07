# Smart pill box

How many automated things do you want in your life? Almost all I think 😊 What if you always forget to do some routine task like taking pills for example? All reminders don’t work and that fancy pill boxes are too boring? There might be an engineering solution 😊 This article is about how to make it.

## What do we need?

Usually most pill boxes have 7 segments for each day of the week. But it would be cool if someone pulled up the pill for you, reminded you to take it and finally remembered the time you took it. And repeat it daily from Monday until Sunday.
So, we can assemble this device:

![Scheme](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/smart-cup-sketch.drawio.png)

This device will do the following steps:

1.  Every day at 7 AM the servo motor turns its rotor for some angle. This will pull the shelf for some distance, opening the gap.
2.  One pill falls from the shelf into this gap and falls into the "smart cup"
3.  The smart cup has a small hole in the bottom, the laser in the top and the light sensor under the cup. So, after the pill falls into the cup, the laser emits light on the light sensor.
4.  Light sensor measures the illumination. If illumination is not so high, it means the laser beam does not go through the cup hole and the sensor picks up only the room light. This is how our device understands that the pill is inside the cup.
5.  When the pill is inside the cup, one of the 7 LEDs starts blinking red. For Monday it will be the first LED, for Tuesday the second, etc.
6.  Step 4 comprising laser flashing and illumination measuring is repeated with some interval and LED continues blinking.
7.  When you notice the blinking, it’s time for you to take a pill 😊
8.  You take the pill and the laser beam goes through the hole in the cup's bottom and the light sensor gets very powerful illuminance which is many times more than room light. This is how our device understands that the cup is empty.
9.  As soon as the device understands that the pill is taken, it writes the current time into the journal and the corresponding LED stops blinking and lights up green.
10. Our job for today is done. The device switches the laser off and waits for the next morning.
    9A. Alternative scenario - if pill is not taken until 8 PM, device writes an error to the journal, LED stops blinking, turns the red light on, switches laser off and sleeps until next morning.

This device is also connected to your home network via WiFi and hosts some Web UI. If your phone or computer is connected to your home network too, you can open this Web page, check the device state, read the journal for previous days and even control the servo motor, LEDs and laser.
Looks like a good plan. Let’s do it 😊

## Part 1 Hardware

- As I don’t have a 3D printer yet, we will build our device with Lego 😊.
- [Raspberry Pi zero W](https://www.berrybase.de/en/raspberry-pi/raspberry-pi-computer/boards/raspberry-pi-zero-wh) will be device's brain. With Micro SD card, micro-sd to usb adapter, micro-sd to hdmi adapter and [power supply adapter](https://www.berrybase.de/raspberry-pi/raspberry-pi-computer/stromversorgung/netzteile-fuer-die-steckdose/micro-usb-netzteil/ladeadapter-5v/1a-flache-bauform-schwarz).
- Servo motor [SG92R Micro Servo](https://www.berrybase.de/bauelemente/elektromagnetische-bauelemente/motoren-servos/sg92r-micro-servo)
- [5V Laser Diode](https://www.berrybase.de/bauelemente/aktive-bauelemente/dioden-gleichrichter/5v-laserdiode-248-6mm-mit-anschlusskabel)
- Digital Light sensor [BH1750](https://www.berrybase.de/en/sensors-modules/light/bh1750-digitaler-lichtsensor)
- 7 LEDs [(3-pin bi-color)](https://www.berrybase.de/bauelemente/aktive-bauelemente/leds/led-sortimente/5mm-led-set-70-st-252-ck)
- Some resistors, plates, 2N2222A transistor and soldering skills 😊
  ![All parts](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/IMG_4516.png)
  First, we need to [set up our Pi](https://projects.raspberrypi.org/en/projects/raspberry-pi-getting-started). Light sensor will use I2C protocol. Don't forget to [turn it on](https://www.raspberrypi-spy.co.uk/2014/11/enabling-the-i2c-interface-on-the-raspberry-pi/).
  Second, we need to connect everything together using a [40-pin port](https://pinout.xyz/pinout/3v3_power).

1.  Light sensor uses the i2c protocol. So, it will use only 4 pins. Pin VCC - to 3,3 V power; Pin SDA to GPIO2; Pin SDL to GPIO3; Pins Add and Gnd to Ground.
    ![Light sensor scheme](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/Schemes-Light-sensor.png)

2.  Servo motor will be controlled via PWD (Pulse Width Modulation). So, it has only 3 pins. Red pin should be connected to +5V pin, black one - to the ground, and yellow one - to any GPIO pin which we will use programmatically to set up the pulse. In our case we will take GPIO18
    ![Servo motor scheme](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/Schemes-Servo.png)
3.  LEDs. A Normal LED has 2 pins - ground and +3,3V. Using it with Raspberry is simple: ground is connected to the ground, and +3,3V - to any GPIO pin. To control the LED programmatically, you should use the Wiring Pi library or any other. And set 0 to switch LED off and 1 to switch it on. But in our case we want each of 7 LEDs to light green or red. That’s why we will use bi-color LEDs with common anode. This LED has 3 pins. One of them is anode and should be connected to the +3,3V pin. Other 2 pins are cathodes, one of them for green and one for red. LED is switched off if all 3 pins have + 3.3V. If one cathode has 0, LED lights green for example. This scheme describes the difference:
    ![Leds connection](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/Schemes-Leds_Example.png)
    And in our case we will need 7 LEDs:
    ![Leds connection2](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/Schemes-LEDs.png)
    And here we should proof our soldering skills:
    | | |
    |---|----|
    |![Soldered LEDs1](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/solderedLeds1.png) |![Soldered LEDs2](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/solderedLeds2.png)|
4.  Technically the laser is diode and can be controlled the same way as LED. But the problem is that it needs to be connected in a 5V circuit. But all Raspberry GPIO pins which are controlled programmatically can switch on only +3.3V. Hopefully, Raspberry Pi has +5V pin but it always switches on and can not be controlled programmatically. So, we need some switcher which should connect and disconnect the 5V circuit with the laser using +3.3V GPIO pin. The NPN silicon junction transistor. [2N2222A](https://en.m.wikipedia.org/wiki/2N2222) will fit our needs. So, the electronic scheme will look like this:
    ![Laser scheme](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/Schemes-Laser.png)
    The 5V circuit is opened or closed via the collector-emitter junction. When GPIO4 programmatically is set to 1, then +3.3V creates the base-emitter current. At this point the transistor is “opened” which means the laser is switched on. If GPIO4 is set to 0, there is no base-emitter current. Transistor is “closed” which means the laser is off.
    And again, we need to solder this scheme:
    | | |
    |---|----|
    |![Soldered laser1](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/solderedTransistor1.png) |![Soldered laser2](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/solderedTransistor2.png)|

Ok, now we have all parts soldered and connected. It’s time to assemble it:
| | | | |
|---|----|---|----|
|![Lego1](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble1.png) |![Lego2](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble2.png)|![Lego3](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble3.png) |![Lego4](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble4.png)|
|![Lego5](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble5.png) |![Lego6](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble6.png)|![Lego7](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble7.png) |![Lego8](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/assemble8.png)|

## Part 2 Software

We did a great job. Our pills reminder is almost ready to use. Now we have to teach it how to use all those things we connected to Raspberry Pi pins. The Raspberry community already prepared for us some basic things. We will need 2 libraries. PiGPIO we will use to communicate with GPIO pins - set up 0 or +3.3V on each pin. This feature we will use to switch on and off lasers and LEDs. Also this library can send pulses to GPIO pins. This library will be used to operate the servo motor. Another library called WiringPi will help us to communicate with the light sensor via i2c protocol. Both libraries are written in C. And both can be used in C/C++ and Python programs. And these libraries are already included in standard Raspberry Pi OS. As well as gcc compiler and Python runtime. So, we don’t need to do some extra steps to set up our Raspberry Pi. We just can start to write code.
Of course these libraries have been ported to modern frameworks as well. And my first idea was to use .Net core because I’m familiar with it. But our version of Raspberry Pi is very specific. First, it has only 512 MB RAM. And second, Microsoft does not have .net runtime version for this particular CPU architecture. And there is no official Node package for Pi Zero as well. There are only Python and C left. And guess what? It’s time to remember the word “pointer”😊
![Pointer meme](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/pointer-meme.png)

### Architecture

Raspberry Pi OS is the Linux system. And Linux has a built-in message queue for communicating between processes. This will be our key point to build the distributed loosely coupled application 😊
![Architecture](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/Schemes-Architecture.png)
Actually we will create 3 different applications which will be running as separate processes. Each application will be deployed as a service using Linux system daemon and will start after the system boot.

1. Servo service. When service starts, it reads its state from a special csv file, moves the servo motor to the desired angle. And then the process will sleep awaiting a special message from the message queue. When the “move servo” message arrives, the process awakes, moves the servo motor for a new angle, writes angle to the csv file and sleeps again.
2. LEDs service. Works the same way. It has its own csv file to keep the state and has its own queue message type. LEDs service controls all LEDs and the laser according to the message data.
3. Main service. This service waits for the “start” message. When message arrives, it makes this sequence of operations

![Main program algorithm](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/pills-main-program.png)
All 3 services are written in pure C. Code you can find in the src folder.
To make debug easier, I have created the CLI tool which can be run from the command line with some parameters. And regarding the parameters, the tool sends the message to the queue.
To start the main algorithm by timetable I have created a Linux Cron job which calls the CLI tool every morning and the tool sends appropriate message to the queue to start the process.
So, using the built-in message queue we have built the robust micro service architecture 😊

### Web server.

Besides LEDs we need a more relevant way to control the device. Simple web interface is the best solution for this task. And to host a web page we need a web server.
As long as .net and Node can not be run in RPI Zero, we can create the simplest web host using Python. It will start as a service similar to the services described before. And this service can accept Post and Get requests. Since our device can be accessible only inside the local network, we don’t need any authentication mechanisms.
Using POST requests with some query parameters we can control LEDs and servo motor. Using GET requests we can read csv report files. And by parameterless URL, our web server will return ‘index.htm’ So, we can deploy the static web site to our device!
To build a static web site we have plenty of JS frameworks such as Angular or React. But I have used Blazor Web assembly. Source code you can find here[]. And here is how web UI looks like:
![Web UI](https://github.com/DrMboga/RaspberryPillsReminder/blob/main/media/webUi.png)
This UI shows a pill report. Also we can set up the servo angle using a slider and switch off LEDs by clicking the appropriate LED icon.
Ok. Finally we have created a useful IoT device! Thanks for your attention. Hope you liked it😎
