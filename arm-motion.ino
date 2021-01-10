#include "Wire.h"
#include "WiFi.h"
#include <PubSubClient.h>

int16_t rawAccX, rawAccY, rawAccZ, rawTemp, rawGyroX, rawGyroY, rawGyroZ;
float gyroXoffset, gyroYoffset, gyroZoffset;
float temp, accX, accY, accZ, gyroX, gyroY, gyroZ;
float angleGyroX, angleGyroY, angleGyroZ, angleAccX, angleAccY, angleAccZ;
float pitch, roll, angleZ;
float interval;
long preInterval;
float accCoef, gyroCoef;
long timer = 0;

int recno = 0;
char recno_str[8];
char angle[8];

// Change the credentials below, so your ESP8266 connects to your router
const char* ssid = "Dialog 4G DE7";
const char* password = "04q9M2Er";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more outputs)
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  Wire.begin();
  mpu6050_begin();

}

void loop() {

  if (!client.connected()) {
    reconnect();
  } else {
    
    //example publish
    client.publish("sprinterdev/armmotion-left/constat", "Connected");
    Serial.print(" connected ");
  }


  mpu6050_update();
  if (millis() - timer > 500) {
    //    Serial.print("pitch : ");
    //    Serial.println(pitch);
  }

  //  timeClient.update();
  //  formattedDate = timeClient.getFormattedTime();
  //  int str_len = formattedDate.length() + 1;
  //  char char_array[str_len];
  //  formattedDate.toCharArray(char_array, str_len);

  dtostrf( recno, 1, 2, recno_str);

  dtostrf( pitch, 1, 2, angle);
  client.publish("sprinterdev/arm-left/recno", recno_str);
  //    client.publish("sprinterDev/mpu6056-left/time", char_array);
  client.publish("sprinterdev/arm-left/angle", angle);


  Serial.print("pitch : ");
  Serial.println(pitch);
  Serial.print(recno);
  recno++;

  client.loop();
}


void mpu6050_begin() {

  //Activate the MPU-6050
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x19);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission

  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1a);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission

  //Configure the accelerometer (+/-8g)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1c);                                                    //Send the requested starting register
  Wire.write(0x00);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission
  //Configure the gyro (500dps full scale)
  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x1b);                                                    //Send the requested starting register
  Wire.write(0x08);                                                    //Set the requested starting register
  Wire.endTransmission();                                              //End the transmission

  Wire.beginTransmission(0x68);                                        //Start communicating with the MPU-6050
  Wire.write(0x6b);                                                    //Send the requested starting register
  Wire.write(0x01);                                                    //Set the requested starting register
  Wire.endTransmission();

  mpu6050_update();


  preInterval = millis();
}


void mpu6050_update() {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((int)0x68, 14);

  rawAccX = Wire.read() << 8 | Wire.read();
  rawAccY = Wire.read() << 8 | Wire.read();
  rawAccZ = Wire.read() << 8 | Wire.read();
  rawTemp = Wire.read() << 8 | Wire.read();
  rawGyroX = Wire.read() << 8 | Wire.read();
  rawGyroY = Wire.read() << 8 | Wire.read();
  rawGyroZ = Wire.read() << 8 | Wire.read();

  temp = (rawTemp + 12412.0) / 340.0;

  accX = ((float)rawAccX) / 16384.0;
  accY = ((float)rawAccY) / 16384.0;
  accZ = ((float)rawAccZ) / 16384.0;

  angleAccX = atan2(accY, sqrt(accZ * accZ + accX * accX)) * 360 / 2.0 / PI;
  angleAccY = atan2(accX, sqrt(accZ * accZ + accY * accY)) * 360 / -2.0 / PI;


  pitch = ((0.98 * (pitch + gyroX * interval)) + (0.02 * angleAccX));
  roll = (0.98 * (roll + gyroY * interval)) + (0.02 * angleAccY);

  preInterval = millis();


}
