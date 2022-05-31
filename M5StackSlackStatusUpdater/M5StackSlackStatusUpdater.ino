#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <M5Core2.h>
#include <WiFi.h>
#include "time.h"
#include "arduino_secrets.h"
#include "AXP192.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// variables - change these if needed
///////////////////////////////////////////////////////////////////////////////////////////////////

// 2500 (dim) to 3300 (bright)
int disp_volt = 2525;

// 1 (smallest) to 7 (biggest)
int font_size = 2;

// Secrets grabbed from the arduino_secrets.h file
char ssid[]    = SECRET_SSID;
char pass[]    = SECRET_PASS;
char api[]     = SECRET_SLACK_API_TOKEN;
char api_str[100];

// Slack statuses - Change these!
char labelA[]  = "Lunch";
char statusA[] = "Lunch";
char emojiA[]  = ":pizza:";

char labelB[]  = "BRB";
char statusB[] = "BRB";
char emojiB[]  = ":running:";

// Setting a blank status resets your status
char labelC[]  = "Reset";
char statusC[] = "";
char emojiC[]  = "";

// Date/time
const char* ntpServer = "pool.ntp.org";  // NTP server to use
const long  gmtOffset_sec = -18000;      // How many seconds (seconds = hours * 3600) away from UTC you are (e.g., NYC is -5 hours, so -18000 seconds)
const int   daylightOffset_sec = 3600;   // How many seconds offset to calculate for daylight savings time


///////////////////////////////////////////////////////////////////////////////////////////////////
// program - dont touch anything below here!
///////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////
// functions
/////////////////////////////////
void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    M5.Lcd.println("Failed to obtain time");
    return;
  }
  M5.Lcd.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
}

void batteryPower()
{
  float batVoltage = M5.Axp.GetBatVoltage();
  //Serial.print("STATE: Battery voltage: ");
  //Serial.println(batVoltage);
  float batPercentage = ( batVoltage < 3.2 ) ? 0 : ( batVoltage - 3.2 ) * 100;
  String f_batPercentage = String(batPercentage, 1);
  M5.Lcd.print(f_batPercentage);
  M5.Lcd.println("%");
  //Serial.print("STATE: Battery percentage: ");
  //Serial.println(batPercentage);
}

void slackStatusUpdater(char* pass_status, char* pass_emoji){
  /*
    input:    1. status text
              2. status emoji text

    output:   Prints to the LCD and serial
  */
  // Declare an object
  HTTPClient http;

  // Set headers
  http.begin("https://slack.com/api/users.profile.set");
  http.addHeader("Authorization", api_str);
  http.addHeader("Content-Type", "application/json; charset=utf-8");

  // Create a JSON document
  // What we need is something that looks like this
  //  {
  //    "profile": {
  //      "status_text": "riding a train",
  //      "status_emoji": ":mountain_railway:"
  //    }
  //  }
  DynamicJsonDocument doc(1024);
  JsonObject profile = doc.createNestedObject("profile");
  profile["status_text"] = pass_status;
  profile["status_emoji"] = pass_emoji;

  // Store the document in a string, the serialize the data to JSON
  String requestBody;
  serializeJson(doc, requestBody);
  Serial.print("STATE: Request body is: ");
  Serial.println(requestBody);

  // Send the POST request
  // Get the integer value of the response code and the response of the actual request
  int httpResponseCode = http.POST(requestBody);
  String response = http.getString();

  // Handle the response code
  if(httpResponseCode > 0){
    // If response code is a positive value, print the value
    M5.Lcd.setCursor(1, 32);
    M5.Lcd.println(httpResponseCode);
    Serial.print("STATE: Response code is: ");
    Serial.println(httpResponseCode);
    Serial.print("STATE: Response is: ");
    Serial.println(response);

    // If response code is a positive value, that means Slack is online and your request was received by Slack, but not that your status was actually updated (e.g., bad password)
    // Grab the returned JSON and look for the value of the key "ok" (will be a bool)
    DeserializationError status = deserializeJson(doc, response);
    bool ok = doc["ok"];

    // Handle "ok" status
    if(ok){
      // If status is ok, great
      M5.Lcd.println("Update accepted");
      Serial.print("STATE: OK status is: ");
      Serial.println(ok);
    } else {
      // If status is not "ok", grab the value of the key "error" and close the connection
      const char* err = doc["error"];
      M5.Lcd.println("Update failed!");
      M5.Lcd.println(err);
      Serial.println("ERROR: Update failed! (see error above)");
      http.end();
    }

   } else {
    // If response code is not a positive value, print the value and close the connection
    M5.Lcd.setCursor(1, 32);
    M5.Lcd.println(httpResponseCode);
    Serial.print("ERROR: Response code is: ");
    Serial.println(httpResponseCode);
    Serial.print("ERROR: Response is: ");
    Serial.println(response);
    http.end();
  }

  // Close the connection
  http.end();
}


void setup(){
  // Initialize some stuff
  Serial.begin(115200);
  Serial.println("STATE: Starting setup");
  M5.begin();

  // Use the variables set earlier
  M5.Axp.SetLcdVoltage(disp_volt);
  M5.Lcd.setTextSize(font_size);

  // Build the bearer token string
  // Should look like "Bearer xoxp-......."
  strcpy(api_str, "Bearer ");
  strcat(api_str, api);
  //Serial.println(api_str);

  // Disconnect from previously connected access point(s)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  M5.Lcd.setCursor(1, 1);
  M5.Lcd.clear();
  M5.Lcd.println("Connecting to WiFi...");
  Serial.println("STATE: Connecting to WiFi...");
  delay(1000);
  WiFi.begin(ssid, pass);

  // Try to connect to WiFi forever
  while (WiFi.status() != WL_CONNECTED)
  {
    M5.Lcd.println("Connecting...");
    Serial.println("STATE: Connecting...");
    delay(1000);
  }

  // When successfully connected to WiFi
  IPAddress ip = WiFi.localIP();
  IPAddress sm = WiFi.subnetMask();
  IPAddress gw = WiFi.gatewayIP();

  // Print WiFi info
  M5.Lcd.println("WiFi Connected!");
  Serial.println("STATE: WiFi Connected!");

  // Print WiFi info to
  M5.Lcd.println();
  M5.Lcd.print("IP:      ");
  M5.Lcd.println(ip);
  M5.Lcd.print("Subnet:  ");
  M5.Lcd.println(sm);
  M5.Lcd.print("Gateway: ");
  M5.Lcd.println(gw);
  M5.Lcd.println();

  // Print WiFi info to Serial
  //Serial.println();
  //Serial.print("IP:      ");
  //Serial.println(ip);
  //Serial.print("Subnet:  ");
  //Serial.println(sm);
  //Serial.print("Gateway: ");
  //Serial.println(gw);
  //Serial.println();

  // Get some information about the date/time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  batteryPower();
  delay(5000);

  M5.Lcd.clear();

  Serial.println("STATE: Starting loop");
}

void loop() {
  // Print the date/time
  M5.Lcd.setCursor(1,1);
  printLocalTime();

  // Print the battery level
  M5.Lcd.setCursor(250,1);
  batteryPower();

  // Set the location of the labels
  M5.Lcd.setCursor(23,215);
  M5.Lcd.printf(labelA);

  M5.Lcd.setCursor(142,215);
  M5.Lcd.printf(labelB);

  M5.Lcd.setCursor(235,215);
  M5.Lcd.printf(labelC);

  // What to do if each button is pressed
  if (M5.BtnA.wasPressed()) {
    Serial.println("STATE: ButtonA was pressed");
    slackStatusUpdater(statusA, emojiA);
    delay(3000);
    M5.Lcd.clear();
  }
  if (M5.BtnB.wasPressed()) {
    Serial.println("STATE: ButtonB was pressed");
    slackStatusUpdater(statusB, emojiB);
    delay(3000);
    M5.Lcd.clear();
  }
  if (M5.BtnC.wasPressed()) {
    Serial.println("STATE: ButtonC was pressed");
    slackStatusUpdater(statusC, emojiC);
    delay(3000);
    M5.Lcd.clear();
  }

  M5.update();
}
