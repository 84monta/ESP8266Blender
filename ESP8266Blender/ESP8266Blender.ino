#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *MYESP_SSID = "MYESP";
const char *MYESP_PASSWD = "12345678ABC"; // more 8char

ESP8266WebServer server( 80 );
IPAddress ip( 192, 168, 100, 1 );
IPAddress subnet( 255, 255, 255, 0 );

#define LOOP            10
#define THRESHOLD       15000

const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
String RetStr = "TOP";

//#define DEBUG

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  //Setup Access Point
  APInit();

  //Setup Jyro Sensor(MPU-6050)
  MPUInit();

  //Setup WebServer
  WebSrvInit();
}

void loop() {
  // put your main code here, to run repeatedly:

  UpdateValues();
  server.handleClient();
  //Serial.println(AcX);
}

void APInit()
{
  //disable access point function
  //WIFI_AP      Accesspoint
  //WIFI_STA     Cilent
  //WIFI_AP_STA  Accesspoint and Cilent
  WiFi.mode(WIFI_AP);
  WiFi.softAP(MYESP_SSID, MYESP_PASSWD);
  WiFi.softAPConfig(ip, ip, subnet);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

void MPUInit()
{
  //usage of Wire.begin for ESP8266
  //  Wire.begin(SDA,SCL)
  Wire.begin(5, 4);

  Wire.setClock(400000L);
  //Initialize MPU-6050
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write((byte)0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
}

void WebSrvInit() {
  server.on("/direction", []() {
 
    if ( AcZ > THRESHOLD) {
      RetStr = "TOP";
    } else if ( AcZ < -THRESHOLD) {
      RetStr = "BOTTOM";
    } else if (AcX > THRESHOLD) {
      RetStr = "FRONT";
    } else if (AcX < -THRESHOLD) {
      RetStr = "BACK";
    } else if (AcY < -THRESHOLD) {
      RetStr = "LEFT";
    } else if (AcY > THRESHOLD) {
      RetStr = "RIGHT";
    }

    server.send(
      200,
      "text/html",
      RetStr   );
  });

  server.on("/value", []() {
    server.send(
      200,
      "text/html",
      String(AcX)  + "," + String(AcY)  + "," + String(AcZ) + "," + String(GyX) + "," + String(GyY)  + "," +  String(GyZ)   );
  });

  server.on("/", []() {
    server.send(
      200,
      "text/html",
      "<h1>ESP Working</h1>"  );
  });

  server.begin();
}


void UpdateValues()
{
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true); // request a total of 14 registers
//  AcX = AcX = AcZ = Tmp = GyX = GyY = GyZ = 0;
//  for (int i = 0; i < LOOP; i++) {
//    AcX += Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
//    AcY += Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
//    AcZ += Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
//    Tmp += Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
//    GyX += Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
//    GyY += Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
//    GyZ += Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
//  }
//  AcX /= LOOP;
//  AcY /= LOOP;
//  AcZ /= LOOP;
//  Tmp /= LOOP;
//  GyX /= LOOP;
//  GyY /= LOOP;
//  GyZ /= LOOP;

  AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

#ifdef DEBUG
  Serial.print("AcX = "); Serial.print(AcX);
  Serial.print(" | AcY = "); Serial.print(AcY);
  Serial.print(" | AcZ = "); Serial.print(AcZ);
  Serial.print(" | Tmp = "); Serial.print(Tmp / 340.00 + 36.53); //equation for temperature in degrees C from datasheet
  Serial.print(" | GyX = "); Serial.print(GyX);
  Serial.print(" | GyY = "); Serial.print(GyY);
  Serial.print(" | GyZ = "); Serial.println(GyZ);
#endif

  delay(100);
}
