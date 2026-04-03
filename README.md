## Casette - DIY guitar processor based on RaspberryPi and NAM captures

# Main highlights:
- Running on raspberry pi 4
- Touchscreen
- Usb powered
- DietPi RPi OS
- based on NAM captures: https://www.neuralampmodeler.com/
- IR loader
- Time based effects
- Noise gate
- Presets handling
- Tuner

BOM:
- raspberry PI 4 or higher
- HiFiBerry DAC2 ADC Pro sound card
- Waveshare LCD 4,3'' IPS 800x480px DSI touchscreen
- Hammond case
- Jacks, switches, cables, etc.

Compilation:
- i am using cross compilation - compiling code on RPi is a nightmare, this means that on Windows i am compiling unix code, using all the libraries pulled from DietPi on Raspberry
- u need to additionally create deps root folder with these libs:
<img width="294" height="250" alt="image" src="https://github.com/user-attachments/assets/6b140877-7627-45be-945d-1591815af4e6" />

Raspberry config:
- u need some lightweight OS distro - i used DietPi
- u need to create JACK Audio Server on your RPi (jackd), and configure your sound card 
- for UI i used LVGL framework with framebuffers
- generally there is a lot of config to make it work nice, boot fast, i will keep adding those hints here...

<img width="1080" height="1440" alt="image" src="https://github.com/user-attachments/assets/9690719d-2d70-4bbf-8748-a4a78e6fa292" />
<img width="1080" height="1440" alt="image" src="https://github.com/user-attachments/assets/3e6c7ed3-0ccd-4241-b8e1-1a00b7ede5f6" />

