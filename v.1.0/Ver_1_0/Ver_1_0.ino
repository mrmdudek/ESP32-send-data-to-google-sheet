#include <WiFiClientSecure.h>
#include "string.h"
#include <OneWire.h>
#include <DallasTemperature.h>

const int oneWireBus = 22; 

    // Wifi and google script parameters
    const char* ssid     = "===YOUR_SSID_HERE==";     // your network SSID (name of wifi network)
    const char* password = "==YOUR_PASSWORD_HERE=="; // your network password
    char *server = "www.script.google.com";  // Server URL
    char *GScriptId = "==YOUR_GS_HERE==";
    const int httpsPort = 443;
    
    // set time between measurements in seconds. Range 4 - 32,767 seconds.
    long sec_delay = 258;// DS18b20 read + send data =~ 2s

    // DS18b20 define
    OneWire oneWire(oneWireBus);
    DallasTemperature sensors(&oneWire);
    uint8_t numberOfDevices;// Number of temperature devices found
    DeviceAddress tempDeviceAddress;// We'll use this variable to store a found device address
    float DsTempC;// variable contain temperature data when single device on line

    WiFiClientSecure client;

void setup() 
{
    //Initialize serial and wait for port to open:
    Serial.begin(115200);
    delay(100);
    
    connect_wifi();
    sensors.begin();
    DS_look_for_devices();// search devices on one wire line
}


void loop() {
    connect_host();
    String dane = String("tDS=") + DS_read_temperature_from_single_device();
    send_data(dane);
    long ms_delay = sec_delay*1000;
    delay(ms_delay);
}

void connect_wifi(void)
{
    // wifi connect
    Serial.print("Connecting to wifi: ");
    Serial.println(ssid);
    Serial.flush();
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// function connect_host
void connect_host(void)
{
    Serial.println("\nStarting connection to server...");
    if (!client.connect(server, httpsPort))
    {
      Serial.println("Connection failed!");
    }
    else 
    {
      Serial.println("Connected to server!");
    }
}

void send_data (String Data)
{
    // Make a HTTP request:
    String Request = String("GET ") + "/macros/s/" + GScriptId + "/exec?" + Data + " HTTP/1.1\r\n" + "Host: script.google.com\r\n" + "User-Agent: ESP8266\r\n" + "Connection: close\r\n" + "\r\n\r\n";
    
    client.println(Request);
    
    while (client.connected()) 
    {
      String line = client.readStringUntil('\n');
      if (line == "\r") 
      {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) 
    {
      char c = client.read();
      Serial.write(c);
    }
    client.stop();
}

void DS_look_for_devices(void)
{
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");
}

float DS_read_temperature_from_single_device(void)
{
  sensors.requestTemperatures();
  delay(750);
  if (sensors.getAddress(tempDeviceAddress, 0)) // Search the wire for address
  {
    return sensors.getTempC(tempDeviceAddress);
  }
  else return -1.1;
}
