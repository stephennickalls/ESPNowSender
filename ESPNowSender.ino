
#include <esp_now.h>
#include <esp_sleep.h>
#include <DHT.h>
#include <WiFi.h>

#define DHTPIN 4 
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temp;
  float humid;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

const uint64_t sleepInterval = 200000000; // 10 minutes in microseconds

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println("Stop broadcasting, going to deep sleep...");
  esp_sleep_enable_timer_wakeup(sleepInterval);
  esp_deep_sleep_start();
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

  // get the status of Trasnmitted packet
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
  float humid = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(humid) || isnan(temp)) {
      Serial.println("Failed to read from DHT sensor!");
      // Go to deep sleep again
      // esp_sleep_enable_timer_wakeup(sleepInterval);
      // esp_deep_sleep_start();
  }
  // Set values to send
  myData.temp = temp;
  myData.humid = humid;
  
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