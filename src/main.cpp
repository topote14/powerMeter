#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "InternetTime.h"
#include "parser.h"
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <parser.h>
#include <Arduino.h>

//-------------------VARIABLES MEDIDOR--------------------------
#define SAMPLES 100
#define PIN_SIGNAL A0
#define SERIAL_BAUD 115200
#define RMS_VOLTAGE 250.0
#define SAMPLING_FREQUENCY 1000.0
#define RESISTANCE 1.0
#define DELAY_MS 1000

unsigned long prev_time, t_sample;
float accumulated_energy = 0;
float thd;
float max_current_ever = 0;

//-------------------VARIABLES GLOBALES--------------------------
void guardar_conf();
void escanear();
void grabar(int addr, String a);

int contconexion = 0;
unsigned long currentMillis = 0;
unsigned long timer1 = 0;
unsigned long timer2 = 0;
unsigned long timer3 = 0;
#define TIME_INTERVAL (1000)

char ssid[50];
char pass[50];

const char *ssidConf = "tutorial";
const char *passConf = "12345678";
const char *mqtt_server = "34.234.201.52";

bool reconnected = false;
bool internet = true;
//--------------------------------------------------------------
WiFiClient espClient;
ESP8266WebServer server(80);
//--------------------------------------------------------------

PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
char msgAux[MSG_BUFFER_SIZE];
char msgAux2[MSG_BUFFER_SIZE];
String buffMsg[50];
String message;
String frame = "";
String mensaje = "";

//-----------CODIGO HTML PAGINA DE CONFIGURACION---------------
String pagina = "<!DOCTYPE html>\n<html>\n<head>\n  <title>Tutorial Eeprom</title>\n  <meta charset='UTF-8'>\n  <style>\n    /* Clase para el formulario */\n    .formulario {\n      margin: 0 auto;\n      width: 300px;\n      text-align: center;\n      border: 1px solid #ccc;\n      background-color: #f2f2f2;\n      padding: 20px;\n    }\n\n    /* Clase para los campos del formulario */\n    .campo {\n      padding: 10px;\n      width: 80%;\n      margin: 10px 0;\n    }\n\n    /* Clase para el botón de envío */\n    .boton-enviar {\n      background-color: #4CAF50;\n      color: white;\n      padding: 10px 20px;\n      border: none;\n      cursor: pointer;\n    }\n\n    /* Clase para el botón de escanear */\n    .boton-escanear {\n      background-color: #4CAF50;\n      color: white;\n      padding: 10px 20px;\n      border: none;\n      cursor: pointer;\n    }\n\n    /* Hover effect para los botones */\n    .boton-enviar:hover, .boton-escanear:hover {\n      background-color: #3e8e41;\n    }\n  </style>\n</head>\n<body>\n  <form action='guardar_conf' method='get' class='formulario'>\n    <label>SSID:</label>\n    <br><br>\n    <input class='campo' name='ssid' type='text'>\n    <br>\n    <label>PASSWORD:</label>\n    <br><br>\n    <input class='campo' name='pass' type='password'>\n    <br><br>\n    <input class='boton-enviar' type='submit' value='GUARDAR'>\n    <br><br>\n  </form>\n  <a href='escanear'><button class='boton-escanear'>ESCANEAR</button></a>\n  <br><br>\n</body>\n</html>";

String paginafin = "</body>"
                   "</html>";

char *readData();
void sendData(char *msg);
char *readDataFromAnalog(String frame);
int count = 0;
void addData(char *msg);

//------------------------SETUP WIFI-----------------------------
void setup_wifi()
{
  // Conexión WIFI
  WiFi.mode(WIFI_STA); // para que no inicie el SoftAP en el modo normal
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED and contconexion < 50)
  { // Cuenta hasta 50 si no se puede conectar lo cancela
    ++contconexion;
    delay(250);
    Serial.print(".");
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
  }
  if (contconexion < 50)
  {
    Serial.println("");
    Serial.println("WiFi conectado");
    Serial.println(WiFi.localIP());
    digitalWrite(13, HIGH);
  }
  else
  {
    Serial.println("");
    Serial.println("Error de conexion");
    digitalWrite(13, LOW);
  }
}
//-------------------RECONECTAR CLIENTE MQTT--------------------
void reconnect()
{

  // Loop until we're reconnected
  while (!client.connected())
  {
    internet = false;
    //-------------------Read and accumulate data when disconnected------------------------------
    currentMillis = millis();
    if (Serial.available())
    {
      if (currentMillis - timer1 >= TIME_INTERVAL) // if data is available and timer is expired
      {
        timer1 = currentMillis; // restart timer
        reconnected = false;
        addData(readData());
      }
      else if (currentMillis - timer1 < TIME_INTERVAL)
      {
        Serial.read();
        // Serial.println("Dato descartado");
      }
    }
    //-------------------------------------------------------------------------------------------

    // Attempt to connect every 5 seconds
    if (currentMillis - timer2 >= 5000)
    {
      timer2 = currentMillis; // restart timer

      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      if (client.connect(clientId.c_str()))
      {
        Serial.println("connected");
        // Once connected, publish an announcement...
        client.publish("outTopic", "INIT");
        // ... and resubscribe
        client.subscribe("inTopic");
      }
      else
      {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
      }
    }
  }
}
//-------------------RECIBO DE MENSAJE VIA MQTT--------------------
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1')
  {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
                                    // but actually the LED is on; this is because
                                    // it is active low on the ESP-01)
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}
//-------------------PAGINA DE CONFIGURACION--------------------
void paginaconf()
{
  server.send(200, "text/html", pagina + mensaje + paginafin);
}

//--------------------MODO_CONFIGURACION------------------------
void modoconf()
{

  delay(100);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);
  delay(100);
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13, LOW);

  WiFi.softAP(ssidConf, passConf);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("IP del acces point: ");
  Serial.println(myIP);
  Serial.println("WebServer iniciado...");

  server.on("/", paginaconf); // esta es la pagina de configuracion

  server.on("/guardar_conf", guardar_conf); // Graba en la eeprom la configuracion

  server.on("/escanear", escanear); // Escanean las redes wifi disponibles

  server.begin();

  while (true)
  {
    server.handleClient();
  }
}

//---------------------GUARDAR CONFIGURACION-------------------------
void guardar_conf()
{

  Serial.println(server.arg("ssid")); // Recibimos los valores que envia por GET el formulario web
  grabar(0, server.arg("ssid"));
  Serial.println(server.arg("pass"));
  grabar(50, server.arg("pass"));

  mensaje = "Configuracion Guardada...";
  paginaconf();
}

//----------------Función para grabar en la EEPROM-------------------
void grabar(int addr, String a)
{
  int tamano = a.length();
  char inchar[50];
  a.toCharArray(inchar, tamano + 1);
  for (int i = 0; i < tamano; i++)
  {
    EEPROM.write(addr + i, inchar[i]);
  }
  for (int i = tamano; i < 50; i++)
  {
    EEPROM.write(addr + i, 255);
  }
  EEPROM.commit();
}

//-----------------Función para leer la EEPROM------------------------
String leer(int addr)
{
  byte lectura;
  String strlectura;
  for (int i = addr; i < addr + 50; i++)
  {
    lectura = EEPROM.read(i);
    if (lectura != 255)
    {
      strlectura += (char)lectura;
    }
  }
  return strlectura;
}

//---------------------------ESCANEAR----------------------------
void escanear()
{
  int n = WiFi.scanNetworks(); // devuelve el número de redes encontradas
  Serial.println("escaneo terminado");
  if (n == 0)
  { // si no encuentra ninguna red
    Serial.println("no se encontraron redes");
    mensaje = "no se encontraron redes";
  }
  else
  {
    Serial.print(n);
    Serial.println(" redes encontradas");
    mensaje = "";
    for (int i = 0; i < n; ++i)
    {
      // agrega al STRING "mensaje" la información de las redes encontradas
      mensaje = (mensaje) + "<p>" + String(i + 1) + ": " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ") Ch: " + WiFi.channel(i) + " Enc: " + WiFi.encryptionType(i) + " </p>\r\n";
      // WiFi.encryptionType 5:WEP 2:WPA/PSK 4:WPA2/PSK 7:open network 8:WPA/WPA2/PSK
      delay(10);
    }
    Serial.println(mensaje);
    paginaconf();
  }
}

//---------------------------LEER DATOS----------------------------
char *readData()
{
  // si hay internet, saco fecha y hora de internet
  frame = getFromTerminal(); // get data from terminal
  String time = getTime(internet);
  String date = getDate(internet);
  message = frame + "," + date + "," + time;  // concatenate data
  const char *c = message.c_str();            // convert to char
  snprintf(msg, MSG_BUFFER_SIZE, "%s", c);    // convert to char array
  snprintf(msgAux, MSG_BUFFER_SIZE, "%s", c); // convert to char array
  memset(msg, 0, sizeof(msg));                // clear buffer
  return (msgAux);                            // return message
}

char *readDataFromAnalog(String frame)
{
  // si hay internet, saco fecha y hora de internet
  frame = parseData(frame); // get data from terminal
  String time = getTime(internet);
  String date = getDate(internet);
  message = frame + "," + date + "," + time;  // concatenate data
  const char *c = message.c_str();            // convert to char
  snprintf(msg, MSG_BUFFER_SIZE, "%s", c);    // convert to char array
  snprintf(msgAux, MSG_BUFFER_SIZE, "%s", c); // convert to char array
  memset(msg, 0, sizeof(msg));                // clear buffer
  return (msgAux);                            // return message
}

//---------------------------ENVIAR DATOS----------------------------
void sendData(char *msg)
{
  client.publish("outTopic", msg); // publish message
  memset(msg, 0, sizeof(msg));     // clear buffer
  Serial.flush();
}

//---------------------------AGREGAR DATOS----------------------------
void addData(char *msg)
{
  Serial.print("Added ");
  Serial.print(count);
  Serial.print(" strings: ");
  buffMsg[count] = msg;
  count++;
  Serial.println(msg);
}
//------------------------SETUP-----------------------------
void setup()
{
  pinMode(13, OUTPUT); // D7
  Serial.begin(115200);
  Serial.println("");

  EEPROM.begin(512);

  pinMode(14, INPUT); // D5
  if (digitalRead(14) == 0)
  {
    modoconf();
  }

  leer(0).toCharArray(ssid, 50);
  leer(50).toCharArray(pass, 50);

  setup_wifi();
  client.setServer(mqtt_server, 1234);
  client.setCallback(callback);
  setUpTime();

  prev_time = millis();
  t_sample = millis();

  if (Serial.available())
    Serial.read(); // lee por si hay algo en el buffer del serial.
}

//--------------------------LOOP--------------------------------
void loop()
{
  reconnected = true;
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();
  currentMillis = millis();
  if (!reconnected)
  {
    Serial.println("Reconnected");
    for (int i = 0; i < count; i++)
    {
      const char *c = buffMsg[i].c_str();
      snprintf(msgAux2, MSG_BUFFER_SIZE, "%s", c);
      sendData(msgAux2);
      memset(msg, 0, sizeof(msgAux2)); // clear buffer
      delay(50);
    }
    if (Serial.available())
      Serial.read();
    count = 0;
  }
  internet = true;

  // Si hay datos en el puerto serie, me fijo si se activo el timer. Si aún no se activo, los descarto (sino se acumulan y se bugea todo).
  // Si ya se activo, envio los datos.

  if (currentMillis - timer1 >= TIME_INTERVAL) // if data is available and timer is expired
  {

    float sample;
    float max_sample = 0;
    float sum = 0;
    float active_power = 0;
    float current_time = millis();
    float elapsed_time = (current_time - prev_time) / 1000.0;
    float previous_sample = 0;
    float harmonic_current[7] = {0, 0, 0, 0, 0, 0, 0};

    for (int i = 0; i < SAMPLES; i++)
    {
      sample = analogRead(PIN_SIGNAL);
      if (sample > max_sample)
      {
        max_sample = sample;
      }
      sum += sample * sample;
      active_power += sample * previous_sample;
      previous_sample = sample;
    }

    float RMS_CURRENT = sqrt(sum / SAMPLES);
    active_power /= SAMPLES;
    active_power /= RMS_VOLTAGE * RMS_VOLTAGE;
    active_power *= SAMPLING_FREQUENCY;
    accumulated_energy += active_power * (elapsed_time / 3600.0);

    if (RMS_CURRENT > max_current_ever)
    {
      max_current_ever = RMS_CURRENT;
    }

    float percentage = 10;
    for (int i = 0; i < 7; i++)
    {
      harmonic_current[i] = RMS_CURRENT * (percentage / 100.0);
      percentage -= 1.5;
    }

    float thd = 0;
    if (RMS_CURRENT > 0)
    {
      thd = sqrt(pow(harmonic_current[1], 2) + pow(harmonic_current[2], 2) + pow(harmonic_current[3], 2) + pow(harmonic_current[4], 2) + pow(harmonic_current[5], 2) + pow(harmonic_current[6], 2)) / RMS_CURRENT;
      thd = thd;
    }

    String trama = "r\t" + (String)RMS_VOLTAGE + "\t" + (String)RMS_CURRENT + "\t" + (String)max_sample + "\t" + (String)max_current_ever + "\t" + (String)harmonic_current[0] + "\t" + (String)harmonic_current[1] + "\t" + (String)harmonic_current[2] + "\t" + (String)harmonic_current[3] + "\t" + (String)harmonic_current[4] + "\t" + (String)harmonic_current[5] + "\t" + (String)harmonic_current[6] + "\t" + (String)thd + "\t" + (String)active_power + "\t" + (String)accumulated_energy + "\t\t" + "s\t" + (String)RMS_VOLTAGE + "\t" + (String)RMS_CURRENT + "\t" + (String)max_sample + "\t" + (String)max_current_ever + "\t" + (String)harmonic_current[0] + "\t" + (String)harmonic_current[1] + "\t" + (String)harmonic_current[2] + "\t" + (String)harmonic_current[3] + "\t" + (String)harmonic_current[4] + "\t" + (String)harmonic_current[5] + "\t" + (String)harmonic_current[6] + "\t" + (String)thd + "\t" + (String)active_power + "\t" + (String)accumulated_energy + "\t\n"+ "\0";
    // Serial.println(trama);
    // trama = "Fase[°],Vrms[V],Irms[A],Ipk[A],Imax[A],Ih1[A],Ih2[A],Ih3[A],Ih4[A],Ih5[A],Ih6[A],Ih7[A],Ithd[%],Pa[kW],E[kWh]\n"+trama;

    timer1 = currentMillis; // restart timer
    sendData(readDataFromAnalog(trama));
  }
}