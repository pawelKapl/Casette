# Casette - DIY guitar processor based on RaspberryPi and NAM captures

## Main highlights:
- Running on raspberry pi 4
- Touchscreen
- Usb powered
- DietPi RPi OS
- based on NAM captures: https://www.neuralampmodeler.com/ and lightweight NeuralAudio library for processing those: https://github.com/mikeoliphant/NeuralAudio
- IR loader
- Time based effects
- Noise gate
- Presets handling
- Tuner

## Features
- Neural amp modeler amp simulation unit (A1 & A2 standard) with full tonestack
- Running two NAM instances (amps) at once with parallel/series adjustment and blend control. 
- IR loader for cab simulation
- Noise Gate
- Delays: Mono, Tape, Dual Tap, Ping Pong, Circular, reverse (coming)
- Reverbs: room
- Modulator - with vibrato mode
- Cpu usage monitor
- Tuner
- Bypass
- Presets & memory

## BOM:
- Raspberry PI 4 or higher
- HiFiBerry DAC2 ADC Pro sound card
- Waveshare LCD 4,3'' IPS 800x480px DSI touchscreen
- Hammond case
- Jacks, switches, cables, etc.

## Compilation:
- i am using cross compilation - compiling code on RPi is a nightmare, this means that on Windows i am compiling unix code, using all the libraries pulled from DietPi on Raspberry
- u need to additionally create deps root folder with these libs:

<img width="294" height="250" alt="image" src="https://github.com/user-attachments/assets/6b140877-7627-45be-945d-1591815af4e6" />

## Raspberry config:
- u need some lightweight OS distro - i used DietPi
- enable i2c, spi and uart
- u need to create JACK Audio Server on your RPi (jackd), and configure your sound card 
- for UI i used LVGL framework with framebuffers
- services to run on startup:

JACKD server (jackd.server):
```
[Unit]
Description=JACK Audio Server
Requires=sound.target
After=sound.target

[Service]
ExecStart=/usr/bin/jackd -P10 -d alsa -d hw:0 -r 48000 -p 64 -n 2 -i 2 -o 2
Restart=on-failure
Environment=JACK_NO_AUDIO_RESERVATION=1

[Install]
WantedBy=multi-user.target
```
Casette (casette.service):
```
[Unit]
Description=Casette
Requires=jackd.service
Conflicts=getty@tty1.service

[Service]
TTYPath=/dev/tty1
StandardOutput=journal
StandardError=journal
ExecStart=/root/Casette/build/Casette
WorkingDirectory=/root/Casette/build
Restart=on-failure

[Install]
WantedBy=multi-user.target
```

- cmdline.txt should be:
```
root=PARTUUID=<keep yours here> rootfstype=ext4 rootwait net.ifnames=0 logo.nologo console=null consoleblank=0 quiet splash vt.global_cursor_default=0
```
- config.txt should additionaly contain:
```
dtoverlay=piscreen,drm
dtoverlay=hifiberry-dacplusadcpro
```

<img width="1080" height="1440" alt="image" src="https://github.com/user-attachments/assets/9690719d-2d70-4bbf-8748-a4a78e6fa292" />

<img width="1080" height="1440" alt="image" src="https://github.com/user-attachments/assets/3e6c7ed3-0ccd-4241-b8e1-1a00b7ede5f6" />

### Test video: 
[![Watch the video](https://i.ytimg.com/vi/pGTkpytEGvQ/oar2.jpg?sqp=-oaymwEoCJUDENAFSFqQAgHyq4qpAxcIARUAAIhC2AEB4gEKCBgQAhgGOAFAAQ==&rs=AOn4CLDyASWbmikuPllF9DrZUUQPOz3kVA&usqp=CCk)](https://www.youtube.com/shorts/pGTkpytEGvQ)

### Test Audio:
[![Watch the video](https://i.ytimg.com/vi/c09Gy6-Nhic/hqdefault.jpg?sqp=-oaymwFBCNACELwBSFryq4qpAzMIARUAAIhCGAHYAQHiAQoIGBACGAY4AUAB8AEB-AH-CYAC0AWKAgwIABABGGUgZShlMA8=&rs=AOn4CLDWLlUPZHn1p3z6lXIs_XmGChFysQ)](https://youtu.be/c09Gy6-Nhic)

