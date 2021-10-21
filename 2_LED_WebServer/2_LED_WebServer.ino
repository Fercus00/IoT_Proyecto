#include <WiFi.h>
#include <WiFiClient.h>

//Cambiar por nombre y contrasena
const char* ssid     = "IZZI-D251";
const char* password = "La_cubana12345";

//web server establece numero de puerto a 80
WiFiServer server(80);

//Se tiene un RELAY incorporado en terminal 2, se puede usar otro GPIO
#define RELAY  16

String estado = "";

void setup() {
  Serial.begin(115200);
  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);
  // Conectando a WiFi
  WiFi.begin(ssid, password);
  // Checa si esta conectado
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  //Imprime la direccion IP local
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.print("La dirección IP es: ");
  Serial.println(WiFi.localIP());  //Muestra IP
  // Inicio del Servidor web.
  server.begin();
  Serial.println("Servidor web iniciado.");
}

void loop() {
  // Consulta si se ha conectado algún cliente.
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
    Serial.print("Nuevo cliente: ");
    Serial.println(client.remoteIP());
    // Espera hasta que el cliente envíe datos.
    while(!client.available()){ delay(1); }
    /////////////////////////////////////////////////////
    // Lee la información enviada por el cliente.
    String req = client.readStringUntil('\r');
    Serial.println(req);
    // Realiza la petición del cliente.
    if (req.indexOf("RELAY_abierto_on") != -1) {estado = "Encendido";}
    if (req.indexOf("RELAY_auto_on") != -1){estado = "Automatico";}
    if (req.indexOf("RELAY_cerrado_on") != -1){estado = "Apagado";}
    //////////////////////////////////////////////
    // Página WEB. ////////////////////////////
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  Importante.
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head><meta charset=utf-8></head>");

    //client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    //client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
    //client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
   //client.println(".button2 {background-color: #77878A;}</style></head>");

    // Web Page Heading
    client.println("<body><h1>Control de puerta</h1>");
    // Display current state
    client.println("<p>GPIO 5 - State " + estado + "</p>");

    client.println("<p><a href='RELAY_abierto_on'><button class='button'>Abierto</button></a></p>");
    client.println("<p><a href='RELAY_auto_on'><button class='button'>Automatico</button></a></p>");
    client.println("<p><a href='RELAY_cerrado_on'><button class='button'>Cerrado</button></a></p>");

    client.println("</font></center></body></html>");

    Serial.print("Cliente desconectado: ");
    Serial.println(client.remoteIP());
    client.flush();
    client.stop();
    Serial.println(estado);

  if(estado == "Encendido"){
    digitalWrite(RELAY, LOW); 
  }
  else if(estado == "Automatico"){
    
  }
  else if(estado == "Apagado"){
    digitalWrite(RELAY, HIGH); 
  }
}
