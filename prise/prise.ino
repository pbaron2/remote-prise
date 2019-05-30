#include <ESP8266WiFi.h>

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Timezone.h>

#include <PubSubClient.h>


//#define DEBUG

#define PIN_CTRL      0
#define PIN_LED       1

#define WIFI_SSID     "UPC614C757"
#define WIFI_PASS     "tHbdk5cykK8m"

#define MQTT_SERVER   "ec2-18-222-208-169.us-east-2.compute.amazonaws.com"
#define MQTT_USER     "v0h0lU3JLt0ZHpyFAMtQ"
#define MQTT_PASS     ""



WiFiClient espClient;
PubSubClient client(espClient);

bool etatPrise = false;



void setup()
{
  #ifdef DEBUG
    Serial.begin(115200);
  #else
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
  #endif

  pinMode(PIN_CTRL, OUTPUT);
  digitalWrite(PIN_CTRL, !etatPrise);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  #ifdef DEBUG
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
  #endif
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    #ifdef DEBUG
      Serial.print(".");
    #endif
  }
  
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  #ifdef DEBUG
    Serial.println("\nStarted !");
  #else
    digitalWrite(PIN_LED, HIGH);
  #endif
}



void loop()
{
  if (!client.connected())
  {
    #ifndef DEBUG
      digitalWrite(PIN_LED, HIGH);
    #endif
    
    reconnect();
    
    #ifndef DEBUG
      digitalWrite(PIN_LED, HIGH);
    #endif
  }
  client.loop();


  digitalWrite(PIN_CTRL, !etatPrise);
}



void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    #ifdef DEBUG
      Serial.print("Attempting MQTT connection...");
    #endif
    
    // Attempt to connect
    if (client.connect("Prise", MQTT_USER, MQTT_PASS))
    {
      #ifdef DEBUG
        Serial.println("connected");
      #endif

      // resubscribe
      client.subscribe("v1/devices/me/rpc/request/+");
      
      client.publish("v1/devices/me/attributes", "{\"etatPrise\":\"false\"}");
    }
    else
    {
      #ifdef DEBUG
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      #endif
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void callback(char* topic, byte* payload, unsigned int length)
{
  #ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");

    for (int i=0;i<length;i++)
    {
      Serial.print((char)payload[i]);
    }
    Serial.print("\n\t");
  #endif

  String top = String((char*) topic);
  String str = String((char*) payload);

  if(str.substring(10, 20).equals("\"setValue\""))
  {
    #ifdef DEBUG
      Serial.println("methode=SET_VALUE");
    #endif
    
    if(str.substring(30, 34).equals("true"))
    {
      etatPrise = true;
      #ifdef DEBUG
        Serial.println("\t\tetatPrise=true");
      #endif
    }
    else if(str.substring(30, 35).equals("false"))
    {
      etatPrise = false;
      #ifdef DEBUG
        Serial.println("\t\tetatPrise=false");
      #endif
    }
  }
  else if(str.substring(10, 20).equals("\"getValue\""))
  {
    #ifdef DEBUG
      Serial.println("methode=GET_VALUE");
    #endif
  }

  #ifdef DEBUG
    Serial.println("Response");
  #endif
  client.publish(("v1/devices/me/rpc/response/" + top.substring(26, top.length())).c_str(), "");
    
  client.publish("v1/devices/me/attributes", ("{\"etatPrise\":" + String(etatPrise ? "true" : "false") + "}").c_str());
}



