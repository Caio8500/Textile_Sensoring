#include "mADS.h"

// TODO: fazer métodos de set gain e etc baseado em atributos privados da classe.

ADS15::ADS15(uint8_t adsAddr = 0x48){
  _adsAddr = adsAddr;
  // i2c interface initialization 
  _theWire->begin();                // join i2c bus (address optional for master)
  
}

ADS15::~ADS15(){
  delete _theWire; // freeing memory taken by the wire object
}


void ADS15::test(){
  Serial.println(_adsAddr);
}


void ADS15::configADS(uint8_t samplingModeCode, uint8_t opModeCode, uint8_t fsrCode, uint8_t samplingRateCode = 0x00){
  /* This method configures the ADS module, the hex codes used for config are the same as in the datasheet  
  :param samplingMode: sets differential or single-ended reading
  :param opMode: sets continuous or single shot acquisition
  :param fsr: sets ADC reading range
  :param samplingRate: sampling rate used if ADC configured to continuous acquistion        
  :return: void
   */
  // whether it is single shot or continuous mode, the data rate will limit the max sample rate 
  //_opMode = opMode;
  LOG_DEBUG_LN("*** Setting ADS configuration up ***")

  _samplingModeCode = samplingModeCode;
  _opModeCode = opModeCode;
  _samplingRateCode = samplingRateCode;  
  _fsrCode = fsrCode;          
  //uint8_t *buffer{new uint8_t[3]{0x00}}; // 3 elements 8 bit buffer that will be sent to the ads (must be 8 bits because i2c transmits 8 bits)
  
  switch(_samplingModeCode){
    case(0x00):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_DIFF_0_1;
    break;
    
    case(0x01):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_DIFF_0_3;
    break;

    case(0x02):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_DIFF_1_3;
    break; 

    case(0x03):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_DIFF_2_3;
    break;

    case(0x04):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_SINGLE_0;
    break;
    
    case(0x05):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_SINGLE_1;
    break;

    case(0x06):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_SINGLE_2;
    break;
    
    case(0x07):
      _samplingMode = ADS1X15_REG_CONFIG_MUX_SINGLE_3;
    break;
  }

  switch(_opModeCode){
    // continuous
    case(0x00):
      _opMode = ADS1X15_REG_CONFIG_MODE_CONTIN;
    break;

    // single-shot
    case(0x01):
      _opMode = ADS1X15_REG_CONFIG_MODE_SINGLE;
    break;        
  }

  switch(_samplingRateCode){
    case(0x00):
      _samplingRate = RATE_ADS1115_8SPS;
    break;
    
    case(0x01):
      _samplingRate = RATE_ADS1115_16SPS;
    break;

    case(0x02):
      _samplingRate = RATE_ADS1115_32SPS;
    break; 

    case(0x03):
      _samplingRate = RATE_ADS1115_64SPS;
    break;

    case(0x04):
      _samplingRate = RATE_ADS1115_128SPS;
    break;
    
    case(0x05):
      _samplingRate = RATE_ADS1115_250SPS;
    break;

    case(0x06):
      _samplingRate = RATE_ADS1115_475SPS;
    break;
    
    case(0x07):
      _samplingRate = RATE_ADS1115_860SPS;
    break;
  } 

  switch(_fsrCode){
    case(0x00):
      _fsr = ADS1X15_REG_CONFIG_PGA_6_144V;
    break;
    
    case(0x01):
      _fsr = ADS1X15_REG_CONFIG_PGA_4_096V;
    break;

    case(0x02):
      _fsr = ADS1X15_REG_CONFIG_PGA_2_048V;
    break; 

    case(0x03):
      _fsr = ADS1X15_REG_CONFIG_PGA_1_024V;
    break;

    case(0x04):
      _fsr = ADS1X15_REG_CONFIG_PGA_0_512V;
    break;
    
    case(0x05):
      _fsr = ADS1X15_REG_CONFIG_PGA_0_256V;
    break;

    case(0x06):
      _fsr = ADS1X15_REG_CONFIG_PGA_0_256V;
    break;
    
    case(0x07):
      _fsr = ADS1X15_REG_CONFIG_PGA_0_256V;
    break;
  } 

}

// setters 
void ADS15::setSamplingMode(uint8_t samplingModeCode){
    configADS(samplingModeCode, _opModeCode, _fsrCode, _samplingRateCode); // calls ads config method
}

void ADS15::setOpMode(uint8_t opModeCode){
    configADS(_samplingModeCode, opModeCode, _fsrCode, _samplingRateCode); // calls ads config method
}

void ADS15::setFSR(uint8_t fsrCode){
  configADS(_samplingModeCode, _opModeCode, fsrCode, _samplingRateCode); // calls ads config method
}

void ADS15::setSamplingRate(uint8_t samplingRateCode){
    configADS(_samplingModeCode, _opModeCode, _fsrCode, samplingRateCode); // calls ads config method
}

// getters
uint8_t ADS15::getSamplingMode(){return _samplingModeCode;}
uint8_t ADS15::getOpMode(){return _opModeCode;}
uint8_t ADS15::getFSR(){return _fsrCode;}
uint8_t ADS15::getSamplingRate(){return _samplingRateCode;}


void ADS15::enableRDYPin(){
  /* This method enables the conversion ready pin - the pin must be then configures as digital input in the main program 
  :return: void
   */ 
  uint16_t config {0x00};
  LOG_DEBUG_LN("*** Enabling RDY PIN ***");
  
  // set hithresh msb to 1
  config = ADS1X15_REG_HI_THRESH_SET_RDY_PIN; // setting HITHRESH MSB to 1
  writeToADS(ADS1X15_REG_POINTER_HITHRESH, config);

  // set lothresh msb to 0
  config = 0x00;
  config = ADS1X15_REG_LO_THRESH_SET_RDY_PIN;
  writeToADS(ADS1X15_REG_POINTER_LOWTHRESH, config);  // setting LOTHRESH MSB to 0
   
}


void ADS15::writeToADS(uint8_t registerPointer, uint16_t dataBytes){
  /* This method writes content to the register pointed by the registerPointer
  :param registerPointer: defines which register to point to 
  :param dataBytes: 2 bytes corresponding to the data we want to write to the pointed register     
  :return: void
  */
  uint8_t *buffer{new uint8_t[3]{0x00}}; // 3 elements 8 bit buffer that will be sent to the ads (must be 8 bits because i2c transmits 8 bits)
  breakBytes(buffer, registerPointer, dataBytes);
  LOG_DEBUG("Writing to register: ")
  LOG_DEBUG_LN_HEX(buffer[0])
  LOG_DEBUG_LN("Writing bytes: ")
  LOG_DEBUG_LN_HEX(buffer[1])
  LOG_DEBUG_LN_HEX(buffer[2])
  LOG_DEBUG_LN("---------")
  _theWire->beginTransmission(_adsAddr);
  _theWire->write(buffer, (size_t)3);
  LOG_DEBUG("Transmission Error code") 
  uint8_t error =_theWire->endTransmission();
  LOG_DEBUG_LN(error)
  delete buffer;
}

void ADS15::writeToADS(uint8_t registerPointer){
  /* This method is used to point to the appropriate register without writing any data to it
  :param registerPointer: defines which register to point to
  :return: void
  */
  LOG_DEBUG("Writing to pointer register: ")
  LOG_DEBUG_LN_HEX(registerPointer)
  _theWire->beginTransmission(_adsAddr);
  _theWire->write(registerPointer);
  LOG_DEBUG("Transmission Error code") 
  uint8_t error = _theWire->endTransmission();
  LOG_DEBUG_LN(error);
}

void ADS15::writeAndCheck(uint8_t registerPointer, uint16_t dataBytes){
  /* This method writes data to the given register, reads the data stored in it and checks whether it is what we wrote to it.
  Application: mainly debug
  :param registerPointer: defines which register to point to 
  :param dataBytes: 2 bytes corresponding to the data we want to write to the pointed register     
  :return: void
  */
  writeToADS(registerPointer,dataBytes);
  uint16_t receivedBytes = readFromADS(registerPointer);
  if (dataBytes == receivedBytes){
    LOG_DEBUG("Successfull transmission of bytes")
    LOG_DEBUG_LN_HEX(dataBytes)
    LOG_DEBUG_LN("To pointer register")  
    LOG_DEBUG_LN_HEX(registerPointer)  
  }  
  else 
    LOG_DEBUG("Failed transmission of bytes")
    LOG_DEBUG_LN_HEX(dataBytes)
    LOG_DEBUG_LN("To pointer register")  
    LOG_DEBUG_LN_HEX(registerPointer)  
    LOG_DEBUG("Sent: ")
    LOG_DEBUG_LN_HEX(dataBytes)
    LOG_DEBUG("Received: ")
    LOG_DEBUG_LN_HEX(receivedBytes)  
}


uint16_t ADS15::readFromADS(uint8_t registerPointer){
  /* This method reads the register (2 bytes) that is currently being pointed   
  :param registerPointer: defines which register to point to      
  :return: two bytes in the form of a unsigned 16 bits integer
  */  
  uint16_t reading{0x00};
  int nBytesReceived{0};
  writeToADS(registerPointer);  // pointing to the register we want to read from
  nBytesReceived = _theWire->requestFrom(_adsAddr, (uint8_t)2); // request 2 bytes from the ADS (bytes from the register pointed to)
  if (nBytesReceived >= 2){  // if two bytes were received
    reading = _theWire->read();
    reading = reading << 8; // shift 8 bits to the left, we receive the MSB first
    reading |= _theWire->read(); // get the LSB without affecting the MSB part
    LOG_DEBUG("The Master received bytes: ")
    LOG_DEBUG_LN_HEX(reading >> 8) // print MSB
    LOG_DEBUG_LN_HEX(reading & 0xFF) // print LSB
  }
  else{
    LOG_DEBUG_LN("The Master did not receive enough bytes from ADS! ")
  }
  return reading;    
}

int16_t ADS15::getRawADCReading(){
  /* This method starts a conversion and returns its reading in the form of a *signed* 16 bit integer (ads return data in 2's complement format)       
  :return: two bytes in the form of a signed 16 bits integer
  */  
  
  uint16_t baseConfig {0x0000};
  _config &= 0x0000; // reset _config bytes  

  // writing default baseConfig 
  baseConfig |= ADS1X15_REG_CONFIG_CQUE_1CONV |    // Assert after one conversion, we set this so that the RDY pin can be used as a conversion ready pin
            ADS1X15_REG_CONFIG_CLAT_NONLAT |  // Non-latching (default val)
            ADS1X15_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
            ADS1X15_REG_CONFIG_CMODE_TRAD;   // Traditional comparator (default val)
  

  
  if (_opMode == ADS1X15_REG_CONFIG_MODE_SINGLE){ // if single shot mode start the conversion by setting the OS pin
    uint16_t startConversion {0x00};
    startConversion = ADS1X15_REG_CONFIG_OS_SINGLE; // Set 'start single-conversion' bit, this bit both controls conversion start and returns the conversion status
    _config |= startConversion;    
  }
  
  _config |= baseConfig | _samplingMode | _opMode | _fsr | _samplingRate;  // assemble _config register
  
  LOG_DEBUG_LN("*** Starting ADS conversion ***")
  writeToADS(ADS1X15_REG_POINTER_CONFIG, _config); // Write start single-conversion bit to config register
  
  if(_opMode == ADS1X15_REG_CONFIG_MODE_SINGLE){ // if single shot mode wait for the conversion to end
    //Serial.println("Single Shot Mode");
    while(!conversionComplete()){} ;    
  } 
  else{
    //Serial.println("Continuous Mode");    
  }
  return getLatestConversion();

}

int32_t ADS15::getmVADCReading(bool computeTomV = false, int16_t rawADCReading = 0){
  /* This method operates in 2 different ways, 
  it can be used to start a conversion and convert the result to mv
  or it can receive a raw ADC reading obtained elsewhere and just convert it to mV, without starting a new conversion
  :return: 4 bytes in the form of a signed 32 bits integer that corresponds to a value in mV
  */  
  int32_t scaleFactor{0}; // scale factor ir uV
  int32_t reading{0};
  switch(_fsrCode){
    case(0x00):
      scaleFactor = 6144000/32767;
    break;
    
    case(0x01):
      scaleFactor = 4096000/32767;
    break;

    case(0x02):
      scaleFactor = 2048000/32767;
    break; 

    case(0x03):
      scaleFactor = 1024000/32767;
    break;

    case(0x04):
      scaleFactor = 512000/32767;
    break;
    
    case(0x05):
      scaleFactor = 256000/32767;
    break;

    case(0x06):
      scaleFactor = 256000/32767;
    break;
    
    case(0x07):
      scaleFactor = 256000/32767;
    break;
  } 
  if (!computeTomV){
    reading = (int32_t)getRawADCReading();   
  }
  else{
    reading = rawADCReading;    
  } 
/*   Serial.println(_fsrCode);
  Serial.println(reading);
  Serial.println(scaleFactor);
  Serial.println(reading*scaleFactor/1000); */
  
  return (reading*scaleFactor)/1000;
}

int16_t ADS15::getLatestConversion(){
  /* This method returns the latest conversion available in the conversion register       
  :return: two bytes in the form of a signed 16 bits integer
  */ 
  return (int16_t)readFromADS(ADS1X15_REG_POINTER_CONVERT); // the cast is necessary because the ADS sends data in two's complement
}

bool ADS15::conversionComplete(){
  /* This method checks if the ADC is busy or free, if it is free, it means the conversion is ready because we first write 1 to the OS bit, it is automatically cleared by the hw, when conversion is done
  the hw sets the OS bit to 1 again
  return: bool that tells whether conversion is done or not 
  */  
  return((readFromADS(ADS1X15_REG_POINTER_CONFIG) & ADS1X15_REG_CONFIG_OS_NOTBUSY) != 0); // While the os bit is not 1 (adc is free), the adc is busy with a conversion
}

void ADS15::breakBytes(uint8_t *buffer, uint8_t registerPointerBytes, uint16_t dataBytes){
  /* This method breaks information in 3 bytes, one byte for the register pointer and the other 2 with the data we want to transmit to the pointed register 
  :param buffer: array or pointer to which data will be written to
  :param registerPointerBytes: byte that indicates to which register we want to point to
  :param dataBytes: data we want to write to the currently pointed register
  :return: void  
  */
  buffer[0] = registerPointerBytes;
  buffer[1] = dataBytes >> 8; // sending MSB, 8 byte right shift to get the MSBs 
  buffer[2] = dataBytes & 0xFF; // sending LSB (mask all "1")
}
