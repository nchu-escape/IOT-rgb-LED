#include <ESP8266WiFi.h>
#include <aREST.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define red 14
#define green 12
#define blue 13

aREST rest = aREST();
// WiFi parameters
const char *ssid = "wifi_ssid";
const char *password = "wifi_pass";
// The port to listen for incoming TCP connections
#define LISTEN_PORT 80
// Create an instance of the server
WiFiServer server(LISTEN_PORT);
// Variables to be exposed to the API
int r = 0;
int g = 0;
int b = 0;
int breathing_light = 0;

void setup(void)
{
  Serial.begin(115200);
 
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  rest.variable("r", &r);
  rest.variable("g", &g);
  rest.variable("b", &b);
  rest.variable("breathing_light", &breathing_light);
  rest.set_id("60");         //need change
  rest.set_name("LED"); //need change
  rest.function("set_color", set_color);
  rest.function("set_breathing_light", set_breathing_light);
  WiFi.enableSTA(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  // Start the server
  server.begin();
  Serial.println("Server started");
  // Print the IP address
  Serial.println(WiFi.localIP());
}

int color_state = 0;
int color_g = 0;
int color_b = 0;
long unsigned int T=0;

void loop()
{
  // Handle REST calls
  WiFiClient client = server.available();
  
  if(breathing_light==1){
    analogWrite(red, 0);
    if(color_state==0 && millis()-T>30){
      color_g += 5;
      analogWrite(green, color_g);
      if(color_g == 255){
        color_state = 1;
      }
      T = millis();
    }
    if(color_state==1 && millis()-T>30){
      color_b += 5;
      analogWrite(blue, color_b);
      if(color_b == 255){
        color_state = 2;
      }
      T = millis();
    }
    if(color_state==2 && millis()-T>30){
      color_g -= 5;
      analogWrite(green, color_g);
      if(color_g == 0){
        color_state = 3;
      }
      T = millis();
    }
    if(color_state==3 && millis()-T>30){
      color_b -= 5;
      analogWrite(blue, color_b);
      if(color_b == 0){
        color_state = 0;
      }
      T = millis();
    }
  }
  
  if (!client)
    return;  
  while (!client.available())
    delay(1);
  rest.handle(client);
}

int const_r = 0;
int const_g = 0;
int const_b = 0;

int set_color(String command) {
  breathing_light = 0;
  int number = (int) strtol( &command[0], NULL, 16);
  // Split them up into r, g, b values
  r = number >> 16;
  g = number >> 8 & 0xFF;
  b = number & 0xFF;
  const_r = r;
  const_g = g;
  const_b = b;
  Serial.print(r); Serial.print('\t'); Serial.print(g); Serial.print('\t'); Serial.println(b);
  analogWrite(red, r);
  analogWrite(green, g);
  analogWrite(blue, b);
  return 1;
}

int set_breathing_light(String command){
  int state = command.toInt();
  breathing_light = state;
  T = millis();
  if(state==0){
    analogWrite(red, const_r);
    analogWrite(green, const_g);
    analogWrite(blue, const_b);
  }else{
    analogWrite(red, 0);
    analogWrite(green, 0);
    analogWrite(blue, 0);
  }
  return 1;
}
