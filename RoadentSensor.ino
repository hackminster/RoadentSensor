

#include "WiFi_login.h" // in file to be excluded from GIT

#include <ESP8266HTTPClient.h>

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

void setup() {
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

  // On boot up the time value will be 0 UTC, which is 1970.  
  // This is just showing you that, so you can see it change when the current data is received
  Serial.printf("Time is currently set by a constant:\n");
  showTime();

}

///////////////////////////////////////////////////////
// get time from internet - based on NTP-TZ-DST (v2) //
//                *** end of code ***                //
///////////////////////////////////////////////////////

// 
int timeTrigger = 0;
int period = 3600;
long timeStamp = 0;

int distance = 327;
int _speed = 2.37;
int wheel = 128;

String postData;


void loop() {
  now = time(nullptr); 
  if ( (uint32_t)now > timeTrigger ) {
    Serial.println(ctime(&now));     
    timeTrigger = ((uint32_t)now/period)*period+period;
    timeStamp = ((uint32_t)now/period)*period-1;  

    
    String postData = "wheel="+String(wheel)+"&distance="+String(distance)+"&speed="+String(_speed)+"&datetime="+String(timeStamp);
    
    Serial.print("postData: ");
    Serial.println(postData);
    if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;
      http.begin("http://www.hackminster.co.uk/POST_exp.php");
//      http.begin("http://www.hackminster.co.uk/URLpost.php");
      http.addHeader("Content-Type","application/x-www-form-urlencoded");
  
      int httpCode = http.POST(postData);

      String payload = http.getString();
  
      Serial.println(httpCode);
      Serial.println(payload);
  
      http.end();
  
    }else{
      Serial.println("Error in WiFi connection");
    }


  }


}
