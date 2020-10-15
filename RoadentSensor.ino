

#include "WiFi_login.h" // in file to be excluded from GIT

#include <ESP8266HTTPClient.h>

// Define IR sensor pin:
#define IR_SENSOR D3

// Define LED pins:
int redLED = D7;
int greenLEDs = D8;

// Define wheel diameter constant [mm]:
#define Diameter 130

// Variable for recording distance:
float Distance = 0;

// Define integer variables:
int Counts = 0; // counts the number of times that the IR beam has been broken
int DistInt = 0; // rounded distance
int pi = 0; // serial message from Pi

// Define timing variables:
float LastTrigger = 0.0;
float Period = 0.0; 
float Speed = 0.0;
float MaxSpeed = 0.0;
float LastSpeed = 0.0;

///////////////////////////////////////////////////////
// get time from internet - based on NTP-TZ-DST (v2) //
//               *** start of code ***               //
///////////////////////////////////////////////////////

#include <TZ.h>
#define MYTZ TZ_Europe_London

#include <ESP8266WiFi.h>
#include <coredecls.h>                  // settimeofday_cb()
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

// this line is necessary, not sure what it does
extern "C" int clock_gettime(clockid_t unused, struct timespec *tp);

// An object which can store a time
static time_t now;

void showTime() {   // This function is used to print stuff to the serial port, it's not mandatory
  now = time(nullptr);      // Updates the 'now' variable to the current time value
  Serial.print("seconds since epoc: ");
  Serial.println((uint32_t)now);
  Serial.println();
}

void time_is_set_scheduled() {    // This function is set as the callback when time data is retrieved
  // In this case we will print the new time to serial port, so the user can see it change (from 1970)
  showTime();
}

void ICACHE_RAM_ATTR BeamBreak ();


void setup() {

  // Setup the LED:
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(redLED,OUTPUT);
  pinMode(greenLEDs,OUTPUT);


    
  // Setup the IR sensor input with a pull-up resistor:
  //  pinMode(IR_SENSOR,INPUT);
  pinMode(IR_SENSOR,INPUT); // internal pull-up replaced with 3.3v wired pull-up (sharing sensor with 3.3v node MCU)
  
  // Attach interrupt tp IR sensor pin:
  attachInterrupt(digitalPinToInterrupt(IR_SENSOR),BeamBreak, FALLING);
  
  Serial.begin(115200);
  Serial.println("\nStarting...\n");

  // install callback - called when settimeofday is called (by SNTP or us)
  // once enabled (by DHCP), SNTP is updated every hour
  settimeofday_cb(time_is_set_scheduled);

  // This is where your time zone is set
  configTime(MYTZ, "pool.ntp.org");

  // start network
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  digitalWrite(redLED, HIGH);

  // On boot up the time value will be 0 UTC, which is 1970.  
  // This is just showing you that, so you can see it change when the current data is received
  Serial.printf("Time is currently set by a constant:\n");
  showTime();

}

///////////////////////////////////////////////////////
// get time from internet - based on NTP-TZ-DST (v2) //
//                *** end of code ***                //
///////////////////////////////////////////////////////

// variables for timing
int timeTrigger = 0;
int period = 3600;
long timeStamp = 0;


// fake data for testing
int distance = 0;
int _speed = 0;
int wheel = 100;

String postData; // string of data to be posted to PHP on server


void BeamBreak() {
  
  Counts++;
//  Serial.print("interrupt");
  // Calculated distance in meters:
  Distance = Diameter * 3.14 * Counts / 1000;
  DistInt = Distance;
  Period = millis() - LastTrigger;
  Speed = Diameter / Period * 7.028; // pi x 1000ms x 2.23694 mph/ m/s

  if (Speed - LastSpeed < 3) { // filters out false readings
    
    LastSpeed = Speed; // updates max speed
    if (Speed > MaxSpeed) {
      MaxSpeed = Speed;
    }      
    
    LastTrigger = millis(); 
  }
}


void upload2(String postData) {
    int httpCode = 0;
    while(httpCode != 200){
      HTTPClient http;
      http.begin("http://www.hackminster.co.uk/URLpost.php");
  
      http.addHeader("Content-Type","application/x-www-form-urlencoded");
  
      httpCode = http.POST(postData);
  
      String payload = http.getString();
  
      Serial.println(httpCode);
      Serial.println(payload);
  
      http.end();
      delay(10000);
    }
    

}



void loop() {

  // Illuminate board LED each time IR beam is broken:
  if (digitalRead(IR_SENSOR) == LOW) {
    digitalWrite(greenLEDs, LOW); 
  }
  else
  {
    digitalWrite(greenLEDs, HIGH);
  }


  
  now = time(nullptr); 
  if ( (uint32_t)now > timeTrigger ) {
    
 

    
    //
    Serial.println(ctime(&now));     
    timeTrigger = ((uint32_t)now/period)*period+period;
    timeStamp = ((uint32_t)now/period)*period-1;  
    
    String postData = "wheel="+String(wheel)+"&distance="+String(DistInt)+"&speed="+String(MaxSpeed)+"&datetime="+String(timeStamp);

    DistInt = 0;
    MaxSpeed = 0;
    Counts = 0;
    
    Serial.print("postData: ");
    Serial.println(postData);
    
    if(WiFi.status()== WL_CONNECTED){
//      HTTPClient http;
//      http.begin("http://www.hackminster.co.uk/POST_exp.php");
//
//      http.addHeader("Content-Type","application/x-www-form-urlencoded");
//  
//      int httpCode = http.POST(postData);
//
//      String payload = http.getString();
//  
//      Serial.println(httpCode);
//      Serial.println(payload);
//  
//      http.end();

    upload2(postData);
  
    }else{
      Serial.println("Error in WiFi connection");
//      delay(30000);
//      if(WiFi.status()== WL_CONNECTED){
//        upload2(postData);
//      }else{
//        Serial.println("Error in WiFi connection again");
//      }
      
    }


  }


}
