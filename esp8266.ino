#include <Ultrasonic.h>
#include <ESP8266WiFi.h>

#define echoPin1 D7
#define trigPin1 D6
#define echoPin2 D1
#define trigPin2 D2

const char* host = "api.thingspeak.com";
String path = "/update?key=YOUR-THINGSPEAK-APIKEY-HERE";

const char* ssid = "YOUR-WIFINETWORK-SSID-HERE";
const char* password = "PASSWORD-HERE";
char nivelAtualStr[7];
float tamLixeira;
float nivelAnterior=0, nivelAtual=0;

Ultrasonic ultrasonic1(trigPin1, echoPin1);
Ultrasonic ultrasonic2(trigPin2, echoPin2);

void setup() {

  Serial.begin(9600);
  Serial.print("Lendo dados do sensor...\n");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  tamLixeira = calibraSensores();

}

float calculaDiferencaPerc(float dist1, float dist2) {

  float diferenca_perc=100;

  if (dist1 > dist2) {
    diferenca_perc = (((dist1-dist2)/dist1)*100);
  } else if (dist2 > dist1) {
    diferenca_perc = (((dist2-dist1)/dist2)*100);
  }
  Serial.print("diferenca percentual: ");
  Serial.print(diferenca_perc);
  Serial.print("\n");
  return diferenca_perc;
}

float calibraSensores() {

  Serial.print("----- Calibragem ------\n");
  float dist1, dist2, diferenca_perc;

  do {

    dist1 = ultrasonic1.distanceRead(CM);
    dist2 = ultrasonic2.distanceRead(CM);
    diferenca_perc = calculaDiferencaPerc(dist1, dist2);
    delay(1000);

  } while (diferenca_perc > float(5) || dist1 == dist2);
  Serial.print("Tamanho da lixeira: ");
  Serial.print(tamLixeira);
  Serial.print("\n");
  Serial.print("----- End Calibragem ------\n");
  return (dist1 > dist2) ? dist2 : dist1;

}

float calculaDistancia() {

  float dist1, dist2, diferenca_perc;

  do {
    dist1 = ultrasonic1.distanceRead(CM);
    dist2 = ultrasonic2.distanceRead(CM);
    diferenca_perc = calculaDiferencaPerc(dist1, dist2);
    delay(200);
  } while (diferenca_perc <= float(5));
  Serial.print("diferenca percentual: ");
  Serial.print((dist1+dist2)/2);
  Serial.print("\n");
  return (dist1+dist2)/2;
}

void loop() {

  float distanciaMedia = calculaDistancia();

  if (distanciaMedia <= tamLixeira) {
      nivelAtual = (((tamLixeira-distanciaMedia)/tamLixeira)*100);
      if (nivelAnterior == 0)
        nivelAnterior = nivelAtual;
  }

  Serial.print("distanciaMedia: ");
  Serial.print(distanciaMedia);
  Serial.print("\n");

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  dtostrf(nivelAtual, 3, 2, nivelAtualStr);
  client.print(String("GET ") + path + "&field1=1&field2="+ nivelAtual + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: keep-alive\r\n\r\n");

  delay(10000);

}
