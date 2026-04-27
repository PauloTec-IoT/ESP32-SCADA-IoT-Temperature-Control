#include <WiFi.h>
#include <WiFiClientSecure.h>// ===== WIFI =====
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ===== FIREBASE =====
const char* host = "https://seu-projeto.firebaseio.com";
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ===== WIFI =====
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ===== FIREBASE =====
const char* host = "https://seu-projeto.firebaseio.com";
// PINOS

#define ONE_WIRE_BUS 4
#define RELE 18

// CONTROLE
float temp;
bool estadoRele = false;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiClientSecure client;

void setup() {
  Serial.begin(115200);

  pinMode(RELE, OUTPUT);
  digitalWrite(RELE, HIGH);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  client.setInsecure();
  sensors.begin();
}

void loop() {

  // ===== TEMPERATURA =====
  sensors.requestTemperatures();
  temp = sensors.getTempCByIndex(0);

  Serial.println(temp);

  // ===== ENVIA TEMPERATURA =====
  HTTPClient http;
  http.begin(client, String(host) + "/sensor/temperatura.json");
  http.addHeader("Content-Type", "application/json");
  http.PUT(String(temp));
  http.end();

  // ===== LÊ MODO =====
  String modo = "";
  http.begin(client, String(host) + "/controle/modo.json");
  if (http.GET() > 0) {
    modo = http.getString();
    modo.trim();
    modo.replace("\"", "");
  }
  http.end();

  // ===== LÊ COMANDO MANUAL =====
  String cmd = "";
  http.begin(client, String(host) + "/controle/rele.json");
  if (http.GET() > 0) {
    cmd = http.getString();
    cmd.trim();
  }
  http.end();

  // ===== LÓGICA =====
  if (modo == "auto") {

    // HISTERSE
    if (temp >= 31 && !estadoRele) {
      estadoRele = true;
    }
    if (temp <= 29 && estadoRele) {
      estadoRele = false;
    }

  } else {

    // MANUAL
    estadoRele = (cmd == "true");
  }

  // ===== APLICA NO RELÉ =====
  digitalWrite(RELE, estadoRele ? LOW : HIGH);

  // ===== ATUALIZA ESTADO NO FIREBASE =====
  http.begin(client, String(host) + "/controle/rele.json");
  http.addHeader("Content-Type", "application/json");
  http.PUT(estadoRele ? "true" : "false");
  http.end();

  delay(2000);
}