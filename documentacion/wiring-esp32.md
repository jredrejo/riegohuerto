# Conexionado ESP32 NodeMCU

Documento de referencia tras la migración desde ESP8266 NodeMCU a **ESP32 NodeMCU
DevKit** (ver `ESP-32_NodeMCU_Developmentboard_Pinout.pdf`).

## Mapeo de pines (antiguo vs nuevo)

| Función                 | ESP8266 NodeMCU (antiguo) | ESP32 NodeMCU (nuevo) |
|-------------------------|---------------------------|------------------------|
| Relé huerto (IN1)       | `D1` (GPIO5)              | `GPIO5`                |
| VCC módulo relé         | `3V3` *(ver nota)*        | `Vin` / `5V`           |
| GND                     | `GND`                     | `GND`                  |

Se ha elegido **GPIO5** en el ESP32 porque:

- Mantiene el mismo número GPIO que el `D1` del ESP8266 (D1 ESP8266 ≡ GPIO5),
  lo que simplifica la migración mental del esquema.
- En el pinout del ESP32 NodeMCU AZ-Delivery (ver
  `ESP-32_NodeMCU_Developmentboard_Pinout.pdf`) GPIO5 es un pin de propósito
  general sin restricciones especiales (no está marcado como "internal flash"
  ni "SPI RAM").
- GPIO5 es un **pin de strapping** del ESP32: debe estar a `HIGH` durante el
  `boot`. Combinado con un módulo de relé activo a nivel BAJO (que normalmente
  lleva pull-up en la entrada IN), esto se traduce en "relé desactivado / riego
  cerrado" durante el arranque, evitando aperturas espurias.

> **Nota sobre VCC del relé**: el esquema Fritzing original (ESP8266) alimentaba
> el módulo de relé desde el pin `3V3`. Para un módulo de relé **5 V** lo
> recomendable es alimentar `VCC` desde `Vin`/`5V` (entrada USB 5 V del ESP32),
> ya que la bobina del relé necesita ~5 V para activarse de forma fiable. La
> señal de control `IN1` sigue siendo de 3.3 V desde GPIO5, lo cual funciona con
> los optoacopladores habituales de estos módulos.

> **Nota sobre el segundo canal**: el esquema Fritzing original cableaba
> **dos** canales de relé (IN1 ← `D1`/GPIO5, IN2 ← `D2`/GPIO4) para dos
> electroválvulas, pero el firmware actual (`huerto.ino`, `opciones.h`) sólo
> controla **una** electroválvula a través de `HUERTO_PIN`. Si en el futuro se
> reactiva el segundo canal, un equivalente directo en ESP32 sería `GPIO4`
> (mismo número GPIO que el `D2` del ESP8266).

## Esquema lógico

```
                    +-----------------------+
                    |    ESP32 NodeMCU      |
                    |                       |
                    |  Vin / 5V  o----------+------> VCC  módulo relé 2ch (5 V)
                    |  GND       o----------+------> GND  módulo relé 2ch
                    |  GPIO5     o----------+------> IN1  módulo relé 2ch (3.3 V lógico)
                    |                       |
                    +-----------------------+

      Módulo relé 2 canales 5V (activo a LOW)
            |
            +---- COM / NO ---->  Electroválvula del huerto  ---->  Fuente AC/DC
```

## Notas de migración

- El esquema Fritzing original (`esquema.fzz`, `esquema_bb.png`, `esquema_bb.pdf`)
  está hecho con la placa ESP8266 NodeMCU. Para regenerarlo con la nueva placa
  hay que abrir Fritzing, sustituir el componente por un ESP32 NodeMCU DevKit y
  reconectar `IN1` del relé al pin `GPIO5` del nuevo módulo. El resto del cableado
  (5V, GND, relé y electroválvula) no cambia.
- En el ESP32 hay que evitar como salida los GPIO de boot (0, 2, 12, 15) si se
  añaden más actuadores; los GPIO 34–39 son **sólo entrada**.
