#include <Wire.h>
#include "mADS.h"
#include "mTimerOne.h"
#include "utils.h"
#include <OneWire.h> // http://automacile.fr/ds18b20-capteur-de-temperature-one-wire-arduino/

// testing timer 1
// using timer 1 for acquisition interrupts
// syntax: variable_name<<number_of_positions


// acquisition with SD card

// https://lastminuteengineers.com/arduino-micro-sd-card-module-tutorial/

// TIMING DEBUG PIN //
// we use this pin to time a code section with the help of an oscilloscope
const int DEBUG_PIN{14}; // Pin A0


// MULTIPLEXER //
const int CSA_PIN{4}; // chip select pin for MUX A
const int CSB_PIN{5}; // chip select pin for MUX B
const int WR_PIN{6};  // write pin
const int EN_PIN{7};  // enable pin 
const uint8_t* ADDR_MUX {new uint8_t[4]{0,1,2,3}}; // array that maps the address pins to the mux module


// ADS ACQUISITION //
const int N_SENSORS{16};
int* dataBuffer {new int[N_SENSORS]{0}}; // buffer that will write sampled values to Serial and SD card
int nInterrupts{0}, nTransfer{0}; // number of interrupts and data transfers
const int DATA_RATE{64}; // multiplexing both channels takes around 7 milliseconds. By testing, we've realized we can't go higher than 64 sps
int32_t adcReading{0}, adcCode{0}; // variables that will receive ADC reading
int nSamples{0};
float temp{0.0};

// creating ADS1115 object 
ADS15* satiADS {new ADS15(0x48)}; // 1001 000 (ADDR PIN = GND)

// FLOW CONTROL //
bool samplingAuth{false}, dataTransferAuth{false}, transferDone{false}; // controls when acquisition and mux shift happens

// TEMPERATURE SENSOR // 
const int DS18B20_PIN=7;
const int DS18B20_ID=0x28;
float DS18B20_temperature;
const int SERIAL_PORT=9600;
// Déclaration de l'objet ds
OneWire ds(DS18B20_PIN); // on pin DS18B20_PIN (a 4.7K resistor is necessary)

void setup() {
  // ADS11115 setup
  satiADS->configADS(0x04, 0x01, 0x02, 0x04); // single shot, 128 sps
  

  // Mux pinout setup //
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, HIGH); // mux disabled
  pinMode(CSA_PIN,OUTPUT);
  digitalWrite(CSA_PIN, HIGH); // making sure chip sel pins are deactivated 
  pinMode(CSB_PIN,OUTPUT);
  digitalWrite(CSB_PIN, HIGH); // making sure chip sel pins are deactivated 
  pinMode(WR_PIN,OUTPUT);
  digitalWrite(WR_PIN, LOW); // starting with w/r pin low 
  // setup mux addr pins
  pinMode(ADDR_MUX[0], OUTPUT);
  pinMode(ADDR_MUX[1], OUTPUT);
  pinMode(ADDR_MUX[2], OUTPUT);
  pinMode(ADDR_MUX[3], OUTPUT);  

  // DEBUG pin setup //
  pinMode(DEBUG_PIN, OUTPUT);

  // TIMER SETUP //
  cli();//stop interrupts
  setupTimerOne();
  startTimerOne();
  sei();//allow interrupts


}

void loop() {
  // sampling time
  //Serial.println("Test");
  if (samplingAuth){    
    //Serial.println("................");
    /*     adcCode = satiADS->getRawADCReading();    
    adcReading = satiADS->getmVADCReading(true, adcCode); */
    // multiplex channels from both mutexes
    // Timing DEBUG wrapper --> we want to output a square wave and mesure its period //
    
/*     mUtils::multiplex(nSamples, 'A', CSA_PIN, CSB_PIN, WR_PIN, EN_PIN, ADDR_MUX);
    mUtils::multiplex((N_SENSORS - 1) - nSamples, 'B', CSA_PIN, CSB_PIN, WR_PIN, EN_PIN, ADDR_MUX); // mux b is reversed in regards to mux A */
    mUtils::multiplex(nSamples, 'A', CSA_PIN, CSB_PIN, WR_PIN, EN_PIN, ADDR_MUX);
    mUtils::multiplex((N_SENSORS - 1) - nSamples, 'B', CSA_PIN, CSB_PIN, WR_PIN, EN_PIN, ADDR_MUX); // mux b is reversed in regards to mux A
    
    // read sensor value with ADS module
    adcReading = satiADS->getmVADCReading();
    dataBuffer[nSamples] = adcReading;
    //Serial.println(dataBuffer[nSamples]);
    //Serial.print("ADC reading: ");
    //Serial.print(nSamples);
    //Serial.print(" ");
    //Serial.print("ADC code: ");
    //Serial.print(adcCode);
    //Serial.print("; ");
    //Serial.print(adcReading);
    //Serial.println(" mV");
    // if all sensors have been read, reset nSamples
    if (nSamples == N_SENSORS - 1){
      nSamples = 0;      
    }
    else{
      if(nSamples == 0){
        //temp = mUtils::getTemperatureDS18b20(ds, DS18B20_ID)*100; // get temp at start of measures batch, we multiply by 1000 to simplify the reception from the python end
        temp = 25.0;         
      }
      nSamples++;      
    }
    
    samplingAuth = false;
    transferDone = false; // if we are currently sampling, it means we haven't transferred anything yet
  }
  
  // sending to serial port and saving to SD time
  // we use thet transfer done flag to write precisely the amount of data we want, regardless of dataTransferAuth being bool true
  // we also write to the serial port in this code section so that we take as little time as possible during the interruption
  // in the final version there'll be no Serial communication during sampling  
  if(dataTransferAuth && !transferDone){
    
    // write buffer to serial port //   
    Serial.begin(115200);
    // Serial port setup - we do it here so we can still control the tx and rx pins that correspond to the mux address
     // this baud rate guarantees that the serial port won't send out garbage, slow baud causes desynchronization problems
    delayMicroseconds(100);
    //while(!Serial);  
    //Serial.println("................");
    if (nTransfer == 0){
      Serial.print("&") ; // indicating that this line should be captured by python     
    }  
    Serial.print("Sensor ");
    Serial.print(nTransfer);
    //Serial.print(" ");
    Serial.print(":");
    Serial.print(dataBuffer[nTransfer]);
    Serial.print(";");
  
    if(nTransfer == N_SENSORS - 1){
      Serial.print("Temp: ");
      Serial.print(temp);
      Serial.print("\n");

    }     
    Serial.end();
    //delayMicroseconds(100);    
    // data transfer completed
    if(nTransfer == N_SENSORS - 1){
      nTransfer = 0;    
      transferDone = true; 
                
    }
    else{
      nTransfer++; // if transfer is not done yet
    }
  }
  
}

ISR(TIMER1_COMPA_vect){
  //Serial.println(nInterrupts);
  nInterrupts++;
  // toggles samplingAuth
  if(nInterrupts == DATA_RATE - 1){ // if a second has elapsed
    nInterrupts = 0;
   // nSamples = 0;
  }
  else if (nInterrupts > N_SENSORS && nInterrupts < DATA_RATE - 1){
    samplingAuth = false;
    dataTransferAuth = true;  
    //Serial.println(nInterrupts);  
    //Serial.println("Saving to sd card...");
    //Serial.println("^");
  }
  else if(nInterrupts <= N_SENSORS){
    samplingAuth = true;
    dataTransferAuth = false;
  }
  }