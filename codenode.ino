/* Intelligent Sleep Systems
 * ENGS 89/90
 * NodeMCU Code
 * Rev.6
 * 2/26/19
 */

//**Libraries**
#include <Time.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <stdio.h>
#include <time.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic


//**Definitions**//
//**Firebase and Wifi**//
#define FIREBASE_HOST "sleepdata-d5465.firebaseio.com"
#define FIREBASE_AUTH "4yrdL1zlT7J6iTsAWdlp1nJLMm3PA1Y25QciH3N1"
#define WIFI_SSID "Dartmouth Public"
#define WIFI_PASSWORD "No Password"

//**NodeMCU Interface**//
#define D0 16
#define D1 5    
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1
#define D11 9
#define D12 10

//**Other Pins**// 
#define fsrPin D4
#define ctPin D1
#define calPin D8

//**Serial communication b/w node and arduino**//
SoftwareSerial s(D6,D5);
int data; //variable to store data

//**State Variable Declarations**//
int onBed = 0;      //state variable for load cell status
int wetBed = 0;     //state variable for conductive thread status
String date;        //string to keep up with data
String UID;
String DID = "867530900";



void setup() {
  
  //**Serial beginnings**//
  s.begin(9600);          //begins serial comm. b/w node and arduino
  Serial.begin(9600);     //begins serial monitoring with 9600 baudrate
    
  //**Pin Declarations**//
  pinMode(fsrPin, INPUT);     //interfacing with load cell
  pinMode(ctPin, INPUT);     //interfacing with conductive thread

  pinMode(calPin, OUTPUT);    //calibration pin 
  digitalWrite(calPin,LOW); //set it to low initally

  //**Initiation**//
  Serial.println("DID: "+DID);
  //**WIFI**//
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.autoConnect();
    
  //**Firebase**// 
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //begins firebase database connection
  
  //**Get local time from interwebs**//
  Serial.println("Getting current time...");
  StaticJsonBuffer<50> jsonBuffer1; //to create Timestamp on Firebase
  StaticJsonBuffer<50> jsonBuffer2;
  JsonObject& timeStampObject = jsonBuffer1.createObject();
  timeStampObject[".sv"] = "timestamp";
  Firebase.set("current_time", timeStampObject);// Setting of timestamp to the firebase server
  if (Firebase.failed()) {
    Serial.print("pushing /date failed:");
    Serial.println(Firebase.error());
    return;
  }
  delay(1000);
  FirebaseObject object = Firebase.get("current_time"); // Retrive th JSON from firebase
  if (Firebase.failed()) {
    Serial.print("getting /date failed:");
    Serial.println(Firebase.error());
    return;
  }
  JsonObject& timeStampObject1 = jsonBuffer2.createObject(); // create a new Json object to store the json value
  timeStampObject1["time"] = object.getJsonVariant();   // Get the timestamp from json into timestampobject1.
  if (object.failed()) {
    Serial.print("getting the date from the object failed:");
    Serial.println(object.error());
  }
  String fireDate = timeStampObject1["time"];// store it into a variable
  //Serial.println(fireDate);
  Firebase.remove("current_time");
  delay(1000);
  int epoch = fireDate.substring(0, 10).toInt();  //convert fireDate to int
  time_t t = time_t(epoch);  //convert int to time_t
  setTime(t); //update internal clock
  if(hour()-5<18){
    date = String(month()) + "-" + String(day()-1) + "-" + String(year());  //writing out todays date
  } else {
    date = String(month()) + "-" + String(day()) + "-" + String(year());  //writing out todays date
  }
  //dealing with leap year
  if(day() == 0){
    date = String(2) + "-" + String(28) + "-" + String(year());  //writing out todays date
  } else if (day()-1 == 0) {
    date = String(2) + "-" + String(28) + "-" + String(year());  //writing out todays date    
  }
  Serial.println("Time Set!");
  Serial.print("Today's Date: ");
  Serial.println(date);

  //**Pairing**//
  Serial.println("Getting UID...");
  if(Firebase.getString("Pairing/"+DID+"/UID") == NULL)
  {
    Firebase.setString("Pairing/"+DID+"/UID","0");
    Serial.print("Pairing");
    while(Firebase.getString("Pairing/"+DID+"/UID") == "0")
    {
    Serial.print(".");
    delay(500);
    }
    Serial.println(" ");
    UID = Firebase.getString("Pairing/"+DID+"/UID");
  } else {
    UID = Firebase.getString("Pairing/"+DID+"/UID");
  }
  Firebase.setBool("userData/"+UID+"/Profile/hasD",true);
  Serial.print("UID: ");
  Serial.println(UID);  
  
  //**Calibration**//
  Firebase.setInt("userData/"+UID+"/Profile/calibrate",0);
  Serial.println("Waiting to calibrate");
  while(Firebase.getInt("userData/"+UID+"/Profile/calibrate") == 0){ //waits for user to set to calibration mode
    Serial.print(".");
    delay(500);
  }
  delay(500);
  Serial.println(" ");
  digitalWrite(calPin, HIGH); //tells arduino to go in calibration state
  delay(500);
  Serial.println("Calibrating");
  int i = 0;
  while(i < 10){    //gives arduino 10 seconds to calibrate
    Serial.print(".");
    delay(1000);
    i++;
  }
  Serial.println(" ");
  digitalWrite(calPin, LOW);
  delay(500);
  Firebase.setInt("userData/"+UID+"/Profile/calibrate",2);
  Serial.println("Initiation Complete!");
  
}



void loop() {

//**Keeping up w/ today's date**//
  if(hour()-5 == 18 & second() < 5){
    date = String(month()) + "-" + String(day()) + "-" + String(year());  //writing out todays date
    Serial.println(date);
  }
//  Serial.print("String now(): ");
//  Serial.println(String(now()));

  //**Updating firebase with sensors**//
  Serial.print("onBed value: " );     //testing
  Serial.println(onBed);
  Serial.print("FSR Pin Value: ");
  Serial.println(digitalRead(fsrPin));
  Serial.print("wetBed value: " );     //testing
  Serial.println(wetBed);
  Serial.print("CT Pin Value: ");
  Serial.println(digitalRead(ctPin));


  //**Load Cell**//
  if (onBed == 0 && digitalRead(fsrPin) == HIGH) {         //if you werent on bed and now are, update firebase
    Serial.println("---------UPDATE--------");
    Serial.println("In the enter bed state");
    Serial.println("---------UPDATE--------");    
    onBed = 1; 
    Firebase.pushInt(date+"/"+"enters", int(now()));
    delay(200); 
    Firebase.pushInt("userData/"+UID+"/"+date+"/"+"enters", int(now()));  
    if (Firebase.failed()) {
      Serial.print("pushing /onBed failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
  if (onBed == 1 && digitalRead(fsrPin) == LOW) {            //if you were on bed and get out, update firebase
    Serial.println("--------UPDATE-------");
    Serial.println("In the exit bed state");
    Serial.println("--------UPDATE-------");    
    onBed = 0;
    Firebase.pushInt(date+"/"+"exits", int(now()));
    delay(200);
    Firebase.pushInt("userData/"+UID+"/"+date+"/"+"exits", int(now())); 
    if (Firebase.failed()) {
      Serial.print("pushing /onBed failed:");
      Serial.println(String(Firebase.error()));
      return;
    }
  }


  //**Conductive Thread**//
  if (wetBed == 0 && digitalRead(ctPin) == HIGH) {         //if you wet the bed, upnight firebase
    Serial.println("------UPDATE-----");
    Serial.println("wet bed state");
    Serial.println("------UPDATE-----");
    wetBed = 1;
//    Firebase.pushInt(date+"/"+"wets", int(now()));
//    delay(200);
//    Firebase.pushInt("userData/"+UID+"/"+date+"/"+"wets", int(now()));
    if (Firebase.failed()) {
      Serial.print("pushing /wetBed failed:");
      Serial.println(Firebase.error());
      return;
    }
  }
  if (wetBed == 1 && digitalRead(ctPin) == LOW) {            //if you dryed the bed, upnight firebase
    Serial.println("------UPDATE-----");
    Serial.println("Dry bed state");
    Serial.println("------UPDATE-----");
    wetBed = 0;
  }

  //**Accel**//
  if(onBed == 1){ //only update databse with movement if in the onbed state
    s.write("s");
    if (s.available()>0)
    {
      data=s.read();
      Serial.print("Accel value: ");
      Serial.println(data);
      Firebase.pushString(date+"/"+"movement", String(now()) + "000 " + String(data));
      delay(200);
      Firebase.pushString("userData/"+UID+"/"+date+"/"+"movement", String(now()) + "000 " + String(data));
      if (Firebase.success() == false) {
        Serial.print("pushing /movemenet failed:");
        Serial.println(Firebase.error());
        return;
     }
    }
  }
  delay(1000);
  Serial.println(" ");
}
