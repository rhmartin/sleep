# ENGS 90 Serta Simmons Bedding Children's Sleep Monitoring System Firmware
## Sponsor: JD Velilla at Serta Simmons
### Code by: Cristian Vences, Rachel Martin, and Byung-Wook Choe
### Project members: Cristian Vences, Dan Choe, Emily Chao, Lily Hanig, Rachel Martin, & Sheppard Somers
### Advisor: Professor Geoffrey Luke
Completed Fall 2018 and Winter 2019

# Overview
This firmware is for use on the embedded controllers for the SSB sleep monitoring system for children. The code allows reading and processing of data from two Force Sensitive Resistors, an acceleromenter, and conductive thread. It then sends the data to a database hosted by Firebase.

# Usage
## ```Arduino_Code_Rev3.ino``` 
This file should be flashed onto the Arduino.
## ```NodeMCU_Code_Reg6.ino``` 
This file should be flashed onto the NodeMCU.
In order to connect to an individual's WPA2 protected WiFi, one must first connect to the WiFi ESPXXX and then use the portal that appears to select their WiFi.
The Firebase desired for storing data is also hardcoded in the file under the macro ```FIREBASE_HOST``` and ```FIREBASE_AUTH```. In order to use the device properly, the host should be the URL of the database with the https:// removed and the trailing / removed. The authorization key is the "secret" found in Settings->Project Settings->Service Accounts->Database Secrets.

# Associated Application
The github for the iOS application that displays the data sleep data graphically to parents can be found [here](https://github.com/emilyjchao/sleep).
