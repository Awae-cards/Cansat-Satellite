#include "mpu6050.h"
#include <Adafruit_MPL3115A2.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
//#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>
//SoftwareSerial loraSerial(14, 12); // (rxPin (D5), txPin (D6), inverse_logic, buffer size);
String str;
HardwareSerial loraSerial(1);//comment when back
HardwareSerial ss(2);
static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;

int packageID = 0;

#define RST 4


TinyGPSPlus gps;
String Date;
String Time;
String Location;

Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();
int t=0;

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, float message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void appendFilechar(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

void setup() {
  delay(1000);
  
  pinMode(2, OUTPUT);  // D7 on ESP8266
  pinMode(12,OUTPUT);
  pinMode(RST,OUTPUT);
  led_off();
  
  // Open serial communications and wait for port to open:
  Serial.begin(115200);  // Serial communication to PC
  
	ss.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
	Serial.println(F("DeviceExample.ino"));
	Serial.println(F("A simple demonstration of TinyGPSPlus with an attached GPS module"));
	Serial.print(F("Testing TinyGPSPlus library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
	Serial.println(F("by Mikal Hart"));
	Serial.println();

  loraSerial.begin(57600,SERIAL_8N1, 14, 27);  // Serial communication to RN2483
  digitalWrite(RST,LOW);
  delay(200);
  digitalWrite(RST,HIGH);
  loraSerial.setTimeout(3000);
  lora_autobaud();
  
  led_on();
  delay(1000);
  led_off();

  Serial.println("Initing LoRa");
  
  //loraSerial.listen();
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  loraSerial.println("sys get ver");
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
  loraSerial.println("mac pause");
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
//  loraSerial.println("radio set bt 0.5");  // Uncomment if we want to use FSK 
//  wait_for_ok();
  
  loraSerial.println("radio set mod lora"); // Comment if we want to use FSK 
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
  loraSerial.println("radio set freq 869100000");
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
  loraSerial.println("radio set pwr 14");  //max power 14 dBm
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
  loraSerial.println("radio set sf sf6");
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
  loraSerial.println("radio set afcbw 41.7");
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  
  loraSerial.println("radio set rxbw 125");
  str = loraSerial.readStringUntil('\n');
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
  
  loraSerial.println("radio set cr 4/5");
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

  Serial.println("ready for lora");


  Init_mpu6050();
  Serial.println("Adafruit_MPL3115A2 test!");
    if(!SD.begin()){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);   
    listDir(SD, "/", 0);
    removeDir(SD, "/sensor");
    createDir(SD, "/sensor");
    writeFile(SD,"/sensor/Pressure.txt","Inches(Hg) ");
    writeFile(SD,"/sensor/Altiitude.txt","meters ");
    writeFile(SD,"/sensor/Temperature.txt","*C ");
    writeFile(SD,"/sensor/Gyroscope.txt","velocity");

    /*wifi
    */
    Serial.println();
}

void loop() {

  if (! baro.begin()) {
        Serial.println("Couldnt find sensor");
        return;
  }

    float pascals = baro.getPressure();
    Serial.print(pascals/3377); Serial.println(" Inches (Hg)");

    float altm = baro.getAltitude();
    Serial.print(altm); Serial.println(" meters");

    float tempC = baro.getTemperature();
    Serial.print(tempC); Serial.println("*C");


    appendFile(SD, "/sensor/Pressure.txt", pascals/3377);
    appendFilechar(SD, "/sensor/Pressure.txt", " ");
    appendFile(SD, "/sensor/Altitude.txt", altm);
    appendFilechar(SD, "/sensor/Altitude.txt", " ");
    appendFile(SD, "/sensor/Temperature.txt", tempC);
    appendFilechar(SD, "/sensor/Temperature.txt", " ");


  
	while (ss.available() > 0) {
		if (gps.encode(ss.read())) {
			displayInfo();
		}
	}

	if (millis() > 5000 && gps.charsProcessed() < 10) {
		Serial.println(F("No GPS detected: check wiring."));
		while(true);
	}
  delay(2000);
  Serial.println(Date);
  Serial.println(Time);
  Serial.println(Location);




  ReadMPU6050();
  // 串口绘图仪 可视化线加速度曲线
  // Serial.print("Acc_x:");
  // Serial.print(mpu6050_data.Acc_X);
  // Serial.print(",");
  // Serial.print("Acc_Y:");
  // Serial.print(mpu6050_data.Acc_Y);
  // Serial.print(",");
  // Serial.print("Acc_Z:");
  // Serial.println(mpu6050_data.Acc_Z);

  // 串口绘图仪 可视化角速度曲线
  Serial.print("Angle_velocity_R:");
  Serial.println(mpu6050_data.Angle_Velocity_R);
  Serial.print(",");
  Serial.print("Angle_velocity_P:");
  Serial.print(mpu6050_data.Angle_Velocity_P);
  Serial.print(",");
  Serial.print("Angle_velocity_Y:");
  Serial.println(mpu6050_data.Angle_Velocity_Y);
  
  float AR=mpu6050_data.Angle_Velocity_R;
  float AP=mpu6050_data.Angle_Velocity_P;
  float AY=mpu6050_data.Angle_Velocity_Y;



  delay(50);

  appendFilechar(SD, "/sensor/Gyroscope.txt", " ");
  appendFile(SD, "/sensor/Gyroscope.txt", mpu6050_data.Angle_Velocity_R);
  appendFilechar(SD, "/sensor/Gyroscope.txt", " ");
  appendFile(SD, "/sensor/Gyroscope.txt", mpu6050_data.Angle_Velocity_P);
  appendFilechar(SD, "/sensor/Gyroscope.txt", " ");
  appendFile(SD, "/sensor/Gyroscope.txt", mpu6050_data.Angle_Velocity_Y);
  appendFilechar(SD, "/sensor/Gyroscope.txt", "\n");

  

  
  
  led_on();


  
  char tempS[15];
  dtostrf(tempC, 3, 1, tempS);
  const char* tempInput=tempS;
  const char* head="Temperature: ";
  char temp[2*strlen(tempInput)+1];
  char header[2*strlen(head)+1];
  stringToHex(tempInput,temp);
  stringToHex(head,header);
  Serial.print("packageID = ");
  Serial.println(packageID);


  char pas[15];
  dtostrf(pascals/3377, 3, 2, pas);
  const char* pasInput=pas;
  head="pascals: ";
  char pa[2*strlen(pasInput)+1];
  header[2*strlen(head)+1];
  stringToHex(pasInput,pa);
  stringToHex(head,header);

  char altitude[15];
  dtostrf(altm, 4, 2, altitude);
  const char* altInput=altitude;
  head="altitude: ";
  char alt[2*strlen(altInput)+1];
  header[2*strlen(head)+1];
  stringToHex(altInput,alt);
  stringToHex(head,header);
  
  const char* TimeInput=Time.c_str();
  head="Time: ";
  char TimeS[2*strlen(TimeInput)+1];
  header[2*strlen(head)+1];
  stringToHex(TimeInput,TimeS);
  stringToHex(head,header);

  const char* DateInput=Date.c_str();
  head="Date: ";
  char DateS[2*strlen(DateInput)+1];
  header[2*strlen(head)+1];
  stringToHex(DateInput,DateS);
  stringToHex(head,header);

  const char* LocationInput=Location.c_str();
  head="Location: ";
  char LocationS[2*strlen(LocationInput)+1];
  header[2*strlen(head)+1];
  stringToHex(LocationInput,LocationS);
  stringToHex(head,header);
  loraSerial.print("radio tx ");
  loraSerial.println(String(temp)+String(pa)+String(alt)+String(TimeS)+String(DateS)+String(LocationS));
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);
  str = loraSerial.readStringUntil('\n');
  Serial.println(str);

  led_off();
  packageID = packageID + 1;

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

void led_on()
{
  digitalWrite(2, 1);
}

void led_off()
{
  digitalWrite(2, 0);
}
void stringToHex(const char* inputString, char* hexStringBuffer) {
  int len = strlen(inputString);
  for (int i = 0; i < len; i++) {
    // 将每个字符转换为其十六进制表示，并追加到 hexStringBuffer 中
    sprintf(hexStringBuffer + (i * 2), "%02X", inputString[i]);
  }
}



void displayInfo() {
	Serial.print(F("Location: ")); 
	if (gps.location.isValid()) {
		Serial.print(gps.location.lat(), 6);
		Serial.print(F(","));
		Serial.print(gps.location.lng(), 6);
    String lat=String(gps.location.lat(),6);
    String lng=String(gps.location.lng(),6);
    String pardon=",";
    Location=lat+pardon+lng;
	} else {
		Serial.print(F("INVALID"));
    Location="xxx.xxxxxx,xxx.xxxxxx";
	}

	Serial.print(F("  Date/Time: "));
	if (gps.date.isValid()) {
		Serial.print(gps.date.month());
		Serial.print(F("/"));
		Serial.print(gps.date.day());
		Serial.print(F("/"));
		Serial.print(gps.date.year());
    String block="/";
    Date=gps.date.month()+block+gps.date.day()+block+gps.date.year();
	} else {
		Serial.print(F("INVALID"));
    Date="xx/xx/xxxx";
	}

	Serial.print(F(" "));
	if (gps.time.isValid()) {
		if (gps.time.hour() < 10) Serial.print(F("0"));
		Serial.print(gps.time.hour());
		Serial.print(F(":"));
		if (gps.time.minute() < 10) Serial.print(F("0"));
		Serial.print(gps.time.minute());
		Serial.print(F(":"));
		if (gps.time.second() < 10) Serial.print(F("0"));
		Serial.print(gps.time.second());
		Serial.print(F("."));
		if (gps.time.centisecond() < 10) Serial.print(F("0"));
		Serial.print(gps.time.centisecond());
    String Hour1;
    String Minute1;
    String Second1;
    String Centisecond1;
    String say=":";
    if(gps.time.hour()<10){ Hour1="0";}else(Hour1="");
    if(gps.time.minute()<10){ Minute1="0";}else Minute1="";
    if(gps.time.second()<10){ Second1="0";}else Second1="";
    if(gps.time.centisecond()<10){Centisecond1="0";}else Centisecond1="";
    Time=Hour1+gps.time.hour()+say+Minute1+gps.time.minute()+say+Second1+gps.time.second()+say+Centisecond1+gps.time.centisecond();
	} else {
		Serial.print(F("INVALID"));
    Time=("xx:xx:xx:xx");
	}

	Serial.println();
}
