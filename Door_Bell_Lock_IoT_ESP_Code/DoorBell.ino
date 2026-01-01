#include<cvzone.h>
#include "arduino_secrets.h"
#include "thingProperties.h"


SerialData serialData(1,1); 
int valsRec[2];
int sendVals[2];
bool flagCloudAction =0; 

#define alert 4
#define flag 2 
#define doorLock 18

void setup() {
  // put your setup code here, to run once:

  // Defined in thingProperties.h
  initProperties();

   //Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  //Serial.begin(9600); 
  serialData.begin(9600); 
  pinMode(alert, OUTPUT);
  pinMode(doorLock, OUTPUT); 
  pinMode(flag, INPUT); 

  

}

void loop() {
  // put your main code here, to run repeatedly:

  ArduinoCloud.update();
  serialData.Get(valsRec);

  if (valsRec[0]==1) 
   {
    bell = true; 
    digitalWrite(alert,HIGH);
    delay(200);
    digitalWrite(alert,LOW);
    delay(200);
    digitalWrite(alert,HIGH);
    delay(200);
    digitalWrite(alert,LOW);
    
    valsRec[0]=0;
   }
    
  else  {
    digitalWrite(alert,LOW);
    bell = false; 
  }

  if (digitalRead(flag)){


        digitalWrite(doorLock, HIGH);
        sendVals[0] = 7; 
        serialData.Send(sendVals);
        lockCloud = true;
        lockChart =1;
        ArduinoCloud.update();
        delay(3000);
        digitalWrite(doorLock, LOW);
        lockCloud = false;
        lockChart = 0; 
  }

  else  {

    sendVals[0] =5; 
    serialData.Send(sendVals);
    delay(40);

  }

 
}

void onFlagCloudChange()  {
  // Add your code here to act upon FlagCloud change

  if (flagCloud) {

        digitalWrite(doorLock, HIGH);
        lockCloud = true;
        lockChart =1;
        sendVals[0] = 7; 
        serialData.Send(sendVals);
        ArduinoCloud.update();
        delay(3000);
        digitalWrite(doorLock, LOW);
        lockCloud = false;
        lockChart = 0; 
    }

  else  {

    sendVals[0] =5; 
    serialData.Send(sendVals);
    delay(40); //25

  }
  
}
