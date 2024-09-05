# Smart Medibox

### Connected Circuit:
![Dashboard](https://github.com/danidudabare/Smart-MediBox/blob/00687b49e9b41d4bd3d1784504dca8644b3af1a3/Images/WOKWi%20Circuit.jpg)

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

## Node-RED Dashboard Overview

The Node-RED dashboard is divided into two main sections:

### 1. **Position of The Shaded Window** (Left Panel)
This section allows the user to configure the position and behavior of the shaded window with the following controls:
- **Controlling Factor**: A multiplier that scales the final position of the window. This factor can be adjusted using a slider.
- **Minimum Angle**: A slider that sets the minimum angle for the window, ensuring the window does not drop below a specified value.
- **Select Your Option**: A dropdown menu allowing the user to select between different preset configurations for the window position (e.g., "Tablet A").

### 2. **Light Intensity Monitoring** (Right Panel)
This section provides real-time information about the light intensity, as measured by sensors, and includes:
- **Highest Light Intensity**: A gauge displaying the current light intensity (in normalized units), showing the highest measured value.
- **Highest Light Intensity Side**: Displays which side (e.g., "LEFT" or "RIGHT") has the highest light intensity.
- **Light Intensity Variation (Last 1 hour)**: A line chart tracking the variation of light intensity over the last hour, useful for identifying changes in ambient lighting conditions.

## Node-RED Flow Overview

The Node-RED flow is designed to process the data from the light sensors and apply user configurations to control the shaded window position. Below is an overview of the flow:

### 1. **Light Intensity & Light Intensity Side** (Upper Nodes)
- **Light Intensity**: This node collects real-time data from the light intensity sensors.
- **Light Intensity Variation**: Tracks and displays the variation in light intensity over time (last 1 hour).
- **Light Intensity Side**: Determines which side has the highest light intensity and sends this data to the dashboard.

### 2. **Angle and Controlling Factor Calculations** (Middle Nodes)
- **Switch Node**: This node selects different behavior or configuration options based on the user’s selection from the dashboard (e.g., "Tablet A").
- **Angle & Controlling Factor Nodes**: These nodes calculate the window angle and controlling factor based on the light intensity data and user settings. Each calculation is handled in separate function nodes (`Angle A`, `CF A`, `Angle B`, etc.).

### 3. **Angle Offset and Controlling Factor Outputs** (Right Side Nodes)
- **Minimum Angle and Controlling Factor Output**: The calculated values are fed into these output nodes to adjust the actual position of the shaded window. These values are also linked back to the sliders in the dashboard so the user can adjust and monitor them in real-time.

## System Architecture

1. **Sensors**: Light intensity sensors provide real-time data about the light levels in different areas.
2. **Node-RED Flow**: Processes the sensor data and computes the appropriate shaded window position based on user inputs.
3. **Dashboard**: A user-friendly interface for real-time monitoring and adjustment of system parameters.

### Node-RED Dashboard:
![Dashboard](https://github.com/danidudabare/Smart-MediBox/blob/00687b49e9b41d4bd3d1784504dca8644b3af1a3/Images/Node%20Red%20Dashboard.jpg)

### Node-RED Flow:
![Flow](https://github.com/danidudabare/Smart-MediBox/blob/00687b49e9b41d4bd3d1784504dca8644b3af1a3/Images/Node%20Red%20Flow.png)

## How to Use the System

1. **Adjusting the Controlling Factor**: Use the "Controlling Factor" slider to modify the multiplier that affects the window's position.
2. **Setting the Minimum Angle**: Use the "Minimum Angle" slider to ensure that the window does not fall below the set angle.
3. **Select Your Option**: Choose from different predefined configurations (e.g., "Tablet A") for different window behaviors.
4. **Monitor Light Intensity**: Observe real-time light intensity changes and the side with the highest light intensity on the right-hand panel.
5. **Track Light Intensity Variation**: Monitor the light intensity trend over the last hour with the provided chart.

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
