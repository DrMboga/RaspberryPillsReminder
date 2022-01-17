# Publish and compile sequence

 - Copy code to the PI
 ```bash
scp -r ./src/c/*.* pi@192.168.0.67:/home/pi/projects/pills-reminder
 ```

 - leds service
 ```bash
gcc leds-service.c pins-mapping.c file-access.c leds.c utils.c messaging.c -o leds-service.out -l pigpio -l pthread
 ```
 - test message sender
 ```bash
gcc message-sender.c messaging.c -o message-sender.out
 ```

# System daemon
```bash
# Copy service description to the systemd folder
sudo cp leds.service /etc/systemd/system/leds.service
# Restart daemon
sudo systemctl daemon-reload
# Start service
sudo systemctl start leds.service
# Enable auto start
sudo systemctl enable leds.service


sudo systemctl stop leds.service
sudo systemctl disable leds.service
# Service logs
sudo journalctl -u leds.service
# Services list
sudo systemctl list-units --type=service --all

# Change execute permissions
chown root message-sender.out
```


 # Python set up

```bash
# sudo pip install posix_ipc
# sudo apt-get install -y python3-posix-ipc
```

# Cron setup

```bash
crontab -e
```
@reboot sudo /home/pi/projects/pills-reminder/leds-service.out

(Don't work)

scp -r ../ipc_test/*.* pi@192.168.0.67:/home/pi/projects/ipc-test