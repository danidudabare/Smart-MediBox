#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHTesp.h>
#include <WiFi.h>
#include <time.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

//Define OLED parameters for an SSD1306 display connected to I2C (SDA, SCL pins)
#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels
#define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // OLED display I2C address

//Define parameters to get time from an NTP server 
#define NTP_SERVER "pool.ntp.org"
#define UTC_OFFSET_DST 0


//Declare objects
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHTesp dhtSensor;
WiFiClient espClient;
PubSubClient mqttClient(espClient);
Servo servo;

// Global variables
// For date and time
String date;
String Time;
int UTC_OFFSET = 0;

int hours = 0;
int minutes = 0;
int seconds = 0;

// For alarms
bool alarm_enabled = true;
int n_alarms = 3;
int alarm_hours[] = {0, 0, 0};
int alarm_minutes[] = {0, 0, 0};
bool alarm_triggered[] = {false, false, false};

// For buzzer
int C = 262;
int D = 294;
int E = 330;
int F = 349;
int G = 392;
int A = 440;
int B = 494;
int HIGH_C = 523;
int notes[] = {C, D, E, F, G, A, B, HIGH_C};
int n_notes = 8;

// For menu
int current_mode = 0;
int max_modes = 5;
String modes[] = {" 1 - Set       Time", 
                  " 2 - Set     Alarm 1", 
                  " 3 - Set     Alarm 2", 
                  " 4 - Set     Alarm 3", 
                  "5- Disable   Alarms"};

char tempArIntensity[6];
char tempArIntensitySide[6];

int angle;
int angle_offset = 30;
float controlling_factor = 0.75;
float maxIntensity;
String maxIntensitySide;

//Define input output pins
#define BUZZER 17
#define LED_1 13
#define LED_2 12
#define DHT_PIN 25

#define PB_CANCEL 27
#define PB_OK 34
#define PB_UP 35
#define PB_DOWN 26

#define LDR_RIGHT 33
#define LDR_LEFT 32
#define SERVO 14

/*-----------------------------------------------------------------------------------------------*/
// Setup code here, to run once:
void setup() {
  Serial.begin(9600);

  pinMode(LED_1, OUTPUT);
  pinMode(LED_2, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  pinMode(PB_CANCEL, INPUT);
  pinMode(PB_OK, INPUT);
  pinMode(PB_UP, INPUT);
  pinMode(PB_DOWN, INPUT);

  pinMode(LDR_RIGHT, INPUT);
  pinMode(LDR_LEFT, INPUT);
  pinMode(SERVO, OUTPUT);

  // Set up DHT sensor with the specified pin and sensor type (DHT22)
  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  // Set up servo with the specified pin
  servo.attach(SERVO);
  
  // Check whether communication has been established succesfully
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("SSD1306 Allocation Failed!");
    for(;;);    // Run an endless loop
  }

  // Successfully established the communication
  Serial.println();
  Serial.println("SSD1306 Allocation Successful!");
  Serial.println();

  // Clear the display buffer
  display.clearDisplay();
  delay(200);

  setupWifi();
  setupMqtt();

  //Display the welcome message
  display.clearDisplay();
  print_line("Welcome to   Smart    MediBox!", 5, 10, 2);
  delay(2000);
  display.clearDisplay();
}


/*-----------------------------------------------------------------------------------------------*/
// Main code here, to run repeatedly:
void loop() {
  if (!mqttClient.connected()) {
    connectToBroker();
  }

  update_time_with_check_alarm();
  print_time_now();
  delay(200);

  // If the OK button is pressed, go to the menu
  if (digitalRead(PB_OK) == LOW) {
    delay(200);
    go_to_menu();
  }

  check_temp_humidity();

  mqttClient.loop();

  updateLightIntensity();
  rotateServo();

  mqttClient.publish("LIGHT INTENSITY", tempArIntensity);
  mqttClient.publish("LIGHT INTENSITY SIDE", tempArIntensitySide);
  delay(500);
}


/*-----------------------------------------------------------------------------------------------*/
// Attempt to connect to Wi-Fi network named "Wokwi-GUEST" with an empty password on channel 6
void setupWifi() {
  WiFi.begin("Wokwi-GUEST", "", 6);
  // Wait until the Wi-Fi connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    display.clearDisplay();
    print_line("Connecting", 5, 10, 2);
    print_line("to Wi-Fi", 15, 30, 2);
    Serial.print(".");
  }

  // Successfully connected to the Wi-Fi
  Serial.println(" Connected to Wi-Fi");
  Serial.println();

  display.clearDisplay();
  print_line("Connected", 10, 15, 2);
  print_line("to Wi-Fi", 15, 35, 2);
  delay(2000);
}


/*-----------------------------------------------------------------------------------------------*/
// Function to print some text on the OLED display.
void print_line(String text, int column, int row, int text_size) { 
  display.setTextSize(text_size);
  display.setTextColor(WHITE);
  display.setCursor(column, row);
  display.println(text);
  display.display();  
}


/*-----------------------------------------------------------------------------------------------*/
// Function to update the time using Wi-Fi
void update_time_over_wifi() {
  // Configure time settings using NTP (Network Time Protocol) server
  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);

  // Initialize a structure to hold time information
  struct tm timeinfo;
  
  // Obtain the current local time
  getLocalTime(&timeinfo);
  
  // Extract hours from the timeinfo structure
  char timeHour[3];
  strftime(timeHour, 3, "%H", &timeinfo);
  hours = atoi(timeHour);

  // Extract minutes from the timeinfo structure
  char timeMinute[3];
  strftime(timeMinute, 3, "%M", &timeinfo);
  minutes = atoi(timeMinute);

  // Extract seconds from the timeinfo structure
  char timeSecond[3];
  strftime(timeSecond, 3, "%S", &timeinfo);
  seconds = atoi(timeSecond);

  // Extract date (in YYYY:MM:DD format) from the timeinfo structure
  char timeDate[11];
  strftime(timeDate, 11, "%F", &timeinfo);
  date = timeDate;

  // Extract time (in HH:MM:SS format) from the timeinfo structure
  char timeTime[25];
  strftime(timeTime, 25, "%H:%M:%S", &timeinfo);
  Time = timeTime;
}


/*-----------------------------------------------------------------------------------------------*/
// Function for displaying the time on the OLED display 
void print_time_now() {
  display.clearDisplay();
  print_line(String(date), 0, 0, 1);  // display date (in YYYY:MM:DD format) on the OLED display
  print_line(Time, 0, 15, 2);         // display time (in HH:MM:SS format) on the OLED display
}


/*-----------------------------------------------------------------------------------------------*/
// Function for checking all the alarms while updating the time
void update_time_with_check_alarm() {
  update_time_over_wifi();

  if (alarm_enabled == true) {
    for (int i = 0; i < n_alarms; i++) {
      if (alarm_triggered[i] == false && alarm_hours[i] == hours && alarm_minutes[i] == minutes) {
        ring_alarm();
        alarm_triggered[i] = true;
      }
    }
  }
}


/*-----------------------------------------------------------------------------------------------*/
// Function for ringing the alarm
void ring_alarm() {

  //Display message
  display.clearDisplay();
  print_line("Medicine    Time!", 20, 20, 2);

  //Turn on the LED
  digitalWrite(LED_1, HIGH);
  bool break_happened = false;
  
  while (break_happened == false && digitalRead(PB_CANCEL) == HIGH) {
    for (int i = 0; i < n_notes; i++) {
      // Break from the loop if we press the CANCEL button
      if (digitalRead(PB_CANCEL) == LOW) {
        delay(200);
        break_happened == true;
        break;
      } 

      // Ring the Buzzer
      tone(BUZZER, notes[i]);
      delay(400);
      noTone(BUZZER);
      delay(2);
    }
  }

  //Turn off the LED and clear the "Medicine Time" message from the display
  digitalWrite(LED_1, LOW);
  display.clearDisplay();
}

/*-----------------------------------------------------------------------------------------------*/
//Function to indicate which button is pressed after going to the menu
int wait_for_button_press() {

  while (true) {

    if (digitalRead(PB_UP) == LOW) {
      delay(200);
      return PB_UP;
    }

    if (digitalRead(PB_DOWN) == LOW) {
      delay(200);
      return PB_DOWN;
    }

    if (digitalRead(PB_OK) == LOW) {
      delay(200);
      return PB_OK;
    }

    if (digitalRead(PB_CANCEL) == LOW) {
      delay(200);
      return PB_CANCEL;
    }

    // So any alarm will ring eventhough you are navigating in the menu
    update_time_with_check_alarm();
  }
}

/*-----------------------------------------------------------------------------------------------*/
// Function for navigating through the menu
void go_to_menu() {

  while (digitalRead(PB_CANCEL) == HIGH) {
    // Display the the current mode on the OLED display
    display.clearDisplay();
    print_line(modes[current_mode], 0, 20, 2);

    int pressed = wait_for_button_press();

    // Go down the menu
    if (pressed == PB_DOWN) {
      delay(200);
      current_mode += 1;
      current_mode = current_mode % max_modes;
    }

    // Go up the menu
    else if (pressed == PB_UP) {
      delay(200);
      current_mode -= 1 ;
      if (current_mode < 0) {
        current_mode = max_modes - 1;
      }
    }

    // Run the relevant mode when OK button is pressed
    else if (pressed == PB_OK) {
      delay(200);
      run_mode(current_mode);
    }

    // Go to the home page where time is displayed
    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }
}

/*-----------------------------------------------------------------------------------------------*/
// Function for running the relevant mode
void run_mode(int mode) {
  // For setting the time
  if (mode == 0) {
    set_time();
  }

  // For setting the alarms
  else if (mode == 1 || mode == 2 || mode == 3) {
    set_alarm(mode - 1);
  }

  // To enable and disable all the alarms at once
  else if (mode == 4) {
    if (alarm_enabled) {
      display.clearDisplay();
      print_line("Alarms    Disabled", 20, 20, 2);

      Serial.println("All Alarms are Disabled");
      Serial.println();
      delay(2000);

      alarm_enabled = false;
    }
    else {
      display.clearDisplay();
      print_line("Alarms    Enabled",20 , 20, 2);

      Serial.println("All Alarms are Enabled");
      Serial.println();
      delay(2000);

      alarm_enabled = true;
    }
  }
}

/*-----------------------------------------------------------------------------------------------*/
// Function for setting the time zone
void set_time() {
  int temp_UTC = 0;
  int temp_hour = 0;

  while (true) {
    display.clearDisplay();
    print_line("Enter    hour: " + String(temp_hour), 30 , 20, 2);

    int pressed = wait_for_button_press();

    // Set the time zone hour from -12 to +12 by incrementing 1 at time
    if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < -12) {         // Avoid decremnting time below -12 hours
        temp_hour = 12;
      }
    }

    else if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1 ;
      if (temp_hour > 12) {           // Avoid incrementing time above +12 hours
        temp_hour = -12;
      }
    }

    else if (pressed == PB_OK) {
      delay(200);
      temp_UTC += temp_hour * 3600;   // Convert the hours into seconds
      break;
    }

    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

  int temp_minute = 0;

  while (true) {

    display.clearDisplay();
    print_line("Enter   minute:" + String(temp_minute), 30 , 20, 2);

    int pressed = wait_for_button_press();

    // Set the time zone minutes from -45 to +45 by incrementing 15 at a time
    if (pressed == PB_DOWN) {
      delay(200);
      temp_minute -= 15;
      if (temp_minute < -45) {      // Avoid decrementing time below -45 minutes
        temp_minute = 45;
      }
    }

    else if (pressed == PB_UP) {
      delay(200);
      temp_minute += 15 ;
      if (temp_minute > 45) {       // Avoid incrementing time above +45 minutes
        temp_minute = -45;
      }
    }

    else if (pressed == PB_OK) {
      delay(200);
      temp_UTC += temp_minute * 60;   // Convert the minutes into seconds
      UTC_OFFSET = temp_UTC;          // Copy the value to the UTC_OFFSET variable
      break;
    }

    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

  // Display "Time is Set" message for 2 seconds
  display.clearDisplay();
  print_line("Time is      set", 20, 20, 2);
  delay(2000);
}

/*-----------------------------------------------------------------------------------------------*/
// Function for setting the alarms
void set_alarm (int alarm) {

  int temp_hour = alarm_hours[alarm];

  while (true) {

    display.clearDisplay();
    print_line("Enter    hour: " + String(temp_hour), 30 , 20, 2);

    int pressed = wait_for_button_press();

    if (pressed == PB_DOWN) {
      delay(200);
      temp_hour -= 1;
      if (temp_hour < 0) {          // Avoid decrementing time below 0 hours
        temp_hour = 23;
      }
    }

    else if (pressed == PB_UP) {
      delay(200);
      temp_hour += 1 ;
      temp_hour = temp_hour % 24;   // Avoid incrementing time above 23 hours
      }

    else if (pressed == PB_OK) {
      delay(200);
      alarm_hours[alarm] = temp_hour;   // Set the new  hours of the alarm
      break;
    }

    else if (pressed == PB_CANCEL) {    
      delay(200);
      break;
    }
  }

  int temp_minute = alarm_minutes[alarm];

  while (true) {

    display.clearDisplay();
    print_line("Enter    minute:" + String(temp_minute), 30 , 20, 2);

    int pressed = wait_for_button_press();

    if (pressed == PB_DOWN) {
      delay(200);
      temp_minute -= 1;
      if (temp_minute < 0) {            // Avoid decrementing time below 0 minutes
        temp_minute = 59;
      }
    }

    else if (pressed == PB_UP) {
      delay(200);
      temp_minute += 1 ;
      temp_minute = temp_minute % 60;   // Avoid incrementing time above 59 minutes
    }

    else if (pressed == PB_OK) {
      delay(200);
      alarm_minutes[alarm] = temp_minute;   // Set the new minutes of the alarm
      break;
    }

    else if (pressed == PB_CANCEL) {
      delay(200);
      break;
    }
  }

  // Retrigger the alarm if it was turned off before
  alarm_triggered[alarm] = false;

  // Dislay the new alarm time on the OLED display for 2 seconds
  display.clearDisplay();
  print_line("Alarm " + String(alarm + 1), 15, 0, 2);
  print_line("is set to", 10, 20, 2);
  print_line(String(temp_hour) + " : " + String(temp_minute), 25, 40, 2);

  Serial.print("Alarm " + String(alarm + 1) + " is set to ");
  Serial.println(String(temp_hour) + " : " + String(temp_minute)); 
  Serial.println();
  delay(2000);
}


/*-----------------------------------------------------------------------------------------------*/
// Function for indicating dangerous temperature and humidity levels
void ring_danger() {
  // Turn on the LED
  digitalWrite(LED_2, HIGH);

  // Turn on the Buzzer
  tone(BUZZER, D, 200);
  tone(BUZZER, C, 200);
  delay(200);
  noTone(BUZZER);

  tone(BUZZER, D, 200);
  tone(BUZZER, C, 200);
  delay(200);
  noTone(BUZZER);

  // turn off the LE
  digitalWrite(LED_2, LOW);
  delay(500);
}


/*-----------------------------------------------------------------------------------------------*/
// Function for checking the temperature and humidity levels
void check_temp_humidity() {
  // Retrieve temperature and humidity data from the DHT sensor
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // If temperature is above 32 degrees Celsius
  if (!(data.temperature <= 32)) {
    print_line("TEMPERATURE HIGH", 0, 40, 1);
    delay(200);
    ring_danger();
  }

  // If temperature is below 26 degrees Celsius
  if (!(data.temperature >= 26)) {
    print_line("TEMPERATURE LOW", 0, 40, 1);
    delay(200);
    ring_danger();
  }

  // If humidity is above 80%
  if (!(data.humidity <= 80)) {
    print_line("HUMIDITY HIGH", 0, 55, 1);
    delay(200);
    ring_danger();
  }

  // If humidity is below 60%
  if (!(data.humidity >= 60)) {
    print_line("HUMIDITY LOW", 0, 55, 1);
    delay(200);
    ring_danger();
  }
}


/*-------------------------------------------------------------------------------------------------*/
void setupMqtt() {
  // Set the MQTT server and port
  mqttClient.setServer("test.mosquitto.org", 1883);
  // Set the callback function to handle received MQTT messages
  mqttClient.setCallback(receiveCallback);
}


/*-------------------------------------------------------------------------------------------------*/
void connectToBroker() {
  // Attempt to connect to the MQTT broker
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT Connection...");
    // If connection is successful
    if (mqttClient.connect("ESP32-123456")) {
      Serial.println("Connected!");
      // Subscribe to specific MQTT topics
      mqttClient.subscribe("ANGLE OFFSET");
      mqttClient.subscribe("CONTROLLING FACTOR");
    }
    else {
      // If connection fails, print the MQTT client state and retry after a delay
      Serial.println("Failed!");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}


/*-------------------------------------------------------------------------------------------------*/
void receiveCallback(char* topic, byte* payload, unsigned int length) {
  // Print the received MQTT message topic
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert the payload bytes to characters and print
  char payloadCharAr[length];
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    payloadCharAr[i] = (char)payload[i];
  }
  Serial.println();

  // Check the received topic and process the payload accordingly
  if (strcmp(topic, "ANGLE OFFSET") == 0){
    angle_offset = atoi(payloadCharAr);
  }
  
  if (strcmp(topic, "CONTROLLING FACTOR") == 0){
    controlling_factor = atof(payloadCharAr);
  }
}


/*-------------------------------------------------------------------------------------------------*/
void updateLightIntensity() {
  float lightIntensityLeft;
  float lightIntensityRight;

  // Read analog values from LDRs and calculate light intensities
  lightIntensityLeft = (4095 - analogRead(LDR_LEFT)) / 4095.00; 
  lightIntensityRight = (4095 - analogRead(LDR_RIGHT)) / 4095.00; 
 
 // Determine which side has maximum light intensity
 if (lightIntensityLeft >= lightIntensityRight) {
  maxIntensity = lightIntensityLeft;
  maxIntensitySide = "LEFT";
 }
 else {
  maxIntensity = lightIntensityRight;
  maxIntensitySide = "RIGHT";
 }

  // Store maximum intensity and its side temporarily as a float and a string
  float tempMaxIntensity = maxIntensity;
  String tempMaxIntensitySide = maxIntensitySide;

  // Convert float values to character arrays
  String(tempMaxIntensity).toCharArray(tempArIntensity, 6);
  String(tempMaxIntensitySide).toCharArray(tempArIntensitySide, 6);
}


/*-------------------------------------------------------------------------------------------------*/
void rotateServo() {
  float D;
  // Determine rotation direction based on maximum intensity side
  if (maxIntensitySide == "LEFT") {
    D = 1.5;
  }
  else {
    D = 0.5;
  }
  // Calculate servo angle based on intensity, offset, and controlling factor
  angle = angle_offset * D + (180 - angle_offset) * maxIntensity * controlling_factor;
  angle = min(angle, 180);
  // Rotate servo to calculated angle and delay for stability
  servo.write(angle);
  delay(15);
}
