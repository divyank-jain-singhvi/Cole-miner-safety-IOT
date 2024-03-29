#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TinyGPSPlus.h>
#include <Arduino.h>
#include <WiFi.h>
#include <ESP_Mail_Client.h>
#include <WiFi.h>
#include "secrets.h"
#include "ThingSpeak.h"

#define DHTPIN 7 // dht11 pin 7
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define DHTTYPE    DHT11 
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#if defined(ESP32)
#endif

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465

#define AUTHOR_EMAIL ""
#define AUTHOR_PASSWORD ""
#define RECIPIENT_EMAIL ""

SMTPSession smtp;

void smtpCallback(SMTP_Status status);
void sendingMail(String msg,String latitude,String longitude);

String latitude;
String longitude;
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
uint32_t delayMS;
const int sensorPin = A0;
int flag=0;
int buzz=6;
const int buttonPin=9;
int buttonState = 0;

char ssid[] = SECRET_SSID;   
char pass[] = SECRET_PASS;   
int keyIndex = 0;
WiFiClient  client;

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
int number1 = 0;
int number2 = 0;
int number3 = 0;

const char *gpsStream =
  "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
  "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
  "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
  "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
  "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
  "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";
TinyGPSPlus gps;
sensors_event_t event;
// sensor_t sensor;
void setup() {
  
  Serial.begin(9600);
  Serial1.begin(115200);
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
   Serial.print("Connecting to Wi-Fi");
   while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  Serial.println(F("starting our program"));
  Serial1.println(F("Start serial 2"));
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  delayMS = sensor.min_delay / 1000;
  while (*gpsStream)
    if (gps.encode(*gpsStream++))
      displayInfo();
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }


    display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,28);
  display.println("WELCOME");
  display.display();
  delay(2000);
  display.clearDisplay();

  // Display Inverted Text
  // display.setTextColor(WHITE,BLACK); // 'inverted' text
  display.setCursor(0,28);
  display.println("Gas level");
  display.display();
  delay(2000);
  display.clearDisplay();
  pinMode(buzz,OUTPUT);
  pinMode(buttonPin, INPUT);
  

 
}

void loop() {
  
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);     
    } 
    Serial.println("\nConnected.");
  }
  ThingSpeak.setField(1, number1);
  ThingSpeak.setField(2, number2);
  ThingSpeak.setField(3, number3);
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

  delay(delayMS);
  display.setCursor(0,0);
  display.setTextSize(2);
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Serial.print("Temperature: ");
    Serial.println(event.temperature);
    // Serial.println("°C");
    display.println(event.temperature);
    number1=event.temperature;
  }
  if (event.temperature > 60){
    digitalWrite(buzz,HIGH);
    if (flag==0){
      latitude = (gps.location.lat());
      longitude = (gps.location.lng());
      sendingMail("Temperature is high , Danger Alert!!",latitude,longitude);
    }
    // delay(2000);
  }
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    Serial.print("Humidity: ");
    Serial.println(event.relative_humidity);
    // Serial.println("percentage");
    display.println(event.relative_humidity);
    number2=event.relative_humidity;
  }
  if (event.relative_humidity > 80){ // humidity of a average mines is 80% humidity 
    digitalWrite(buzz,HIGH);
    if (flag==0){
      latitude = (gps.location.lat());
      longitude = (gps.location.lng());
      sendingMail("Condition of humidity is heigh",latitude,longitude);
    }
    // delay(2000);

  }

  if (isnan(analogRead(sensorPin))) {
    Serial.println(F("Error reading Gas!"));
    
  }
  else{
    Serial.print("Analog output: ");
    Serial.println(analogRead(sensorPin)); 
    display.println(analogRead(sensorPin));
    number3=analogRead(sensorPin);

    displayInfo();
  }    
  if (analogRead(sensorPin) > 10000){ 
    digitalWrite(buzz,HIGH);
    if (flag==0){
      latitude = (gps.location.lat());
      longitude = (gps.location.lng());
    sendingMail("Condition of gas is heigh",latitude,longitude);
      }
    // delay(2000);

  }  
  if (event.temperature < 60){ // temperature of a average mines is 60 degree 
    digitalWrite(buzz,LOW);
    flag=0;
  
    // delay(2000);


  }  
  if (event.relative_humidity < 80){ // humidity of a average mines is 50% humidity 
    digitalWrite(buzz,LOW);
    flag=0;
   
    // delay(2000);

  }  
  if (analogRead(sensorPin) < 10000){ // humidity of a average mines is 50% humidity 
    digitalWrite(buzz,LOW); 
    flag=0;
    // delay(2000);

  }  
  display.display();
  delay(1000);
  display.clearDisplay();
}

void displayInfo(){
  Serial.print(F("Location: ")); 
  if (gps.location.isValid()){
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.println(gps.location.lng(), 6);
  }
  else{
    Serial.print(F("INVALID"));
  }
  Serial.println();
}

void smtpCallback(SMTP_Status status){
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()){

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      SMTP_Result result = smtp.sendingResult.getItem(i);

      
      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    smtp.sendingResult.clear();
  }
}

void sendingMail(String msg,String latitude,String longitude){



  MailClient.networkReconnect(true);
  smtp.debug(1);
  smtp.callback(smtpCallback);
  Session_Config config;
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("Coal mining team");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = F("Rescue operation");
  message.addRecipient(F("Rescue Team"), RECIPIENT_EMAIL);
   
  //Send raw text message
  String textMsg = msg+" latitude:" +latitude+" longitude:"+longitude;
  message.text.content = textMsg.c_str();
  message.text.charSet = "us-ascii";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


  /* Connect to the server */
  if (!smtp.connect(&config)){
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()){
    Serial.println("\nNot yet logged in.");
  }
  else{
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    flag=1;
}
