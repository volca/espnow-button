#include "ESP32_NOW.h"
#include "WiFi.h"
#include "BTHome.h"
#include <esp_mac.h>  // For the MAC2STR and MACSTR macros

/* Definitions */

#define ESPNOW_WIFI_CHANNEL 1

/* Classes */

// Creating a new class that inherits from the ESP_NOW_Peer class is required.

class ESP_NOW_Broadcast_Peer : public ESP_NOW_Peer {
public:
  // Constructor of the class using the broadcast address
  ESP_NOW_Broadcast_Peer(uint8_t channel, wifi_interface_t iface, const uint8_t *lmk) : ESP_NOW_Peer(ESP_NOW.BROADCAST_ADDR, channel, iface, lmk) {}

  // Destructor of the class
  ~ESP_NOW_Broadcast_Peer() {
    remove();
  }

  // Function to properly initialize the ESP-NOW and register the broadcast peer
  bool begin() {
    if (!ESP_NOW.begin() || !add()) {
      log_e("Failed to initialize ESP-NOW or register the broadcast peer");
      return false;
    }
    return true;
  }

  // Function to send a message to all devices within the network
  bool send_message(const uint8_t *data, size_t len) {
    if (!send(data, len)) {
      log_e("Failed to broadcast message");
      return false;
    }
    return true;
  }
};

/* Global Variables */

uint32_t msg_count = 0;
BTHome bthome;

// Create a broadcast peer object
ESP_NOW_Broadcast_Peer broadcast_peer(ESPNOW_WIFI_CHANNEL, WIFI_IF_STA, NULL);

/* Main */

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize the Wi-Fi module
  WiFi.mode(WIFI_STA);
  WiFi.setChannel(ESPNOW_WIFI_CHANNEL);
  while (!WiFi.STA.started()) {
    delay(100);
  }

  Serial.println("ESP-NOW Example - Broadcast Master");
  Serial.println("Wi-Fi parameters:");
  Serial.println("  Mode: STA");
  Serial.println("  MAC Address: " + WiFi.macAddress());
  Serial.printf("  Channel: %d\n", ESPNOW_WIFI_CHANNEL);

  // Register the broadcast peer
  if (!broadcast_peer.begin()) {
    Serial.println("Failed to initialize broadcast peer");
    Serial.println("Rebooting in 5 seconds...");
    delay(5000);
    ESP.restart();
  }

  bthome.begin(false, "", false);
  Serial.println("Setup complete. Broadcasting messages every 5 seconds.");
}

void loop() {
  // Broadcast a message to all devices within the network
  bthome.resetMeasurement();
  /*
  bthome.addMeasurement(ID_ILLUMINANCE, 50.81f);//4 bytes
  bthome.addMeasurement(ID_CO2, (uint64_t)1208);//3
  bthome.addMeasurement(ID_TVOC, (uint64_t)350);//3
  */
  bthome.addMeasurement_state(EVENT_BUTTON, EVENT_BUTTON_PRESS);//2 button press
  std::string str = bthome.buildPacket();
  const uint8_t *data = reinterpret_cast<const uint8_t *>(str.data());

  Serial.printf("Broadcasting message length %d\n", str.length());

  if (!broadcast_peer.send_message((uint8_t *)data, str.length())) {
    Serial.println("Failed to broadcast message");
  }

  delay(5000);
}
