

#include "opciones.h"
#include "credenciales.h"

#include <TimeAlarms.h>
#include <FS.h>
#if (use_mqtt)
#include <PubSubClient.h>
#endif

#include <DHT.h>
#include <DHT_U.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <EEPROM.h>

WiFiClient  cliente_wifi;

#if (use_mqtt)
PubSubClient mqtt_client(cliente_wifi);
char msg[50];
#endif

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

ESP8266WebServer server(80);


unsigned long tiempoInicial;


bool timeBetween(String curtime, String starttime, String endtime) {
  return (((starttime < endtime) and (starttime <= curtime) and  (curtime < endtime)) or ((starttime >= endtime) and ((endtime > curtime) or (curtime >= starttime))));
}


void leerEEPROM() {
  EEPROM.begin(21);
  numAlarms = EEPROM.read(0);
  for (int i = 1; i <= 5; i++) {
    horaEncender[ i - 1] = EEPROM.read(i * 2 - 1);
    minutoEncender[i - 1] = EEPROM.read(i * 2);
    horaApagar[i - 1] = EEPROM.read(i * 2 + 9);
    minutoApagar[i - 1] = EEPROM.read(i * 2 + 10);
  }


  EEPROM.end();
}

void escribirEEPROM() {
  EEPROM.begin(21);
  EEPROM.write(0, numAlarms);  // 5 como máximo
  // (1,2), (3,4) , (5,6), (7,8) , (9 ,10) son h:m de encendido
  // (11,12), (13,14) , (15,16), (17,18) , (19 ,20) son h:m de apagado

  for (int i = 1; i <= 5; i++) {
    EEPROM.write(i * 2 - 1, horaEncender[ i - 1]);
    EEPROM.write(i * 2, minutoEncender[i - 1]);
    EEPROM.write(i * 2 + 9, horaApagar[i - 1]);
    EEPROM.write(i * 2 + 10, minutoApagar[i - 1]);
  }

  EEPROM.end();
}

void abreHuerto() {
  Serial.println("Abriendo riego en el huerto");
  digitalWrite(HUERTO_PIN, LOW);
}
void cierraHuerto() {
  Serial.println("CERRANDO riego en el huerto");
  digitalWrite(HUERTO_PIN, HIGH);
}

void printDigits(int digits)
{
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}
void crearAlarmas() {

  digitalClockDisplay();
  for (int i = 0; i < numAlarms; i++) {
    Serial.print("Poniendo alarma");
    Serial.print(i + 1);
    Serial.print(" de ");
    Serial.print(horaEncender[i]);
    Serial.print(":");
    Serial.print(minutoEncender[i]);
    Serial.print(" hasta ");
    Serial.print(horaApagar[i]);
    Serial.print(":");
    Serial.println(minutoApagar[i]);
    Alarm.alarmRepeat((int)horaEncender[i], (int)minutoEncender[i], 0, abreHuerto);
    Alarm.alarmRepeat((int)horaApagar[i], (int)minutoApagar[i], 0, cierraHuerto);
  }

}





void setup() {
  pinMode(HUERTO_PIN, OUTPUT);
  cierraHuerto();
  Serial.begin(9600);

  Serial.println("Conectando ");

  if (SPIFFS.begin())
  {
    Serial.println("SPIFFS Initialize....ok");
  }
  else
  {
    Serial.println("SPIFFS Initialization...failed");
  }

  WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED)
    { delay(1000);
  
      Serial.print(".");      // Escribiendo puntitos hasta que conecte
    }
    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Nuestra IP: ");
    Serial.println(WiFi.localIP());   // Imprimir nuestra IP al conectar

  /*
     Si no se usa wifiManager porque el essid y passwd de la wifi son fijos:
  */

  Serial.println("Iniciado el servidor HTTP");
  timeClient.begin();
  timeClient.setTimeOffset(7200); // GMT + 2
  timeClient.update();
  setTime(timeClient.getEpochTime());
  tiempoInicial = millis() + intervalo;  //para forzar una lectura en el primer loop
  leerEEPROM();
  crearAlarmas();

#if (use_mqtt)
  mqtt_client.setServer(MQTT_Broker, MQTT_port);
  mqtt_reconnect();
#endif

  server.on("/", handle_OnConnect);
  server.on("/SetHoras", HTTP_POST, handle_Parametros);
  server.on("/AbrirRiego", HTTP_GET, handle_AbrirRiego);
  server.on("/CerrarRiego", HTTP_GET, handle_CerrarRiego);  
  server.onNotFound(handleWebRequests);
  server.begin();

}

void handle_AbrirRiego()
{
  abreHuerto();
  server.send(200, "text/plain", "Riego Abierto");
}
void handle_CerrarRiego()
{
  cierraHuerto();
  server.send(200, "text/plain", "Riego Cerrado");
}

void handle_Parametros()
{
  String encender;
  String apagar;

  if (server.hasArg("plain") == false) { //Check if body received

    server.send(200, "text/plain", "No se ha recibido nada");
    return;

  }

  Serial.println("post:");
  Serial.println(server.arg("plain"));

  // mostrar por puerto serie
  Serial.println(server.argName(0));
  String webPIN = server.arg(String("pin"));

  if (webPIN != (String)PIN) {
    server.send(403, "text/plain", "PIN incorrecto");
    return;
  }

  numAlarms = (byte)server.arg(String("cantidad")).toInt();

  encender = server.arg(String("encender1"));
  apagar = server.arg(String("apagar1"));
  horaEncender[0] = (byte)encender.substring(0, 3).toInt();
  minutoEncender[0] = (byte)encender.substring(encender.length() - 2).toInt();
  horaApagar[0] = (byte)apagar.substring(0, 3).toInt();
  minutoApagar[0] = (byte)apagar.substring(apagar.length() - 2).toInt();

  if (numAlarms > 1) {
    encender = server.arg(String("encender2"));
    apagar = server.arg(String("apagar2"));
    horaEncender[1] = (byte)encender.substring(0, 3).toInt();
    minutoEncender[1] = (byte)encender.substring(encender.length() - 2).toInt();
    horaApagar[1] = (byte)apagar.substring(0, 3).toInt();
    minutoApagar[1] = (byte)apagar.substring(apagar.length() - 2).toInt();
  }

  if (numAlarms > 2) {
    encender = server.arg(String("encender3"));
    apagar = server.arg(String("apagar3"));
    horaEncender[2] = (byte)encender.substring(0, 3).toInt();
    minutoEncender[2] = (byte)encender.substring(encender.length() - 2).toInt();
    horaApagar[2] = (byte)apagar.substring(0, 3).toInt();
    minutoApagar[2] = (byte)apagar.substring(apagar.length() - 2).toInt();
  }
/*
  if (numAlarms > 3) {
    encender = server.arg(String("encender4"));
    apagar = server.arg(String("apagar4"));
    horaEncender[3] = (byte)encender.substring(0, 3).toInt();
    minutoEncender[3] = (byte)encender.substring(encender.length() - 2).toInt();
    horaApagar[3] = (byte)apagar.substring(0, 3).toInt();
    minutoApagar[3] = (byte)apagar.substring(apagar.length() - 2).toInt();
  }
  if (numAlarms > 4) {
    encender = server.arg(String("encender5"));
    apagar = server.arg(String("apagar5"));
    horaEncender[4] = (byte)encender.substring(0, 3).toInt();
    minutoEncender[4] = (byte)encender.substring(encender.length() - 2).toInt();
    horaApagar[4] = (byte)apagar.substring(0, 3).toInt();
    minutoApagar[4] = (byte)apagar.substring(apagar.length() - 2).toInt();
  }
*/
  crearAlarmas();
  escribirEEPROM();
  server.sendHeader("Location", String("/index.html"), true);
  server.send( 302, "text/plain", "");

}

void handle_NotFound()
{
  server.send(404, "text/plain", "Lo que has intentado no existe");
}

void handle_OnConnect()
{
  String row;
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<meta charset=\"UTF-8\">";
  ptr += "<title>Nodo riego Huerto El Olivo</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Nodo Huerto El Olivo</h1>\n";
  ptr += "<div>\n";  
  ptr += "<h3>" +  timeClient.getFormattedTime() + "</h3>\n";
  ptr += "<hr/>\n";  
  ptr += "<div>\n"; 
  ptr += "<h2>Periodos de riego activos</h2>\n";
  
  ptr += "<table>\n"; 
  for (int i = 0; i < numAlarms; i++) {
    row = "<tr><td>De " + String(horaEncender[i]) + ":" + String(minutoEncender[i]) + "</td><td>A "+ String(horaApagar[i]) + ":" + String(minutoApagar[i]) + "</td></tr>";
    ptr += row;
  }
  ptr += "</table>\n";
  ptr += "</div>\n";
  ptr += "<br/><br/>\n";
  ptr += "<a href=\"/alarmas.html\">Cambiar periodos de riego</a>";
  ptr += "</div>\n";

  ptr += "<form action=\"/AbrirRiego\" method=\"get\">";
  ptr += "<button type=\"submit\">Abrir riego</button>";
  ptr += "</form>";
  ptr += "<br/>\n";
  ptr += "<form action=\"/CerrarRiego\" method=\"get\">";
  ptr += "<button type=\"submit\">Cerrar riego</button>";
  ptr += "</form>";
  
  ptr += "</body>\n";
  ptr += "</html>\n";

  server.send(200, "text/html", ptr);
//  server.sendHeader("Location", "/index.html", true);  //Redirect to our html web page
//  server.send(302, "text/plane", "");
}

void handleWebRequests() {
  if (loadFromSpiffs(server.uri())) return;
  handle_NotFound();

}


bool loadFromSpiffs(String path) {
  String dataType = "text/plain";
  Serial.println(path);
  if (path.endsWith("/")) path += "index.htm";

  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".html")) dataType = "text/html";
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "application/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}


#if (use_mqtt)
void mqtt_reconnect() {
  int intentos = 0;
  const int max_intentos = 2; // no intentarlo más de estas veces
  // bucle intentando conectar:
  while (!mqtt_client.connected() && intentos <= max_intentos) {
    Serial.print("Intentando conexión MQTT...");
    // Attempt to connect
    if (mqtt_client.connect("arduinoClient")) {
      Serial.println("Conectado al Broker");

    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" Se intentará de nuevo en 5 segundos");
      delay(1000);
      intentos++;
    }
  }
}
#endif

void loop() {
  server.handleClient();
  Alarm.delay(10);

  if (millis() - tiempoInicial > intervalo) {
    tiempoInicial = millis();
    timeClient.update();
    digitalClockDisplay();
#if (use_mqtt)
    if (!mqtt_client.connected()) {
      mqtt_reconnect();
    }
    mqtt_client.loop();
    // snprintf (msg, 50, "t:%.2f;h:%.2f;ppm:%.0f", t, h, ppmcorregido);
    Serial.print("MQTT topic: ");
    Serial.println(tema);
    Serial.print("MQTT msg: ");
    Serial.println(msg);
    mqtt_client.publish(tema, msg);
#endif


  }

}
