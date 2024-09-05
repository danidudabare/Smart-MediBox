# Smart Medibox

This project is a "Smart Medibox" that helps users set alarms for their medicine schedule, monitor temperature and humidity, and adjust lighting intensity automatically. The system also displays real-time information like date and time on an OLED display.

## Components

- **ESP32**: The main controller used for the project.
- **DHT22**: Temperature and humidity sensor.
- **SSD1306 OLED**: 128x64 I2C display for showing time, date, alarms, and notifications.
- **Servo Motor**: Used to control light intensity.
- **LDR Sensors**: Used to detect light intensity for servo control.
- **Buzzer**: Alerts for medicine time.
- **Push Buttons**: Control the menu navigation and alarm settings.
- **Wi-Fi**: ESP32 connects to Wi-Fi to retrieve time from an NTP server.
- **MQTT**: Publishes light intensity data to a broker.

## Features

### 1. OLED Display
The OLED display shows the current date and time retrieved from an NTP server. It also displays a welcome message when the system starts. The display is updated regularly to reflect real-time values.

### 2. Temperature and Humidity Monitoring
The **DHT22** sensor monitors the temperature and humidity. This data is published to the MQTT broker for real-time monitoring.

### 3. Alarms
You can set up to three different alarms. When it's time to take the medicine, the buzzer will sound, and a message will be displayed on the OLED screen.

### 4. Light Intensity Control
Two **LDR sensors** are used to monitor the light intensity. The servo motor is adjusted accordingly to control light intensity inside the medibox. The system also publishes the light intensity values to the MQTT broker.

### 5. Wi-Fi Connectivity
The ESP32 connects to the Wi-Fi network and uses the **NTP server** to get the current time. If the Wi-Fi is not connected, the system displays a "Connecting to Wi-Fi" message on the OLED.

### 6. Menu Navigation
Using four push buttons, you can navigate through the menu options to set the time, alarms, or enable/disable alarms. The menu is displayed on the OLED screen, and navigation is intuitive with "UP," "DOWN," "OK," and "CANCEL" buttons.

## Pins Setup

| Component  | ESP32 Pin |
|------------|-----------|
| Buzzer     | 17        |
| LED 1      | 13        |
| LED 2      | 12        |
| DHT Sensor | 25        |
| Push Button - OK      | 34        |
| Push Button - Cancel  | 27        |
| Push Button - Up      | 35        |
| Push Button - Down    | 26        |
| LDR Sensor - Left     | 32        |
| LDR Sensor - Right    | 33        |
| Servo Motor           | 14        |

## Libraries Used

- **Adafruit_GFX**: For drawing on the OLED display.
- **Adafruit_SSD1306**: To control the OLED display.
- **DHTesp**: To interface with the DHT22 sensor.
- **WiFi**: For connecting the ESP32 to the Wi-Fi network.
- **PubSubClient**: For MQTT communication.
- **ESP32Servo**: For controlling the servo motor.

## Functions

### `setup()`
This function initializes the hardware and sets up the OLED display, Wi-Fi, MQTT, DHT sensor, and servo. It also prints a welcome message to the OLED and connects to the Wi-Fi.

### `loop()`
The main function checks if the ESP32 is connected to the MQTT broker, updates the time, checks the temperature and humidity, navigates through the menu when the OK button is pressed, and publishes the light intensity data to the MQTT broker.

### `setupWifi()`
This function connects the ESP32 to the specified Wi-Fi network.

### `print_line()`
A helper function to print text on the OLED display.

### `update_time_with_check_alarm()`
Updates the time from the NTP server and checks if any alarms are triggered.

### `ring_alarm()`
Rings the buzzer and displays a notification when the alarm goes off.

### `go_to_menu()`
Handles the menu navigation using the push buttons and displays the current mode on the OLED screen.

### `run_mode()`
Runs the selected mode from the menu, such as setting the time or alarms.

### `updateLightIntensity()`
Reads data from the LDR sensors and adjusts the servo motor's angle to control the light intensity.

## How to Use

1. **Wi-Fi Setup**: Modify the Wi-Fi SSID and password in the `setupWifi()` function if needed.
2. **Alarms**: Use the buttons to set up alarms. The buzzer will ring when it's time to take the medicine.
3. **Light Intensity**: The servo motor will adjust the light intensity inside the box based on data from the LDR sensors.

## How to Compile and Upload

1. Install the required libraries in your Arduino IDE:
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `DHTesp`
   - `PubSubClient`
   - `ESP32Servo`
   
2. Select the ESP32 board and the appropriate COM port.

3. Upload the code to your ESP32.

4. Monitor the serial output to ensure the system is functioning correctly.

---

This project aims to create an automated system to help users maintain a consistent medicine schedule while providing additional features like environmental monitoring and light control.
