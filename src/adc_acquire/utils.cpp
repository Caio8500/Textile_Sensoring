#include "utils.h"

namespace mUtils{
void multiplex(uint8_t currentMuxChannel, char currentMux, const int csaPin, const int csbPin, const int wrPin, const int enPin, const uint8_t *addrMux){
  // mux enable
  digitalWrite(enPin, LOW);

  // reset write pin
  digitalWrite(wrPin,HIGH);  

/*   Serial.println("-----");
  Serial.print("Current Mux: "); */
  if (currentMux == 'A') {
    digitalWrite(csaPin, LOW); // enable mux A                   
    digitalWrite(csbPin, HIGH); // disable mux B
    //Serial.print("A ");
  } 
  else{
    digitalWrite(csaPin, HIGH); // disable mux A                   
    digitalWrite(csbPin, LOW); // enable mux B   
    //Serial.print("B ");     
  }
/*   Serial.print("Current Mux Channel: ");
  Serial.println(currentMuxChannel);
  Serial.println((currentMuxChannel & bitMask[0]) >> 0); // we get the first bit of the currentMuxChannel parameter
  Serial.println((currentMuxChannel & bitMask[1]) >> 1); // second bit
  Serial.println((currentMuxChannel & bitMask[2]) >> 2); // third bit
  Serial.println((currentMuxChannel & bitMask[3]) >> 3); // fourth bit */
  
  // write corresponding address to MUX
  // we right shift enough times so that we can convert the signal in a logical one, either 0 or 1
  digitalWrite(addrMux[0], uint8_t((currentMuxChannel & 0x01) >> 0));  // LSB  
  digitalWrite(addrMux[1], uint8_t((currentMuxChannel & 0x02) >> 1));  
  digitalWrite(addrMux[2], uint8_t((currentMuxChannel & 0x04) >> 2));  
  digitalWrite(addrMux[3], uint8_t((currentMuxChannel & 0x08) >> 3));  // MSB
  
  //delay(1); // little delay to make sure we can for latching the switch address via the w/r pin 
            // data sheet indicates 10ns, we will delay 1ms just to be sure
  delayMicroseconds(1); // if proven to work, we will use this to delay even less and get closer to the 10 ns mark
  digitalWrite(wrPin, LOW);
  delayMicroseconds(1);
  // set write pin and latch the switch input data
  digitalWrite(wrPin, HIGH);              
}

float getTemperatureDS18b20(OneWire ds, const int DS18B20_ID){
  byte i;
  byte data[12];
  byte addr[8];
  float temp =0.0;

  //Il n'y a qu'un seul capteur, donc on charge l'unique adresse.
  ds.search(addr);

  // Cette fonction sert à surveiller si la transmission s'est bien passée
  if (OneWire::crc8( addr, 7) != addr[7]) {
    Serial.println("getTemperatureDS18b20 : <!> CRC is not valid! <!>");
    return false;
  }

  // On vérifie que l'élément trouvé est bien un DS18B20
  if (addr[0] != DS18B20_ID) {
    Serial.println("L'équipement trouvé n'est pas un DS18B20");
    return false;
  }

  // Demander au capteur de mémoriser la température et lui laisser 850ms pour le faire (voir datasheet)
  ds.reset();
  ds.select(addr);
  ds.write(0x44);
  delay(850);
  // Demander au capteur de nous envoyer la température mémorisé
  ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  // Le MOT reçu du capteur fait 9 octets, on les charge donc un par un dans le tableau data[]
  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
  // Puis on converti la température (*0.0625 car la température est stockée sur 12 bits)
  temp = ( (data[1] << 8) + data[0] )*0.0625;

  return temp;  
}
}