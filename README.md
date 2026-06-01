# riegohuerto

Sistema de riego para el huerto usando **ESP32 NodeMCU** y MQTT.

## Hardware

- Placa **ESP32 NodeMCU DevKit** (ver `documentacion/ESP-32_NodeMCU_Developmentboard_Pinout.pdf`).
- Módulo relé de 2 canales 5V (activo a nivel BAJO).
- Electroválvula de plástico (alimentada por el relé).

### Conexionado

| Señal             | Pin ESP32 | Destino                     |
| ----------------- | --------- | --------------------------- |
| Relé huerto (IN1) | **GPIO5** | Entrada IN1 del módulo relé |
| VCC relé          | 5V (Vin)  | VCC módulo relé             |
| GND               | GND       | GND módulo relé             |

El relé se mantiene desactivado a `HIGH` (estado por defecto al arranque de GPIO5) y se activa a `LOW`.

Ver `documentacion/wiring-esp32.md` para más detalle.

> ⚠️ **OBSOLETO**: los ficheros `documentacion/esquema.fzz`,
> `documentacion/esquema_bb.png` y `documentacion/esquema_bb.pdf` corresponden a
> la placa ESP8266 NodeMCU **anterior** y **no reflejan el cableado actual**. Se
> conservan únicamente como referencia histórica y deben regenerarse en Fritzing
> sustituyendo el componente por un ESP32 NodeMCU DevKit (ver `wiring-esp32.md`).

## Software

Requiere el core de **ESP32 para Arduino** (`https://dl.espressif.com/dl/package_esp32_index.json`)
y las siguientes librerías:

- `WiFi` y `WebServer` (incluidas con el core ESP32)
- `SPIFFS` (incluida con el core ESP32)
- `NTPClient`
- `TimeAlarms` / `Time`
- `DHT sensor library`
- `PubSubClient` (sólo si se usa MQTT)

Para subir los ficheros de `src/huerto data/` al sistema de ficheros LittleFS del ESP32 se debe usar el plugin [LittleFS]([GitHub - earlephilhower/arduino-littlefs-upload: Build and uploads LittleFS filesystems for the Arduino-Pico RP2040, RP2350, ESP8266, and ESP32 cores under Arduino IDE 2.2.1 or higher · GitHub](https://github.com/earlephilhower/arduino-littlefs-upload)) (no el `ESP8266FS-0.5.0.zip` que estaba pensado para la placa anterior).
