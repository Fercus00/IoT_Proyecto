#include <NewPing.h>

#define TRIGGER_PIN 17
#define ECHO_PIN 5
#define MAX_DISTANCE 200

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 

void setup() {
   Serial.begin(115200);
}
 
void loop() {
   delay(1000);
   
   Serial.print(sonar.ping_cm());// distancia del sonar como max MAX_DISTANCE
   Serial.println("cm");
}
