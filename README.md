# FluidNC-Pendant
Work originated from https://github.com/gjkrediet/Fluid-controller

<img src="https://github.com/AC8L/FluidNC-Pendant/blob/main/Photos/Front.jpeg" width=50% height=50%>

The original design by gjkrediet was intended for bluetooth, this design is for Wi-Fi version of the FluidNC. Pendant communicates with the controller through Websocket protocol on TCP port 81.

# PCB
PCB was redesigned to hold all components to be soldered into it except the battery and power switch.
GRBL files are included for online order as well as schematics.

## Front of the PCB with soldered components
<img src="https://github.com/AC8L/FluidNC-Pendant/blob/main/Photos/PCB_Mounted.jpeg" width=50% height=50%>

## Back of the PCB
<img src="https://github.com/AC8L/FluidNC-Pendant/blob/main/Photos/PCB_Back.jpeg" width=50% height=50%>

## JST 2.0 soldered to the power switch and JST that comes with TTGO
<img src="https://github.com/AC8L/FluidNC-Pendant/blob/main/Photos/JST_and_Switch_On_Lid.jpeg" width=50% height=50%>

## Battery is fixed behind PCB with a Kapton tape
<img src="https://github.com/AC8L/FluidNC-Pendant/blob/main/Photos/LiPo_Kapton.jpeg" width=50% height=50%>

# 3D printed case
Lid was slightly modified to hold new PCB and power switch. Depending on which tactile switches were used - 3D printed buttons can be vertically scaled/reduced in the slicer.

# Firmware
Development was migrated from Arduino IDE into the PlatformIO to have stricter control over library versioning.
## Installation instructions:
1. Clone git repo: git clone https://github.com/AC8L/FluidNC-Pendant.git
2. Copy/rename sample_Config.h to Config.h
3. Enter your WiFi SSID information and FluidNC hostname/port in Config.h
4. Wait until Visual Studio Code pulls down all depending libraries
5. Overwrite content of .pio/libdeps/esp32dev/TFT_eSPI/User_Setup.h with the content of User_Setup.h.TFT_eSPI
6. Overwrite the content of .pio/libdeps/esp32dev/TFT_eSPI/User_Setup_Select.h with the content of User_Setup_select.h.TFT_eSPI
7. Now firmware will compile and you should be able to program the TTGO T Display.

# Operations
This section should probably be the largest section of this document but let at least get started.

## Red button
- When FluidNC is in ALARM mode, pressing RED button sends UNLOCK command
- Long pressing the RED button puts pendant into the sleep mode
- When in sleep mode, pressing RED button wakes pendant up

## Optical rotator
- By default it is in the mode of adjusting the jog speed
- When pressed - it enters the Menu mode. Menu options can be selected on the screen by scrolling the rotator and finally choosing by a click
- Homing can be done from Menu and changing brightness, turning spindle on and off among other functions

## Joystick
Jogs X and Y axes

## Green button
- Pressing GREEN button allows to change the default parameter that rotator controls.
- For example, after pressing the green button by rotating the rotator we can select the Z access to be controlled by rotator. Then we make a selection by pressing the GREEN button again and use rotator to bring the Z axis up and down.
## Yellow Button
Sends abort commands to the CNC
