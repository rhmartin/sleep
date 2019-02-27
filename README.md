# ENGS 90 Serta Simmons Bedding Children's Sleep Monitoring System Firmware
## Sponsor: JD Velilla at Serta Simmons
### Code by: Cristian Vences, Rachel Martin, and Byung-Wook Choe
### Project members: Cristian Vences, Dan Choe, Emily Chao, Lily Hanig, Rachel Martin, & Sheppard Somers
### Advisor: Professor Geoffrey Luke
Completed Fall 2018 and Winter 2019

# Overview
This firmware is for use on the embedded controllers for the SSB sleep monitoring system for children. The code allows reading and processing of data from two Force Sensitive Resistors, an acceleromenter, and conductive thread. It then sends the data to a database hosted by Firebase.

# Functionality
'''Arduino_Code_Rev3.ino''' should be flashed onto the Arduino.
'''NodeMCU_Code_Reg6.ino''' should be flashed onto the NodeMCU.

The WiFi used to transmit the data from the NodeMCU to the Firebase is hardcoded in the NodeMCU code and should be written with the proper WiFi.
