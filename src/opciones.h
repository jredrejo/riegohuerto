
// Usar mqtt
// #define use_mqtt 1 // Comentar esta línea si no se usa MQTT
#define MQTT_Broker  "172.23.120.19"
#define MQTT_port 1883
#define RAIZ "huerto"  //raiz de la ruta donde va a publicar
String tema_string =  String(RAIZ) + "/riego";
const char* tema = tema_string.c_str();

byte numAlarms=0;
byte horaEncender[5];
byte minutoEncender[5];
byte horaApagar[5]; 
byte minutoApagar[5]; 


// pin electroválvula huerto
const int HUERTO_PIN = D1;

// Cada cuando se envían los datos a la nube
unsigned long intervalo = 30000; // 30 seg
const char* htmlfile = "/index.html";
