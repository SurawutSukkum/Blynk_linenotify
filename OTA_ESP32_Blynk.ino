/*************************************************************

  This is a simple demo of sending and receiving some data.
  Be sure to check out other examples!
 *************************************************************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID ""
#define BLYNK_TEMPLATE_NAME ""
#define BLYNK_AUTH_TOKEN ""

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <NTPClient.h>
#include <esp_task_wdt.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "";
char pass[] = "";

// เปลี่ยน SSID และ PASSWORD เป็นของตัวเอง
const char* host = "esp32";


// ตั้งค่า Port ของ Web Server เป็น Port 80
WebServer server(80);

// หน้า Login
const char* loginIndex = 
 "<form name='loginForm'>"
    "<table width='20%' bgcolor='A09F9F' align='center'>"
        "<tr>"
            "<td colspan=2>"
                "<center><font size=4><b>ESP32 Login Page</b></font></center>"
                "<br>"
            "</td>"
            "<br>"
            "<br>"
        "</tr>"
        "<td>Username:</td>"
        "<td><input type='text' size=25 name='userid'><br></td>"
        "</tr>"
        "<br>"
        "<br>"
        "<tr>"
            "<td>Password:</td>"
            "<td><input type='Pass' size=25 name='pwd'><br></td>"
            "<br>"
            "<br>"
        "</tr>"
        "<tr>"
            "<td><input type='submit' onclick='check(this.form)' value='Login'></td>"
        "</tr>"
    "</table>"
"</form>"
"<script>"
    "function check(form)"
    "{"
    "if(form.userid.value=='admin' && form.pwd.value=='admin')"
    "{"
    "window.open('/serverIndex')"
    "}"
    "else"
    "{"
    " alert('Error Password or Username')/*displays error message*/"
    "}"
    "}"
"</script>";
 

// หน้า Index Page
const char* serverIndex = 
"<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
"<form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
   "<input type='file' name='update'>"
        "<input type='submit' value='Update'>"
    "</form>"
 "<div id='prg'>progress: 0%</div>"
 "<script>"
  "$('form').submit(function(e){"
  "e.preventDefault();"
  "var form = $('#upload_form')[0];"
  "var data = new FormData(form);"
  " $.ajax({"
  "url: '/update',"
  "type: 'POST',"
  "data: data,"
  "contentType: false,"
  "processData:false,"
  "xhr: function() {"
  "var xhr = new window.XMLHttpRequest();"
  "xhr.upload.addEventListener('progress', function(evt) {"
  "if (evt.lengthComputable) {"
  "var per = evt.loaded / evt.total;"
  "$('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "}"
  "}, false);"
  "return xhr;"
  "},"
  "success:function(d, s) {"
  "console.log('success!')" 
 "},"
 "error: function (a, b, c) {"
 "}"
 "});"
 "});"
 "</script>";

BlynkTimer timer;

WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 25200;
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", utcOffsetInSeconds);

int pin0 = 27;
int pin1 = 12;
int pin2 = 13;
int pin3 = 14;

int startTime1;
int endTime1;
int startTime2;
int endTime2;
int startTime3;
int endTime3;
int startTime4;
int endTime4;

#define WDT_TIMEOUT 28

void setup()
{
  Serial.begin(115200);
  delay(100);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  pinMode(pin0, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
 
  digitalWrite(pin0, HIGH);
  digitalWrite(pin1, HIGH);
  digitalWrite(pin2, HIGH);
  digitalWrite(pin3, HIGH);

  timeClient.begin();
  WiFi.begin(ssid, pass);
  Serial.println("");
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  };
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
    Serial.print("Version 2.0");
  delay(3000);



  // Setup a timer function to be called every second
  timer.setInterval(1000L, myTimerEvent);

  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch


  
// ใช้งาน mDNS
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  
// แสดงหน้า Server Index หลังจาก Login
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  
// ขั้นตอนการ Upload ไฟล์
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {

// แฟลช(เบิร์นโปรแกรม)ลง ESP32
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
}
void myTimerEvent() // This loop defines what happens when timer is triggered
{

}
BLYNK_WRITE(V1)
{
    int value = param.asInt();
    if(value == 1)
    {
      digitalWrite(pin0, LOW);
      Serial.print("PIN0 is ");
      Serial.println(pin0);
      Serial.println("PIN0 Status is LOW");
      delay(500);
    }
    else{
      digitalWrite(pin0, HIGH);
      Serial.print("PIN0 is ");
      Serial.println(pin0);
      Serial.println("PIN0 Status is HIGH");
      delay(500);
    }
}

BLYNK_WRITE(V2)
{
    int value = param.asInt();
    if(value == 1)
     { 
       digitalWrite(pin1, LOW);
      Serial.print("PIN1 is ");
      Serial.println(pin1);
      Serial.println("PIN1 Status is LOW");
      delay(500);
     } 
    else
    {
      digitalWrite(pin1, HIGH);
      Serial.print("PIN1 is ");
      Serial.println(pin1);
      Serial.println("PIN1 Status is HIGH");
      delay(500); 
    }
 
}

BLYNK_WRITE(V3)
{
    int value = param.asInt();
    if(value == 1)
     {
      digitalWrite(pin2, LOW);
      Serial.print("PIN2 is ");
      Serial.println(pin2);
      Serial.println("PIN2 Status is LOW");
     }
    else
    {
      digitalWrite(pin2, HIGH);
      Serial.print("PIN2 is ");
      Serial.println(pin2);
      Serial.println("PIN2 Status is HIGH");
      delay(500);
}
}

BLYNK_WRITE(V10)
{
    int value = param.asInt();
    if(value == 1)
    {
      digitalWrite(pin3, LOW);
      Serial.print("PIN3 is ");
      Serial.println(pin3);
      Serial.println("PIN3 Status is LOW");
      delay(500);
    }
    else
     {
      digitalWrite(pin3, HIGH);
      Serial.print("PIN3 is ");
      Serial.println(pin3);
      Serial.println("PIN3 Status is HIGH");
      delay(500);
     }
}


BLYNK_WRITE(V7){
 startTime1 = param[0].asInt();
 endTime1 = param[1].asInt();
 Serial.print("startTime1 is ");
 Serial.println(startTime1);
 Serial.print("endTime1 is ");
 Serial.println(endTime1);
 if(startTime1 == 0 && endTime1 == 0)
 {
  startTime1 = 999999;
  endTime1 = 999999;
 }
   delay(500);
}

BLYNK_WRITE(V8){
 startTime2 = param[0].asInt();
 endTime2 = param[1].asInt();
 Serial.print("startTime2 is ");
 Serial.println(startTime2);
 Serial.print("endTime2 is ");
 Serial.println(endTime2);
 if(startTime2 == 0 && endTime2 == 0)
 {
  startTime2 = 999999;
  endTime2 = 999999;
 }
   delay(500);
}

BLYNK_WRITE(V9){
 startTime3 = param[0].asInt();
 endTime3 = param[1].asInt();
 if(startTime3 == 0 && endTime3 == 0)
 {
  startTime3 = 999999;
  endTime3 = 999999;
 }
 Serial.print("startTime3 is ");
 Serial.println(startTime3);
 Serial.print("endTime3 is ");
 Serial.println(endTime3); 
  delay(500);
}

BLYNK_WRITE(V12){
 startTime4 = param[0].asInt();
 endTime4 = param[1].asInt();
 if(startTime4 == 0 && endTime4 == 0)
 {
  startTime4 = 999999;
  endTime4 = 999999;
 }
  Serial.print("startTime4 is ");
 Serial.println(startTime4);
 Serial.print("endTime4 is ");
 Serial.println(endTime4);
   delay(500);
}

String date[]={"sunday", "monday", "tuesday", "wednesday", "thursday", "friday", "saturday"};
void loop() {



  esp_task_wdt_reset();

  timeClient.update();
  int HH = timeClient.getHours();
  int MM = timeClient.getMinutes();
  int SS = timeClient.getSeconds();
  int dd = timeClient.getDay();
  Serial.print("Date ");
  Serial.print(date[dd]);


  Serial.print("Time ");
  Serial.print(String(HH));  
  Serial.print(":");
  Serial.print(String(MM));
  Serial.print(":");
  Serial.println(String(SS));
  int server_time = 3600*HH + 60*MM + SS;

   if(startTime1 == server_time)
   {
    digitalWrite(pin0, LOW);
   }

   if(startTime2 == server_time)
   {
   digitalWrite(pin1, LOW);
   }

   if(startTime3 == server_time)
   {
   digitalWrite(pin2, LOW);
   }

   if(startTime4 == server_time)
   {
   digitalWrite(pin3, LOW);
   }
   
   if(endTime1 == server_time)
   {
   digitalWrite(pin0, HIGH);
   }

   if(endTime2 == server_time)
   {
   digitalWrite(pin1, HIGH);
   }

   if(endTime3 == server_time)
   {
   digitalWrite(pin2, HIGH);
   }

   if(endTime4 == server_time)
   {
   digitalWrite(pin3, HIGH);
   }
 
   Blynk.run();
   timer.run();
   

   // Server พร้อมการเปิดหน้าเว็บทุก Loop (เพื่อรอการเรียกหน้าเว็บเพื่อรับโปรแกรมใหม่ตลอดเวลา)
  server.handleClient();
  delay(500);
}
