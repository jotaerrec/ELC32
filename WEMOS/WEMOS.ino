#include <map>
#include <ESP8266WiFi.h>

WiFiServer server(80);
WiFiClient client;

const char* WIFI_SSID     = "TeleCentro-9d10";
const char* WIFI_PASSWORD = "KZM4EWYJRTM5";
const int MAX_PINES_DE_ENTRADA = 5;
const int DIGITAL = 0;
const int ANALOGICO = 1;
const int ENTRADA = 0;
const int SALIDA = 1;
const char CR = '\r';
int contadorPin = 0;
unsigned long tiempoUltimaLectura = 0;
const unsigned long intervaloLectura = 10000; // intervalo de 5 segundos en milisegundos

struct Pin {
  int pin = -1;
  int modo;
  int tipo;
};
std::map<byte, bool> pinesUtilizados;
Pin pinesDeEntrada[MAX_PINES_DE_ENTRADA];


void setup()
{
  Serial.begin(9600);
  delay(10);
  Serial.setDebugOutput(false);
  conectarWifi(WIFI_SSID, WIFI_PASSWORD);
  imprimirDetallesConexion();
  server.begin();

  agregarPinEntrada(2, DIGITAL);
  agregarPinEntrada(12, DIGITAL);
  agregarPinEntrada(13, DIGITAL);
  agregarPinEntrada(14, DIGITAL);
  pinMode(0, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(16, OUTPUT);
}

void loop() {
  manejarSolicitudesHTTP();
  client.stop();
}


// Función principal para manejar solicitudes HTTP
void manejarSolicitudesHTTP() {
  client = server.available();
  if (client) {
    saludar();
    Serial.println("Nuevo cliente.");

    while (client.connected()) {
      manejarSolicitudHTTP();
    }
  }
}
void saludar() {
  client.println("Hola!");


}
// Función para manejar una solicitud HTTP
void manejarSolicitudHTTP() {
  String request = client.readStringUntil(CR);
  request.toLowerCase();
  escribirPin(request);
  if (contadorPin != 0 && millis() - tiempoUltimaLectura >= intervaloLectura) {
    leerPinesDeEntrada();
    tiempoUltimaLectura = millis();
  }
}


void escribirPin(String datos) {
  byte pinNumber;
  String pinStr = datos.substring(datos.indexOf("pin:") + 4, datos.indexOf(","));
  pinNumber = (byte)pinStr.toInt();

  String valorStr = datos.substring(datos.indexOf("valor:") + 6, datos.indexOf(",", datos.indexOf("valor:")));
  bool estado = valorStr == "true" ?  HIGH : LOW;
  digitalWrite(pinNumber, estado);
  Serial.print("pin:");
  Serial.print(pinNumber);
  Serial.println(estado);
}

void agregarPinEntrada(byte pinNumber, int tipo) {
  Pin nuevoPin;
  nuevoPin.pin = pinNumber;
  nuevoPin.tipo = tipo;
  nuevoPin.modo = SALIDA;
  pinesDeEntrada[contadorPin] = nuevoPin;
  pinMode(pinNumber, INPUT);
  contadorPin++;
}

void leerPinesDeEntrada() {
  int valorPin = 0;
  Serial.println("Leyendo...");
  for (int i = 0; i < contadorPin; i++) {
    Pin pin = pinesDeEntrada[i];
    if (pin.tipo == DIGITAL) {
      valorPin = digitalRead(pin.pin);
    } else if (pin.tipo == ANALOGICO) {
      valorPin = analogRead(pin.pin);
    }
    imprimirValorPin(pin.pin, valorPin);
  }
}

void imprimirValorPin(int pin, int valor) {
  client.print("valoresDeEntrada:Pin:");
  client.print(pin);
  client.print(":Valor:");
  client.println(valor);
}


void conectarWifi(const char* ssid, const char* password) {
  Serial.println();
  Serial.println();
  Serial.print("Conectando a: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

void imprimirDetallesConexion() {
  Serial.println("");
  Serial.println("WiFi Conectado.");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}
