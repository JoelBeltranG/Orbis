#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

WiFiClientSecure esp32Client;
PubSubClient client(esp32Client);

const char* ssid = "INFINITUME99E";  //"INFINITUME99E" ; //"INFINITUM3FA3_2.4"; //"INFINITUMD19E_2.4";  //"CICESE-LaPaz";
const char* password = "uvXbM95uP6";  //"uvXbM95uP6" ;  // "5pQYbgFgdU"; //"KQ3rYFQet8"; //"redcicese";

const char* ssid_respaldo = "RUT240_6B84" ;
const char* password_respaldo = "u0LBd72Q";

const long interval = 300000;  // Intervalo de 5 segundos (5000 milisegundos)
uint64_t deep_sleep = 3600; // cuanto tiempo estara en deep sleep
// pines de relevadores
int rele2 = 25;
int rele3 = 4;
int rele4 = 35;

// Pines del ADC
#include <driver/adc.h>
#include "esp_adc_cal.h"

// Parámetros de calibración
#define DEFAULT_VREF 1100  // Valor de referencia del voltaje en mV (ajusta si es necesario)
#define CALIBRATION_FACTOR 1.18  // Ajusta este valor basado en la discrepancia observada
esp_adc_cal_characteristics_t *adc_chars;

const char *server ="b9e13cb483c3438f9caf0945f6329083.s1.eu.hivemq.cloud";
int port = 8883;

const char* mqttUsername = "esp32";
const char* mqttPassword = "Seismo-1";

unsigned long lastPublishTime = 0;  // Variable para almacenar el último tiempo de publicación

// Bandera para la conexión
bool usingBackupWiFi = false;

static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

int intententos_restart=0;


void wifiInit(){
  Serial.print("conectandose a ");
  Serial.println(ssid);

  WiFi.begin(ssid,password);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  int retry_count = 0;
  const int max_retries = 50;  // Set a maximum retry limit

  // Intentando conectarse a la red principal
  while (WiFi.status() != WL_CONNECTED && retry_count < max_retries){
    digitalWrite(GPIO_NUM_2,  HIGH);
    delay(100);
    digitalWrite(GPIO_NUM_2,  LOW);
    delay(100);
    retry_count++;  // Increment retry counter
    Serial.print(".");
  }
  
  Serial.println("\n");
  Serial.println(WiFi.status());

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConectado a WiFi principal");
    Serial.print("Direccion IP: ");
    Serial.println(WiFi.localIP());
    usingBackupWiFi = false;
  } else {
    Serial.println("\nError: No se pudo conectar a WiFi princilal. Intentando red de respaldo");
    WiFi.begin(ssid_respaldo,password_respaldo);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    retry_count = 0;
    while (WiFi.status() != WL_CONNECTED && retry_count < max_retries){
      digitalWrite(GPIO_NUM_2,  HIGH);
      delay(100);
      digitalWrite(GPIO_NUM_2,  LOW);
      delay(100);
      retry_count++;  // Increment retry counter
      Serial.print(".");
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConectado a WiFi de respaldo");
      Serial.print("Direccion IP: ");
      Serial.println(WiFi.localIP());
      usingBackupWiFi = true;
    }
    else{
      Serial.println("\nError: No se pudo conectar a WiFi de respaldo.");
    }

  }

}

void scanNetworks() {
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");
  
  if (n == 0) {
    Serial.println("No networks found");
  } else {
    // Bandera para verificar si se encontró la red deseada
    bool found = false;
    
    for (int i = 0; i < n; ++i) {
      // Verificar si la red actual es "RUT240_6B84"
      if (WiFi.SSID(i) == "RUT240_6B84") {
        Serial.println("Red WiFi 'RUT240_6B84' encontrada.");
        found = true;
        break; // Salir del bucle si se encontró la red
      }
    }
    
    // Si no se encontró la red específica
    if (!found) {
      Serial.println("Red WiFi 'RUT240_6B84' no encontrada.");
    }
  }
}


void scanNetworks_publish() {
  int n = WiFi.scanNetworks();
  Serial.println("Scan done");

  if (n == 0) {
    Serial.println("No networks found");
    client.publish("scan_wifi", "No networks found");  // Publicar que no se encontraron redes
  } else {
    // Bandera para verificar si se encontró la red deseada
    bool found = false;
    
    for (int i = 0; i < n; ++i) {
      // Verificar si la red actual es "RUT240_6B84"
      if (WiFi.SSID(i) == "RUT240_6B84") {
        // Construir el mensaje a publicar
        String mensaje = "Red WiFi 'RUT240_6B84' encontrada con RSSI: " + String(WiFi.RSSI(i));
        mensaje += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " Open" : " Encrypted";

        // Publicar el mensaje a través de MQTT
        client.publish("scan_wifi", mensaje.c_str());

        // Mostrar en el monitor serial
        Serial.println(mensaje);

        found = true;
        break; // Salir del bucle si se encontró la red
      }
    }
    // Si no se encontró la red específica
    if (!found) {
      Serial.println("Red WiFi 'RUT240_6B84' no encontrada.");
      client.publish("scan_wifi", "Red WiFi 'RUT240_6B84' no encontrada.");
    }
  }
}



void callback(char* topic, byte* payload, unsigned int length){
  Serial.println("Mensaje recibido");
  Serial.print("[");
  Serial.print(topic);
  Serial.println("] ");

  if (strcmp(topic, "prendido") == 0) {
    int estado = payload[0] - '0';
    digitalWrite(GPIO_NUM_2, estado);
    Serial.print("Estado del LED: ");
    Serial.println(digitalRead(GPIO_NUM_2));
  } 
  
  else if (strcmp(topic, "restart_esp32") == 0) {
    int estado = payload[0] - '0';
    if (estado== 1){
      Serial.println("Mensaje de reinicio recibido");
      Serial.println("Iniciando reinicio de ESP32");
      client.publish("comm", "Iniciando reinicio de ESP32");
      ESP.restart();
    }
  }
  else if (strcmp(topic, "restart_modem1") == 0) {
    int estado = payload[0] - '0';
    if (estado== 1){
      digitalWrite(rele2,LOW);
      delay(10000);
      digitalWrite(rele2,HIGH);
    }
  }
  
  else if (strcmp(topic, "restart_rasp") == 0) {
    int estado = payload[0] - '0';
    if (estado== 1){
      digitalWrite(rele3,LOW);
      delay(5000);
      digitalWrite(rele3,HIGH);
    }
  }


  else if (strcmp(topic, "restart_modem2") == 0) {
    int estado = payload[0] - '0';
    if (estado== 1){
      digitalWrite(rele4,LOW);
      delay(10000);
      digitalWrite(rele4,HIGH);
    }
  }
}

void reconnect(){
  client.setSocketTimeout(30);
  int retry_count = 0;
  const int max_retries = 3;
  while (!client.connected() && WiFi.status() == WL_CONNECTED && retry_count < max_retries){

    Serial.print("Intentando conectarse a MQTT...");
    if (client.connect("arduinoClientjbg", mqttUsername, mqttPassword)){
      Serial.println("Conectado");
      client.subscribe("prendido"); ///////////////////////////////////////////////
      client.subscribe("restart_esp32");
      client.subscribe("restart_modem");
      client.subscribe("restart_rasp");
    }
    else{
      Serial.print("Fallo, rc=");
      Serial.print(client.state());
      Serial.println(" Intentar de nuevo en 10 segundos");
      delay(10000);
      retry_count++;
    }
  }
}

float readBatteryVoltage() {
  uint32_t raw = adc1_get_raw(ADC1_CHANNEL_6);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(raw, adc_chars);
  float actualVoltage = (voltage / 1000.0) * 4 * CALIBRATION_FACTOR; // Ajusta por el divisor de voltaje y calibración
  return actualVoltage;
}


void setup() {
  pinMode(GPIO_NUM_2, OUTPUT);
  // Initialize serial communication at a baud rate of 115200
  Serial.begin(115200);

  //Inicializar los pines de relay
  pinMode(rele2, OUTPUT);
  pinMode(rele3, OUTPUT);

  digitalWrite(rele2, HIGH);
  digitalWrite(rele3, HIGH);

  WiFi.mode(WIFI_STA);
  scanNetworks();  // Add this to scan for available networks


  wifiInit();

  esp32Client.setCACert(root_ca);
  client.setServer(server, port);
  client.setCallback(callback);

  // contador de reset
  intententos_restart=0;

  //Definir pines digitalizadores
  digitalWrite(rele2, HIGH);
  digitalWrite(rele3, HIGH);

  // Print a message when the setup is running
  Serial.println("Hi from setup");
  
  //medicion de voltage
  // Inicializar el ADC con características de calibración
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_12); // GPIO 34 = Canal 6
  adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_12, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);

}

void loop() {
  if (WiFi.status() != WL_CONNECTED || usingBackupWiFi == true){
    client.disconnect();
    Serial.println("WiFi desconectado. Intentando reconectar...");
    wifiInit();
  }

  
  if (!client.connected()){
    reconnect();
    intententos_restart++;
  } else {
      intententos_restart = 0;  // Reiniciar el contador si la conexión es exitosa
    }

  if (intententos_restart>=3){
    ESP.restart();
  }

  client.loop();
  

  
  // Comprobar si han pasado 5 segundos
  unsigned long currentTime = millis();
  if (currentTime - lastPublishTime >= interval) {
    lastPublishTime = currentTime;  // Actualizar el tiempo de la última publicación

    // Leer el estado actual del LED (pin 2)
    int estadoLED = digitalRead(2);  // Obtener el estado actual del LED (HIGH o LOW)
    // Publicar el estado actual del LED
    char mensaje[50];
    sprintf(mensaje, "Estado del LED: %d", estadoLED);  // Crear el mensaje a publicar
    client.publish("comm", mensaje); 
    delay(2000);


    //Se define y Publica el wifi al que se esta conectado y se publica
    String currentSSID = WiFi.SSID();  // Obtener el SSID actual de la red WiFi
    Serial.print("Publicando nombre de la red: ");
    Serial.println(currentSSID);

    char networkMessage[100];
    sprintf(networkMessage, "WiFi: %s", currentSSID.c_str());
    client.publish("network_status", networkMessage);  // Publicar el nombre de la red WiFi


    delay(2000);
    //Se imprime y se publica las redes disponibles
    scanNetworks_publish();
    
    // Publicar el voltaje
    //medicion de voltage
    float batteryVoltage = readBatteryVoltage();
    Serial.print("Voltaje de la bateria: ");
    Serial.println(batteryVoltage);

    char mensaje_volt[50];
    sprintf(mensaje_volt, "volt: %f", batteryVoltage);  // Crear el mensaje a publicar
    client.publish("volt", mensaje_volt);
    

    /*
    // Reiniciar si el voltage es bajo
    if (batteryVoltage<=8){

      // Configurar el tiempo de deep sleep
        esp_sleep_enable_timer_wakeup(deep_sleep * 1000000ULL); //10 segundos
        
        // Entrar en modo deep sleep
        esp_deep_sleep_start();
      
    }
    */

  }
  
  
}

