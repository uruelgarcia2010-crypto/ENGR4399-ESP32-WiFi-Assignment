# ENGR 4399 - ESP32 WiFi / MQTT / JSON Wokwi Simulation

## Project behavior
This project adapts the SunFounder "IoT Communication with MQTT" concept for Wokwi.

- ESP32 connects to `Wokwi-GUEST`.
- NTC temperature sensor is connected to GPIO 34.
- Green pushbutton is connected to GPIO 27.
- LED is connected to GPIO 26 through a 220-ohm resistor.
- Pressing the button reads the NTC temperature and publishes a JSON message over MQTT.
- The ESP32 subscribes to an MQTT topic and parses incoming JSON to turn the LED on or off.

## MQTT broker
Broker: `broker.hivemq.com`
Port: `1883`

### Topics
Temperature:
`ENGR4399/UrielGarcia/TEMP`

LED control:
`ENGR4399/UrielGarcia/LED`

### Temperature JSON example
```json
{
  "temperature": 24.5,
  "unit": "C",
  "device": "ESP32",
  "source": "NTC"
}
```

### LED ON message
Publish this JSON to `ENGR4399/UrielGarcia/LED`:

```json
{"led":"on"}
```

### LED OFF message
```json
{"led":"off"}
```

## Wokwi files
1. Create/open an ESP32 Arduino project in Wokwi.
2. Replace `sketch.ino` with the provided file.
3. Replace `diagram.json` with the provided file.
4. Replace/create `libraries.txt` with the provided file.
5. Start the simulation.
6. Verify the Serial Monitor reports WiFi and MQTT connection.

## Notes
Wokwi's `wokwi-ntc-temperature-sensor` already models a 10k NTC thermistor and a 10k series resistor, so no additional 10k resistor is required in the diagram.
## Wokwi Simulation

The complete ESP32 WiFi/MQTT/JSON simulation can be viewed and tested here:

https://wokwi.com/projects/477192607104016385
