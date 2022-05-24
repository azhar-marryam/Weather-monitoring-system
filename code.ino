//boot button  when it is reset with the button pressed, it will start up and wait you to upload new code, instead of running the code already there.


// Import required libraries
#include "WiFi.h"
//To build the web server we’ll use the ESPAsyncWebServer library as it provides an easy way to build an asynchronous web server.
//“Handle more than one connection at the same time”;
#include "ESPAsyncWebServer.h"
//To read from the DHT sensor, we’ll use the DHT library from Adafruit. 
//To use this library you also need to install the Adafruit Unified Sensor library.
#include <Adafruit_Sensor.h>
#include <DHT.h>


// Replace with your network credentials
const char* ssid = "Android";
const char* password = "password";


unsigned long previousMillis = 0;
unsigned long interval = 30000;

// Digital pin connected to the DHT sensor
#define DHTPIN 2 

//spits out a digital signal on the data pin (no analog input pins needed)
//The digital signal is fairly easy to read using any microcontroller.
//3 to 5V power and I/O
//The DHT sensors are made of two parts, a capacitive humidity sensor and a thermistor. 
// DHT 11
#define DHTTYPE DHT11

// Initialize DHT sensor.
DHT dht(DHTPIN, DHTTYPE);

// Adds a led light (in that case, it is green) to pin 14.
const int greenLED = 14;  

// Adds a buzzer to pin 32.
const int buzzer = 32; 

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDHTTemperature() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  if(t>30){                           // See if the temperature is bigger than 30C.
    digitalWrite(greenLED, HIGH);    // The green led will turn on.
  }
  else{
    digitalWrite(greenLED, LOW);    // The green led will turn off.
  }
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    //prints dash on web page
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}

String readDHTHumidity() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  if(h>40){                           // See if the humidity is bigger than 40C.
    digitalWrite(buzzer, HIGH);    // The buzzer will turn on.
  }
  else{
    digitalWrite(buzzer, LOW);    // The buzzer will turn off.
  }
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    //prints dash on web page
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  }
  return String();
}



void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
}



void setup(){
  // Serial port for debugging purposes
Serial.begin(115200);
 pinMode(greenLED, OUTPUT);           // Change to output the greenLED pin.
 pinMode(buzzer, OUTPUT);           // Change to output the buzzer pin.
 dht.begin();
  initWiFi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
// Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  // Start server
  server.begin();
}


void loop(){

   unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
  
}
