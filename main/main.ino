#include <WiFi.h>
#include <WiFiClient.h>
#include <MFRC522.h> //library responsible for communicating with the module RFID-RC522
#include <SPI.h> //library responsible for communicating of SPI bus

//Cambiar por nombre y contrasena
const char* ssid     = "nombre de red";
const char* password = "contraseña de red";

//web server establece numero de puerto a 80
WiFiServer server(80);

//Se tiene un RELAY incorporado en terminal 2, se puede usar otro GPIO
#define RELAY  13
//sensor optico
#define OPTICO_PIN 32
//rfid
#define SS_PIN 21
#define RST_PIN 22
#define SIZE_BUFFER 18
#define MAX_SIZE_BLOCK 16
#define greenPin 12
#define redPin 32
//monitor
#define OPTIC_1 34
#define OPTIC_2 35

String estado = "";
String in_out = "";
String temporal = "";

//used in authentication
MFRC522::MIFARE_Key key;
//authentication return status code
MFRC522::StatusCode status;
// Defined pins to module RC522
MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init();// Init MFRC522
  WiFi.begin(ssid, password);// Conectando a WiFi

  pinMode(RELAY, OUTPUT);
  pinMode(OPTICO_PIN, INPUT);//sensor optico
  pinMode(OPTIC_1, INPUT);//sensor optico reflector
  pinMode(OPTIC_2, INPUT);//sensor optico reflector
  digitalWrite(RELAY, HIGH);

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
  if (client) {
    Serial.print("Nuevo cliente: ");
    Serial.println(client.remoteIP());

    // Espera hasta que el cliente envíe datos.
    while(!client.available()){ delay(1); }
    /////////////////////////////////////////////////////
    // Lee la información enviada por el cliente.
    String req = client.readStringUntil('\r');
    Serial.println(req);
    // Realiza la petición del cliente.
    if (req.indexOf("RELAY_abierto_on") != -1) {estado = "Abierto";}
    if (req.indexOf("RELAY_auto_on") != -1){estado = "Automatico";}
    if (req.indexOf("RELAY_cerrado_on") != -1){estado = "Cerrado";}

    ///////////////////////////////////////////
    // Página WEB. ////////////////////////////
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  Importante.
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<head><meta charset=utf-8></head>");

    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 {background-color: #77878A;}</style></head>");

    // Web Page Heading
    client.println("<body><h1>Control de puerta</h1>");
    // Display current state
    client.println("<p>Estado " + estado + "</p>");
    // Display current state of the dog
    client.println("<p>Estado del perro " + in_out + "</p>");

    client.println("<p><a href='RELAY_abierto_on'><button class='button'>Abierto</button></a></p>");
    client.println("<p><a href='RELAY_auto_on'><button class='button'>Automatico</button></a></p>");
    client.println("<p><a href='RELAY_cerrado_on'><button class='button'>Cerrado</button></a></p>");

    client.println("</font></center></body></html>");

    Serial.print("Cliente desconectado: ");
    Serial.println(client.remoteIP());
    client.flush();
    client.stop();
  }

  ///Estados de la puerta

  if(estado == "Abierto"){
    digitalWrite(RELAY, LOW); 
    in_out = "";
  }

  else if(estado == "Automatico"){

    if(digitalRead(RELAY) == LOW){
      digitalWrite(RELAY, HIGH);
    }

    if ( ! mfrc522.PICC_IsNewCardPresent()){
      return;
    }

    // Select a card
    if ( ! mfrc522.PICC_ReadCardSerial()){
      return;
    }

    temporal = readingData();
    temporal.trim();

    if(temporal == "pet3"){
      digitalWrite(RELAY, LOW);

      int wasmils = millis();
      int seconds = 0;
      int flag = LOW;

      while(seconds != 5){
        if(digitalRead(OPTICO_PIN) == LOW){
          if(millis() - wasmils >= 1000){
          seconds ++;
          wasmils = millis();
          }
        }
        else{
          seconds = 0;
        }


        if(flag != HIGH){

          if(digitalRead(OPTIC_1) != LOW){
            in_out = "Adentro";
            Serial.println(in_out);
            flag = HIGH;
          }
          else if(digitalRead(OPTIC_2) != LOW){
            in_out = "Afuera";
            Serial.println(in_out);
            flag = HIGH;
          }
        }
      }

      digitalWrite(RELAY, HIGH);
      delay(500);
      detachInterrupt(OPTICO_PIN);
      flag = LOW;
    }

    //instructs the PICC when in the ACTIVE state to go to a "STOP" state
    mfrc522.PICC_HaltA();
    // "stop" the encryption of the PCD, it must be called after communication with authentication, otherwise new communications can not be initiated
    mfrc522.PCD_StopCrypto1();
  }

  else if(estado == "Cerrado"){
    digitalWrite(RELAY, HIGH); 
  }
}

//reads data from card/tag
String readingData()
{
//prints the technical details of the card/tag
mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid));

//prepare the key - all keys are set to FFFFFFFFFFFFh
for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

//buffer for read data
byte buffer[SIZE_BUFFER] = {0};

//the block to operate
byte block = 1;
byte size = SIZE_BUFFER; //authenticates the block to operate
status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
if (status != MFRC522::STATUS_OK) {
  Serial.print(F("Authentication failed: "));
  Serial.println(mfrc522.GetStatusCodeName(status));
  return " ";
}

//read data from block
status = mfrc522.MIFARE_Read(block, buffer, &size);
if (status != MFRC522::STATUS_OK) {
  Serial.print(F("Reading failed: "));
  Serial.println(mfrc522.GetStatusCodeName(status));
  return " ";
}

Serial.print(F("\nData from block ["));
Serial.print(block);Serial.print(F("]: "));

//prints read data
for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
{
  Serial.write(buffer[i]);
}
Serial.println(" ");
return String((char *)buffer);
}
