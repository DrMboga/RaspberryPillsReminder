# Publish and compile sequence

- Copy code to the PI

```bash
scp -r ./src/c/*.* pi@192.168.0.67:/home/pi/projects/pills-reminder
scp -r pi@192.168.0.67:/home/pi/projects/pills-reminder/*.* ./src/backup
scp -r ./src/web-host/*.* pi@192.168.0.67:/home/pi/projects/pills-reminder
```

- leds service compile

```bash
gcc leds-service.c pins-mapping.c file-access.c leds.c utils.c messaging.c -o leds-service.out -l pigpio -l pthread
```

- Servo service compile

```bash
gcc servo-service.c file-access.c utils.c messaging.c -o servo-service.out -l wiringPi
```

- Smart cup service compile

```bash
gcc smart-cup.c file-access.c utils.c messaging.c -o smart-cup.out -l wiringPi
```

- Message sender CLI tool compile

```bash
gcc message-sender.c messaging.c -o message-sender.out
```

# Setup services using system daemon

```bash
# Copy service descriptions to the systemd folder
sudo cp leds.service /etc/systemd/system/leds.service
sudo cp pills-webhost.service /etc/systemd/system/pills-webhost.service
sudo cp servo.service /etc/systemd/system/servo.service
sudo cp smart-cup.service /etc/systemd/system/smart-cup.service
# Restart daemon
sudo systemctl daemon-reload
# Start services
sudo systemctl start leds.service
sudo systemctl start pills-webhost.service
sudo systemctl start servo.service
sudo systemctl start smart-cup.service
# Enable auto start
sudo systemctl enable leds.service
sudo systemctl enable pills-webhost.service
sudo systemctl enable servo.service
sudo systemctl enable smart-cup.service

# Some service tune commands
sudo systemctl stop leds.service
sudo systemctl stop pills-webhost.service
sudo systemctl stop servo.service
sudo systemctl stop smart-cup.service
sudo systemctl disable leds.service
# Service logs
sudo journalctl -u leds.service
sudo journalctl -u smart-cup.service
# Services list
sudo systemctl list-units --type=service --all
(q for exit)

# Change execute permissions
chown root message-sender.out
```

# Cron setup

```bash
sudo crontab -e
```

45 6 \* \* \* /home/pi/projects/pills-reminder/message-sender.out start > /home/pi/cron_log.txt 2>&1

```bash
sudo crontab -l
```
