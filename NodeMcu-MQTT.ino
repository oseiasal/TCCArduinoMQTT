#include "DallasTemperature.h"
#include "OneWire.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const char* ssid = "Enterprise";
const char* passwd = "12345678s";

String soil_str;
String temp_str;
char soil[50];
char temp[50];

boolean lcdDisplay = false;

//Variáveis e objetos globais
WiFiClient espClient;
PubSubClient client(espClient);

// Configura pinos digitais do ESP
#define D1    5
#define D2    4
#define D3    0
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1

#define DS18B20 2
OneWire ourWire(DS18B20);
DallasTemperature sensors(&ourWire);

LiquidCrystal_I2C lcd(0x27, 16, 2);


// Protótipos de funções
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void conectarWifi();
void getInfo();


void setup() {

  Wire.begin(D2, D1);
  sensors.begin();
  lcd.init();


  lcd.backlight();
  Serial.begin(9600);
  lcd.print("Iniciando...");
  Serial.println("Iniciando o sistema");

  conectarWifi();

  //configure to MQTT server
  client.setServer("mqtt.eclipse.org", 1883);
  client.setCallback(callback);

}

void loop() {
  client.loop();
  reconnect();
  delay(1000);

  sensors.requestTemperatures();
  lcd.clear();

}

//print any message received for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


  // Manipular o display
  if ((char)payload[0] == '0') {
    lcd.noBacklight();
  } if ((char)payload[0] == '1') {
    lcd.backlight();
  }


  if ((char)payload[0] == '2') {
    getInfo();
    }

}


void reconnect() {
  if (!client.connected()) {

    if (client.connect("mqtttcc")) {
      lcd.clear();
      lcd.print("MQTT ON");
      client.publish("online", "online");
      client.subscribe("status");
      client.subscribe("info_sensors");
      delay(2000);

    } else {

      lcd.clear();
      lcd.print("Erro mqtt");
      delay(5000);

    }

    delay(200);

    getInfo();



  }

}

void getInfo() {
  delay(100);

  float fsoil = -1 * 0.09765625 * analogRead(A0) + 100;
  float ftemp = sensors.getTempCByIndex(0);

  soil_str = String(fsoil); //converting ftemp (the float variable above) to a string
  soil_str.toCharArray(soil, soil_str.length() + 1); //packaging up the data to publish to mqtt whoa...

  temp_str = String(ftemp);
  temp_str.toCharArray(temp, temp_str.length() + 1);

  delay(1000);

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
  client.publish("tttccc", temp);

  Serial.println(soil);
  Serial.println(temp);
}

void conectarWifi() {

  delay(1000);
  lcd.clear();
  lcd.print("Wifi...");
  if (WiFi.status() == WL_CONNECTED) {
    delay(1000);
    lcd.print("Conectado");
    return;
  }
  WiFi.begin(ssid, passwd); // Conecta na rede WI-FI

  while (WiFi.status() != WL_CONNECTED)
  {

    delay(100);
    Serial.print(".");
    lcd.clear();
    lcd.print("Tentando conectar");
    lcd.setCursor(0, 1);
    lcd.print("ao WiFi");
  }

  Serial.println();
  Serial.print("Conectado com sucesso na rede ");
  Serial.print(ssid);
  Serial.print("");
  Serial.println("IP obtido: ");
  Serial.println(WiFi.localIP());

}
