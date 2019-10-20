#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include "SoftwareSerial.h"
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Wire.h>

#define DS18B20 9

OneWire ourWire(DS18B20); //CONFIGURA UMA INSTÂNCIA ONEWIRE PARA SE COMUNICAR COM O SENSOR
DallasTemperature sensors(&ourWire);

char ssid[] = "Enterprise";   // your network SSID (name)
char pass[] = "12345678s";    // your network password
int status = WL_IDLE_STATUS;  // the Wifi radio's status

// Initialize the Ethernet client object
WiFiEspClient espClient;
PubSubClient client(espClient);
SoftwareSerial soft(2, 3); // RX, TX

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// Variables for printing temp and humidity

String soil_str;
String temp_str;
char soil[50];
char temp[50];

// Prototype
void reconnect();

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  lcd.print("Tentando");
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    lcd.clear();
    Serial.print((char)payload[i]);
    lcd.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {

  lcd.begin(16, 2);
  lcd.print("Init...");
  delay(2000);
  lcd.clear();

  // initialize serial for debugging
  Serial.begin(9600);
  soft.begin(115200);
  sensors.begin();

  // initialize ESP module
  WiFi.init(&soft);
  delay(2000); //necessário para o módulo iniciar e conectar

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    lcd.clear();
    Serial.println("WiFi shield not present");
    lcd.print("WiFi mod !present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    lcd.clear();
    lcd.print("Connecting Wifi");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
  lcd.clear();
  lcd.print("Conectado ao");
  lcd.setCursor(0, 1);
  lcd.print("Wifi");

  //configure to MQTT server
  client.setServer("mqtt.eclipse.org", 1883);
  client.setCallback(callback);
}



void loop() {
  client.loop();
  delay(2000);
  sensors.requestTemperatures();



  reconnect();
  delay(2000);

}




void reconnect() {
  if (!client.connected()) {

    client.connect("mqtttcc");

    client.publish("online", "online");

    client.subscribe("status");
    delay(200);

    float fsoil = -1 * 0.098 * analogRead(A0) + 100;
    float ftemp = sensors.getTempCByIndex(0);

    soil_str = String(fsoil); //converting ftemp (the float variable above) to a string
    soil_str.toCharArray(soil, soil_str.length() + 1); //packaging up the data to publish to mqtt whoa...

    temp_str = String(ftemp);
    temp_str.toCharArray(temp, temp_str.length() + 1);

    lcd.clear();
    lcd.print(soil);
        lcd.setCursor(5, 0);
    lcd.print("%");
    lcd.setCursor(0, 1);
    lcd.print(temp);
    lcd.setCursor(5, 1);
    lcd.print("C");

    char tempe[10];

    client.publish("soil", soil);
    client.publish("temperature", temp);

    Serial.println(soil);
    Serial.println(temp);



  }

}
