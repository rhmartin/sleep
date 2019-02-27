/* Intelligent Sleep Systems
 * ENGS 89/90
 * Arduino Uno Code
 * Rev. 3
 * 2/26/19
 */
 
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
SoftwareSerial s(7, 8);

//FSR SETUP
int fsrPin = A0;     // the FSR and 10K pulldown are connected to a0
int fsrReading;     // the analog reading from the FSR resistor divider
int fsrVoltage;     // the analog reading converted to voltage
unsigned long fsrResistance;  // The voltage converted to resistance, can be very big so make "long"

int fsrPin2 = A1;     // the FSR and 10K pulldown are connected to a0
int fsrReading2;     // the analog reading from the FSR resistor divider
int fsrVoltage2;     // the analog reading converted to voltage
unsigned long fsrResistance2;  // The voltage converted to resistance, can be very big so make "long"

unsigned long threshold = 10000; // initial threshold before calibration
int fsrOut = 2;   // the pin the will indicate to the nodemcu of whether someone is on the bed or not
int calPin = 10;  // pin that reads a signal from the nodemcu to put the arduino in a calibration state
//END FSR SETUP


//CONDUCTIVE THREAD SETUP
int ctPin = A2;     // the ct and 10K pulldown are connected to a4
int ctOut = 3;      // the pin that will indicate to the nodemcu of a wet occurance
int ctReading;     // the analog reading from the ct resistor divider
unsigned long ctResistance;  // The voltage converted to resistance, can be very big so make "long"
//END CONDUCTIVE THREAD SETUP

//ACCEL SETUP
const int xpin = A3;                  // x-axis of the accelerometer
const int ypin = A4;                  // y-axis
const int zpin = A5;                  // z-axis (only on 3-axis models)
float x;    // where the x reading is stored
float y;    // where the y readings is stored
float z;    // where the z reading is stored
int i = 0;  // counter variable for averaging purposes
float avgMag = 0; // where the average magnitude will be stored
float newMag = 0; // where the 0-100 value will be stored for serially communication b/w nodemcu
//END ACCEL SETUP


void setup(void) {

  //SERIAL BEGINNINGS//
  s.begin(9600);        // serial communication b/w arduino and nodemcu
  Serial.begin(9600);   // usb serial communication for serial monitoring

  //ACCEL//
  pinMode(xpin, INPUT);
  pinMode(ypin, INPUT);
  pinMode(zpin, INPUT);

  //CONDUCTIVE THREAD//
  pinMode(ctPin, INPUT);
  pinMode(ctOut, OUTPUT);

  //FSR//
  pinMode(fsrPin, INPUT);
  pinMode(fsrPin2, INPUT);
  pinMode(fsrOut, OUTPUT);

  //CALIBRATION//
  pinMode(calPin, INPUT);


}

void loop(void) {

  //START LOAD CELL CODE//
  Serial.println("----------------LOAD CELL----------------");
  
  //read pins
  fsrReading = analogRead(fsrPin);
  fsrReading2 = analogRead(fsrPin2);
  //Serial.print("Analog reading fsrReading = ");
  //Serial.println(fsrReading);
  //Serial.print("Analog reading fsrReading2 = ");
  //Serial.println(fsrReading2);

  //analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
  fsrVoltage = map(fsrReading, 0, 1023, 0, 5000);
  fsrVoltage2 = map(fsrReading2, 0, 1023, 0, 5000);
  //Serial.print("Voltage reading in mV fsrVoltage = ");
  //Serial.println(fsrVoltage);
  //Serial.print("Voltage reading in mV fsrVoltage2 = ");
  //Serial.println(fsrVoltage2);

  //analog voltage converted into ohms
  //fsr 1
  if (fsrVoltage == 0) {
    fsrResistance = 1000000; // if fsrVoltage reads 0 then effective resistance is really high
    Serial.print("FSR resistance1 in ohms = ");
    Serial.println(fsrResistance);

  } else {
    //The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    //so FSR = ((Vcc - V) * R) / V        yay math!
    fsrResistance = 5000 - fsrVoltage;     //fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance *= 11230;                //11230 ohm resistor
    fsrResistance /= fsrVoltage;
    Serial.print("FSR resistance1 in ohms = ");
    Serial.println(fsrResistance);
  }

  //fsr 2
  if (fsrVoltage2 == 0) {
    fsrResistance2 = 1000000;  // if fsrVoltage2 reads 0 then effective resistance is really high
    Serial.print("FSR resistance2 in ohms = ");
    Serial.println(fsrResistance2);


  } else {
    fsrResistance2 = 5000 - fsrVoltage2;     //fsrVoltage is in millivolts so 5V = 5000mV
    fsrResistance2 *= 111230;                //11230 ohm resistor
    fsrResistance2 /= fsrVoltage2;
    Serial.print("FSR resistance2 in ohms = ");
    Serial.println(fsrResistance2);
  }

  //calibration
  //takes the worst case scenario and scales it down to be able to detect presence of a child
    if(digitalRead(10) == HIGH){
      threshold = min(fsrResistance,fsrResistance2) * .85; 
      Serial.print("New threshold: ");
      Serial.println(min(fsrResistance,fsrResistance2)*.85);
      delay(200);
    }
    
  //add resistances
  unsigned long addResistance = fsrResistance + fsrResistance2;
  Serial.print("Added Resistances: ");
  Serial.println(addResistance);

  //threshold
  Serial.print("Threshold: ");
  Serial.println(threshold);

  //make decision whether child is present based on threshold on fsr resistances
  // if the fsrResistances or the sum are lower than it triggers as someone is on
  if (fsrResistance < threshold || fsrResistance2 < threshold || addResistance < threshold) {
    digitalWrite(fsrOut, HIGH);
    Serial.println("Load cell status: ON");
  } else {
    digitalWrite(fsrOut, LOW);
    Serial.println("Load cell status: NOT ON");
  }
  //END LOAD CELL CODE//

  //START ACCEL//
  Serial.println("------------------ACCEL------------------");

  //read pins with small delays b/w readings
  Serial.print("Analog readings: ");
  x = analogRead(xpin);
  delay(1);
  Serial.print(x);
  Serial.print("\t");

  y = analogRead(ypin);
  delay(1);
  Serial.print(y );
  Serial.print("\t");
  
  z = analogRead(zpin);
  Serial.print(z);
  Serial.print("\n");

  //map to mili-gs
  Serial.print("Mili-gs: ");
  
  float mx= map(x,0,700,-3000,+3000);
  Serial.print(mx);
  Serial.print("\t");
  
  float my= map(y,0,700,-3000,+3000);
  Serial.print(my);
  Serial.print("\t");
  
  float mz= map(z,0,700,-3000,+3000);
  Serial.print(mz);
  Serial.print("\n");
  
  
  //calculate magnitude of the acceleration
  float mag = sqrt((mx * mx) + (my * my) + (mz * mz));
  mag -= 3000; //subtract gravity
  mag = abs(mag); //make sure no negatives
  Serial.println("mag =" + String(mag));
  
  //add magnitudes together to compute an average
  avgMag += mag;
  i++;
  Serial.println("avgMag =" + String(avgMag));
  
  if (i > 90) {
    avgMag /= 90;  //gets final average from 5 readings
    newMag = map(avgMag,0,1000, 0,100); //maps this to 255 cause max is 255 serially
    Serial.print("Out going accel data: ");
    Serial.println(newMag);
    //send averaged mag data serially
    if (s.available() > 0)
    {
      //we divide by four because 255 apparently is the max value that can be sent
      s.write(newMag);
      Serial.print("Out going accel data: ");
      Serial.println(newMag);
    }
    delay(1000);
    i = 0;
    avgMag = 0;
  }
  //END ACCEL

  //START CONDUCTIVE THREAD
  Serial.println("------------CONDUCTIVE THREAD------------");

  //read pin
  ctReading = analogRead(ctPin);
  //Serial.print("Analog reading = ");
  //Serial.println(ctReading);

  //analog voltage reading ranges from about 0 to 1023 which maps to 0V to 5V (= 5000mV)
  ctReading = map(ctReading, 0, 1023, 0, 5000);
  //Serial.print("Voltage reading in mV = ");
  //Serial.println(ctReading);

  if (ctReading == 0) {
    //Serial.println("No pressure");
  } else {
    //The voltage = Vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    //so FSR = ((Vcc - V) * R) / V        yay math!
    ctResistance = 5000 - ctReading;     //ctReading is in millivolts so 5V = 5000mV
    ctResistance *= 221300;                //221300 ohm resistor
    ctResistance /= ctReading;
    Serial.print("Conductive Thread in Ohms = ");
    Serial.println(ctResistance);
  }

  //if the resistance is lower than the threshold then the arduino tells the nodemcu of a wet occurance
  if (ctResistance < 20000) {
    digitalWrite(ctOut, HIGH);
    Serial.println("CT status: WET");
    Serial.print("CT Resistance: ");
    Serial.println(ctResistance);
  }
  else {
    digitalWrite(ctOut, LOW);
    Serial.println("CT status: DRY");
  }
  
  //END CONDUCTIVE THREAD//
  
  delay(1000);

  //END ENTIRE LOOP//
}
