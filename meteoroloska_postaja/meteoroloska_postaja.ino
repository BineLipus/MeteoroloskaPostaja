/*
  # Database and table creation
  DROP DATABASE IF EXISTS arduino_meteorological_station;
  CREATE DATABASE arduino_meteorological_station;
  USE arduino_meteorological_station;
  CREATE TABLE measurement (
      id integer primary key auto_increment,
      temperature float not null,
      humidity float not null,
      pressure float not null,
      recorded timestamp
    );
 */

#include <OneWire.h>

#include <SPI.h>
#include <WiFi101.h>
#include <DS18B20.h> //Temperature sensor
#include <DHT.h> //Humidity sensor
#include <LiquidCrystal.h> //Display
#include "BMP280.h"
#include "Wire.h"
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

#define P0 1013.25

#define DHTPIN 0    // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
#define LCD_CONTRAST_PIN 5

BMP280 bmp; // Pressure sensor pins: SCL (12), SDA (11)
DHT dht(DHTPIN, DHTTYPE); //Humidity sensor
DS18B20 ds(1); //temperature sensor
const int rs = 7, e = 6 , d4 = 4, d5 = 9, d6 = 3, d7 = 2; //Display
LiquidCrystal lcd(rs, e, d4, d5, d6, d7); //Display
const int lcd_columns = 16;
const int lcd_rows = 2;

char ssid[] = "WiFi_ssid";
char pass[] = "WiFi_password";

int status = WL_IDLE_STATUS;
char user[] = "db_user";              // MySQL user login username
char password[] = "db_password";        // MySQL user login password

IPAddress server_addr(192,168,1,10);  // IP of the MySQL *server* here


char INSERT_DATA[] = "INSERT INTO arduino_meteorological_station.measurement (temperature, humidity, pressure) VALUES (%.2f, %.2f, %.3f)";
char query[128];

WiFiClient client;            // Use this for WiFi instead of EthernetClient
MySQL_Connection conn((Client *)&client);
MySQL_Cursor cur = MySQL_Cursor(&conn);

bool showHumidity = true;
int insertEveryN_thIteration = 29; // Insert data into database every minute, but display it every second

void setup()
{
  lcd.begin(lcd_columns, lcd_rows);
  analogWrite(LCD_CONTRAST_PIN, 120);

  Serial.begin(9600);
  dht.begin(); //Communication with Humidity sensor is started here
  bmp.begin(); //Communication with Pressure sensor is started here

  bmp.setOversampling(4);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  connectToWiFi();
}

void connectToWiFi()
{
  
    while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connecting to:");
    lcd.setCursor(0, 1);
    lcd.print(String(ssid));
    delay(1000);
  }

  Serial.println("Connected to wifi");
  printWiFiStatus();
}

double DecimalRound(double input, int decimals)
{
  // Please excuse my peasantness
  double scale = pow(10, decimals);
  return round(input * scale) / scale;
}

void measure(double *temperature, double *pressure, double *humidity)
{
  char result = bmp.startMeasurment();
    if (result != 0)
    {
      delay(result);
      bmp.getTemperatureAndPressure(*temperature, *pressure);
      *pressure = *pressure / 1000;
      *pressure = DecimalRound(*pressure, 3);
    }

    *humidity = dht.readHumidity();
}

void displayData(double temperature, double pressure, double humidity)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temp.");
    lcd.setCursor(lcd_columns - 6, 0);
    lcd.print(String(temperature) + "C");

    if (showHumidity)
    {
      lcd.setCursor(0, 1);
      lcd.print("Humidity");
      lcd.setCursor(lcd_columns - 6, 1);
      lcd.print(String(humidity) + "%");
    }
    else
    {
      int cel_del = floor(pressure);
      int decimalni_del = pressure * 1000 - cel_del * 1000;
      lcd.setCursor(0, 1);
      lcd.print("Pressure");
      lcd.setCursor(lcd_columns - 6, 1);
      lcd.print(String(cel_del) + "." + String(decimalni_del) + "B");
    }
    showHumidity = !showHumidity;
}


void loop() {
  double temperature = 0;
  double humidity = 0;
  double pressure = 0;

  // Take measurments
  Serial.println("Taking measurements");
  measure(&temperature, &pressure, &humidity);

  // Display data on display
  displayData(temperature, pressure, humidity);

  // Insert data in database
  if (--insertEveryN_thIteration == 0)
  {
      Serial.println("Inserting data");
      insert(temperature, pressure, humidity); 
      insertEveryN_thIteration = 29;
  }

  // Wait for 2 seconds
  delay(2000);
}

void insert(double temperature, double pressure, double humidity)
{
  if (conn.connect(server_addr, 3306, user, password)) {
    Serial.println("Running a query");
    
    // Initiate the query class instance
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
    
    // Construct query (String or char[])
    sprintf(query, INSERT_DATA, temperature, humidity, pressure);
    
    // Execute the query
    cur_mem->execute(query);                                              // Note: since there are no results, we do not need to read any data
    delete cur_mem;                                                       // Deleting the cursor also frees up memory used
    Serial.println("Data recorded.");
  } 
  else {
    Serial.println("Connect failed. Trying again on next iteration.");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conn. Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Reconnecting...");
    // Wait for 2 seconds
    delay(2000);
    
    // Reconnect to WiFi (maybe connection was dropped)
    status = WL_IDLE_STATUS; // For WiFi so it can reconnect (else status would still be "WL_CONNECTED" and reconnection would not happen)
    connectToWiFi();
    // Print to display that connection failed

  }
  conn.close();
}


void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
