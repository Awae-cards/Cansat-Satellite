#include <HardwareSerial.h> 
#include <WiFiManager.h>
#include <ThingSpeak.h>
#include <PubSubClient.h>
const char* ssid = "Xiaomi Civi 3";
const char* password = "67160277";
const char* mqttServer = "public.mqtthq.com";
const int mqttPort = 1883;
const char* mqttPath = "/mqtt";
const char* inTopic = "esp8266_control";
const char* outTopic = "esp8266_out";
WiFiClient espClient;
PubSubClient client(espClient);

HardwareSerial loraSerial(1); // (rxPin (D5), txPin (D6)); 
String str; 
#define RST 23


/*wifi part
*/
unsigned long channelID = 2431780; //your channel
const char * myWriteAPIKey = "4ITF4OD63IEOCNYF"; // your WRITE API key
const char* server = "api.thingspeak.com";
const int postingInterval = 1000; // post data every 20 seconds
/*wifi part
*/



int i=1;

void setup_wifi() {
 delay(10);
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
}
void mqttCallback(char* topic, byte* payload, unsigned int length) {
 Serial.print("Message received in topic: ");
 Serial.println(topic);
 
 String message = "";
 for (int i = 0; i < length; i++) {
 message += (char)payload[i];
 }
 Serial.print("Message payload: ");
 Serial.println(message);
 
 if (message == "off") {
 i=0;
 } else if (message == "on") {
 i=1;
 }
}
void reconnect() {
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 if (client.connect("ESP8266Client")) {
 Serial.println("connected");
 client.publish(outTopic, "Hello world, I'm ESP8266Client1");
 client.subscribe(inTopic);
 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 2 seconds");
 delay(2000);
 }
 }
}


 
void setup() { 
  //output LED pin 
  
  // Open serial communications and wait for port to open: 
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  
  pinMode(2,OUTPUT);
  pinMode(RST,OUTPUT);
  digitalWrite(2,0);
  
  Serial.begin(115200); 
  loraSerial.begin(57600,SERIAL_8N1,16,17); 
  digitalWrite(RST, LOW); // Resetting RN2483 by pulling RST pin low in 200 ms
  delay(200);
  digitalWrite(RST, HIGH);
  loraSerial.setTimeout(3000); 
  lora_autobaud(); 
  
  digitalWrite(2,1);
  delay(1000);
  digitalWrite(2,0);
  
 
  Serial.println("Initing LoRa"); 
  
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  loraSerial.println("sys get ver"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("mac pause"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
//  loraSerial.println("radio set bt 0.5"); 
//  wait_for_ok(); 
  
  loraSerial.println("radio set mod lora"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set freq 869100000"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set pwr 14"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set sf sf6"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set afcbw 41.7"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set rxbw 125");  // Receiver bandwidth can be adjusted here. Lower BW equals better link budget / SNR (less noise). 
  str = loraSerial.readStringUntil('\n');   // However, the system becomes more sensitive to frequency drift (due to temp) and PPM crystal inaccuracy. 
  Serial.println(str); 
  
//  loraSerial.println("radio set bitrate 50000"); 
//  wait_for_ok(); 
  
//  loraSerial.println("radio set fdev 25000"); 
//  wait_for_ok(); 
  
  loraSerial.println("radio set prlen 8"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set crc on"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set iqi off"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set cr 4/5"); // Maximum reliability is 4/8 ~ overhead ratio of 2.0 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set wdt 60000"); //disable for continuous reception 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set sync 12"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 
  
  loraSerial.println("radio set bw 125"); 
  str = loraSerial.readStringUntil('\n'); 
  Serial.println(str); 

} 
 
void loop() { 
   if (!client.connected()) {
    reconnect();
    }
    client.loop();
  

 Serial.println(i);
 delay(2000);
  
  if(i==1){

  
  Serial.println("waiting for a message"); 
  loraSerial.println("radio rx 0"); //wait for 60 seconds to receive 
  str = loraSerial.readStringUntil('\n'); 

  
      String hexString = str;
      String decodedString = ""; // Initialize an empty string to store the decoded characters
      for (int i = 0; i < hexString.length(); i += 2) {
        String hexPair = hexString.substring(i, i + 2);
       decodedString += char(strtoul(hexPair.c_str(), NULL, 16));
      }
  Serial.println(str); 
  
  
  if ( str.indexOf("ok") == 0 ) 
  { 
    str = String(""); 
    while(str=="") 
    { 
      str = loraSerial.readStringUntil('\n'); 
    } 
    if ( str.indexOf("radio_rx") == 0 )  //checking if data was reeived (equals radio_rx = <data>). indexOf returns position of "radio_rx" 
    { 
      Serial.println(str); //printing received data 
      String hexString = str;
      String decodedString = ""; // Initialize an empty string to store the decoded characters
      for (int i = 0; i < hexString.length(); i += 2) {
        String hexPair = hexString.substring(i, i + 2);
       decodedString += char(strtoul(hexPair.c_str(), NULL, 16));
      }
      Serial.println(decodedString);
      Serial.print("Temperature: ");
      Serial.println(decodedString.substring(5,9));
      String temp=decodedString.substring(5,9);
      float tempC=temp.toFloat();
      Serial.print("Pressure: ");
      Serial.println(decodedString.substring(9,13));
      String pascal=decodedString.substring(9,13);
      float pascals=pascal.toFloat();
      Serial.print("Altitude: ");
      Serial.println(decodedString.substring(13,18));
      String alt=decodedString.substring(13,18);
      float altm=alt.toFloat();
      Serial.print("Time: ");
      Serial.println(decodedString.substring(18,29));
      Serial.print("Date: ");
      Serial.println(decodedString.substring(29,37));
      Serial.print("Location: ");
      Serial.println(decodedString.substring(37,60));
    ThingSpeak.begin(espClient);
    if (espClient.connect(server, 80)) {
    long rssi = WiFi.RSSI();
    float test = 10;
    Serial.print("RSSI: ");
    Serial.println(rssi); 
    ThingSpeak.setField(4,rssi);
    ThingSpeak.setField(1,pascals);
    Serial.println(pascals);
    ThingSpeak.setField(2,altm);
    ThingSpeak.setField(3,tempC);
    ThingSpeak.setField(8,test);
    ThingSpeak.writeFields(channelID, myWriteAPIKey);
  }
  espClient.stop();
  delay(5*1000);
  }else{
    delay(1000);
  }



    else 
    { 
      Serial.println("Received nothing"); 
    } 
  } 
  else 
  { 
    Serial.println("radio not going into receive mode"); 
    delay(1000); 
  } 
} 
 
void lora_autobaud() 
{ 
  String response = ""; 
  while (response=="") 
  { 
    delay(1000); 
    loraSerial.write((byte)0x00); 
    loraSerial.write(0x55); 
    loraSerial.println(); 
    loraSerial.println("sys get ver"); 
    response = loraSerial.readStringUntil('\n'); 
  } 
} 
 
/* 
 * This function blocks until the word "ok\n" is received on the UART, 
 * or until a timeout of 3*5 seconds. 
 */ 
int wait_for_ok() 
{ 
  str = loraSerial.readStringUntil('\n'); 
  if ( str.indexOf("ok") == 0 ) { 
    return 1; 
  } 
  else return 0; 
} 
 
void toggle_led() 
{ 
  digitalWrite(13, 1); 
  delay(100); 
digitalWrite(13, 0); 
} 
void led_on() 
{ 
digitalWrite(2, 1); 
} 
void led_off() 
{ 
digitalWrite(2, 0); 
} 

