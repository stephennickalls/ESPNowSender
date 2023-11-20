
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <DHT.h>
#include <esp_now.h>
#include <esp_sleep.h>
#include <WiFi.h>

#define DHTPIN 4   
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

//Recevier MAC Address
uint8_t broadcastAddress[] = {0xB0, 0xB2, 0x1C, 0xA6, 0x5F, 0x50};


// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char typeA[32];
  float temp;
  char typeB[32];
  float humidity;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  dht.begin();
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init,  register for callback to
  // get the status of trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}
 
void loop() {

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Set values to send
  strcpy(myData.typeA, "Temp reading");
  myData.temp = temp;
  strcpy(myData.typeB, "Humidity reading");
  myData.humidity = humidity;
  
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(2000);
}