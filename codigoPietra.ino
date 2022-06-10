//Projeto de Controle de Temperatura e Umidade
//Alunos: Lucas Henrique da Cunha e Pietra Bessani Soares

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdlib.h>
#include <DHT.h>

#define DHTPIN 32 // o sensor dht11 foi conectado ao pino D12
#define DHTTYPE DHT11 //Define o tipo de sensor DHT
DHT dht(DHTPIN,DHTTYPE);

//Definição dos topicos
#define TOPICO_PUBLISH_TEMPERATURA   "ESP32/TEMP/TOPICO"
#define TOPICO_PUBLISH_UMIDADE       "ESP32/UMI/TOPICO"

#define LED1 4                        //pino 4
#define LED2 15                       //pino 15

//Dados para conectar no Wi-Fi
const char* ssid = "AB";  
const char* password = "albes111";

const char* mqtt_server = "pietrabsifsp.duckdns.org"; //aqui pode ser o IP do MQTT ou uma URL
const char* mqtt_username = "admin"; //Usuario criado no broker 
const char* mqtt_password = "albes111"; //senha do usuario
const int mqtt_port = 1883; // define a porta

int ledTemp = 0;
int ledUmi = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void init_wifi(){ //inicia a função do wifi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  reconnect_wifi();
}

void init_mqtt(){
client.setServer(mqtt_server, mqtt_port);
client.setCallback(callback);
}

void reconnect_wifi(){ //Função para conectar o Wi-Fi
  if (WiFi.status() == WL_CONNECTED)
        return;
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {//Loop que mostra a tentativa de se conectar no Wi-Fi
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  String messageUmi;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  if(String (topic) == "ESP/TEMP"){
    Serial.print("TEMP: ");
    ledTemp = messageTemp.toInt();
    Serial.print(ledTemp);
  }

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageUmi += (char)message[i];
  }
  if(String (topic) == "ESP/UMID"){
    Serial.print("UMI: ");
    ledUmi = messageUmi.toInt();
    Serial.print(ledUmi);
  }
  Serial.println();
}

void reconnect_mqtt() { //Função para conectar o mqtt
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("ESP/TEMP");
      client.subscribe("ESP/UMID");
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(1000);
    }
  }
}

void verificaConexao (){
  if(!client.connected()){
    reconnect_mqtt();
  }
  reconnect_wifi();
}

float leitura_temperatura(){
  float t = dht.readTemperature();
  float result;

  if(!(isnan(t))){
    result = t;
  }
  else
  result = 0;

  Serial.print(result);
  Serial.println(" ºC");
  return result;
}

float leitura_umidade(){
  float h = dht.readHumidity();
  float result;

  if(!(isnan(h))){
    result = h;
  }else{
  result = 0;
}
Serial.print(result);
Serial.print("%");
return result;
}

void setup(){
  Serial.begin(9600);
  dht.begin();
  init_wifi(); //Inicializa o Wi_Fi
  init_mqtt(); //Inicializa o MQTT
  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
}

void loop(){
  //declaração de variaveis locais
  char temperatura_str[10] = {0};
  char umidade_str[10] = {0};

  verificaConexao();
  client.loop();

  //Preenchimento da string
  //sprintf(temperatura_str,"%.2f", leitura_temperatura());
  //sprintf(umidade_str, "%.2f", leitura_umidade());
    dtostrf(dht.readTemperature(), 1, 2,temperatura_str);
    dtostrf(dht.readHumidity(), 1, 2,umidade_str);

  //Envio dos dados atraves de MQTT
  client.publish("TOPICO_PUBLISH_TEMPERATURA", temperatura_str);
  client.publish("TOPICO_PUBLISH_UMIDADE", umidade_str);
  if (ledTemp == 1){
    digitalWrite(LED1,HIGH);
    Serial.print("LED LIGADO");
  }else{
    digitalWrite(LED1,LOW);
    Serial.print("LED DESLIGADO");
  }
  if (ledUmi == 1){
    digitalWrite(LED2,HIGH);
    Serial.print("LED LIGADO");
  }else{
    digitalWrite(LED2,LOW);
    Serial.print("LED DESLIGADO");
  }
  delay(100);
}
