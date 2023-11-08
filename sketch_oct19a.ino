#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

const char* ssid = "iPhone de Arilson";
const char* password = "12345678";
const char* url = "https://clima-f4d5.onrender.com";

unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

// Configurações MQTT
const char* mqttServer = "maqiatto.com";
const int mqttPort = 1883;
const char* mqttUser = "arilsontere18@hotmail.com";
const char* mqttPassword = "@Ri142008";

WebServer server(80);
DHT dht(22, DHT11);
WiFiClient espClient;
PubSubClient client(espClient);

void setup(void) {

  Serial.begin(115200);
  dht.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Conectado em: ");
  Serial.println(ssid);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  client.setServer(mqttServer, mqttPort);
  // client.setCallback(callback);
}

void loop() {

  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      http.begin(url);

      http.addHeader("Content-Type", "application/json");

      // Crie o objeto JSON que você deseja enviar
      // Substitua isso pelo seu próprio objeto JSON, se necessário
      StaticJsonDocument<200> jsonDocument;
      jsonDocument["data"] = "2023-11-03";
      jsonDocument["temperatura"] = readDHTTemperature();
      jsonDocument["umidade"] = readDHTHumidity();
      String jsonString;
      serializeJson(jsonDocument, jsonString);

      // Realize a solicitação POST
      int httpResponseCode = http.POST(jsonString);

      Verifique a resposta do servidor
      if (httpResponseCode > 0) {
        Serial.print("Código de Resposta: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Erro na solicitação - Código de Resposta: ");
        Serial.println(httpResponseCode);
      }

      // Encerre a solicitação
      http.end();
    } else {
      Serial.println("Wi-Fi Desconectado");
    }
    lastTime = millis();
  }

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

    String temp = String(temperature) + "°C";
    String umidade = String(humidity) + "%";

    // Publique a mensagem no tópico MQTT desejado
    client.publish("arilsontere18@hotmail.com/temp", temp.c_str());
    client.publish("arilsontere18@hotmail.com/umidade", umidade.c_str());
    delay(5000); // Aguarde um intervalo de tempo antes de enviar a próxima leitura
  
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Conectando ao servidor MQTT...");
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Conectado ao servidor MQTT");
      client.subscribe("seu/topico");  // Subscreva aos tópicos MQTT desejados
    } else {
      Serial.print("Falha na conexão - Error code: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

float readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  } else {
    Serial.println(t);
    return t;
  }
}

float readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return -1;
  } else {
    Serial.println(h);
    return h;
  }
}