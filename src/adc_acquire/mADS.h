#include <Arduino.h>
#include <Wire.h>
#include "utils.h"
#ifndef mADS_h
#define mADS_h


/* CONSTANTS and MASKS */


/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
#define ADS1X15_REG_POINTER_MASK (0x03)      ///< Point mask
#define ADS1X15_REG_POINTER_CONVERT (0x00)   ///< Conversion
#define ADS1X15_REG_POINTER_CONFIG (0x01)    ///< Configuration
#define ADS1X15_REG_POINTER_LOWTHRESH (0x02) ///< Low threshold
#define ADS1X15_REG_POINTER_HITHRESH (0x03)  ///< High threshold


/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
// Differential
// MUX[2:0]
#define ADS1X15_REG_CONFIG_MUX_DIFF_0_1 (0x0000) ///< Differential P = AIN0, N = AIN1 (default)
#define ADS1X15_REG_CONFIG_MUX_DIFF_0_3 (0x1000) ///< Differential P = AIN0, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_1_3 (0x2000) ///< Differential P = AIN1, N = AIN3
#define ADS1X15_REG_CONFIG_MUX_DIFF_2_3 (0x3000) ///< Differential P = AIN2, N = AIN3

// Single-Ended
// MUX[2:0]
#define ADS1X15_REG_CONFIG_MUX_SINGLE_0 (0x4000) ///< Single-ended AIN0
#define ADS1X15_REG_CONFIG_MUX_SINGLE_1 (0x5000) ///< Single-ended AIN1
#define ADS1X15_REG_CONFIG_MUX_SINGLE_2 (0x6000) ///< Single-ended AIN2
#define ADS1X15_REG_CONFIG_MUX_SINGLE_3 (0x7000) ///< Single-ended AIN3

// Single shot or Continuous mode
// MODE [0:0]
#define ADS1X15_REG_CONFIG_MODE_CONTIN (0x0000) ///< Continuous conversion mode
#define ADS1X15_REG_CONFIG_MODE_SINGLE (0x0100) ///< Power-down single-shot mode (default)

// Sampling Rate
// DR[2:0]
#define RATE_ADS1115_8SPS (0x0000)   ///< 8 samples per second
#define RATE_ADS1115_16SPS (0x0020)  ///< 16 samples per second
#define RATE_ADS1115_32SPS (0x0040)  ///< 32 samples per second
#define RATE_ADS1115_64SPS (0x0060)  ///< 64 samples per second
#define RATE_ADS1115_128SPS (0x0080) ///< 128 samples per second (default)
#define RATE_ADS1115_250SPS (0x00A0) ///< 250 samples per second
#define RATE_ADS1115_475SPS (0x00C0) ///< 475 samples per second
#define RATE_ADS1115_860SPS (0x00E0) ///< 860 samples per second

// FSR
// PGA [2:0]
#define ADS1X15_REG_CONFIG_PGA_MASK (0x0E00)   ///< PGA Mask
#define ADS1X15_REG_CONFIG_PGA_6_144V (0x0000) ///< +/-6.144V range = Gain 2/3
#define ADS1X15_REG_CONFIG_PGA_4_096V (0x0200) ///< +/-4.096V range = Gain 1
#define ADS1X15_REG_CONFIG_PGA_2_048V (0x0400) ///< +/-2.048V range = Gain 2 (default)
#define ADS1X15_REG_CONFIG_PGA_1_024V (0x0600) ///< +/-1.024V range = Gain 4
#define ADS1X15_REG_CONFIG_PGA_0_512V (0x0800) ///< +/-0.512V range = Gain 8
#define ADS1X15_REG_CONFIG_PGA_0_256V (0x0A00) ///< +/-0.256V range = Gain 16

// Enable Alert/RDY pin as a conversion ready signal (outputs the OS bit)
#define ADS1X15_REG_HI_THRESH_SET_RDY_PIN (0x8000) // set MSB of hi-thresh to 1
#define ADS1X15_REG_LO_THRESH_SET_RDY_PIN (0x0000) // set MSB of lo-thresh to 0
// COMP_QUE[1:0]
#define ADS1X15_REG_CONFIG_CQUE_RDY_PIN (0x0000) // Set the COMP_QUE[1:0] bits to any 2-bit value other than 11 to keep the ALERT/RDY pin enabled,

// Comparator configuration (probably won't be used)
#define ADS1X15_REG_CONFIG_CQUE_1CONV (0x0000) ///< Assert ALERT/RDY after one conversions
#define ADS1X15_REG_CONFIG_CQUE_2CONV (0x0001) ///< Assert ALERT/RDY after two conversions
#define ADS1X15_REG_CONFIG_CQUE_4CONV (0x0002) ///< Assert ALERT/RDY after four conversions
#define ADS1X15_REG_CONFIG_CQUE_NONE  (0x0003) ///< Disable the comparator and put ALERT/RDY in high state (default)
#define ADS1X15_REG_CONFIG_CLAT_NONLAT (0x0000) ///< Non-latching comparator (default)
#define ADS1X15_REG_CONFIG_CPOL_ACTVHI (0x0008) ///< ALERT/RDY pin is high when active
#define ADS1X15_REG_CONFIG_CMODE_TRAD  (0x0000) ///< Traditional comparator with hysteresis (default)
#define ADS1X15_REG_CONFIG_CPOL_ACTVLOW (0x0000) ///< ALERT/RDY pin is low when active (default)

// Start single shot conversion
#define ADS1X15_REG_CONFIG_OS_SINGLE (0x8000) ///< Write: Set to start a single-conversion

// Conversion Status MASK OS
#define ADS1X15_REG_CONFIG_OS_BUSY (0x0000) ///< Read: Bit = 0 when conversion is in progress
#define ADS1X15_REG_CONFIG_OS_NOTBUSY (0x8000) ///< Read: Bit = 1 when device is not performing a conversion


// ENUMS
/* FSR settings */
/* typedef enum {
  GAIN_6_144V = 0x00,
  GAIN_4_096V = 0x01,
  GAIN_2_048V = 0x02,
  GAIN_1_024V = 0x03,
  GAIN_0_512V = 0x04,
  GAIN_0_256V = 0x05
} adsFSR;

typedef enum{
  CONTINUOUS = 0x00,   
  SINGLE_SHOT = 0x01    
} adsOp;
 */




class ADS15{
public:
  // Constructor //
  ADS15(uint8_t adsAddr);
  
  // Destructor
  ~ADS15();
  
  // ADS configuration //
  void configADS(uint8_t samplingModeCode, uint8_t opModeCode, uint8_t fsrCode, uint8_t samplingRateCode =0x00);
  void enableRDYPin(); 
  // setters // 
  void setSamplingMode(uint8_t samplingMode);
  void setOpMode(uint8_t opMode);
  void setFSR(uint8_t fsr);
  void setSamplingRate(uint8_t samplingRate);    
  // getters //
  uint8_t getSamplingMode();
  uint8_t getOpMode();
  uint8_t getFSR();
  uint8_t getSamplingRate();  
  // Acquisition //
  int16_t getRawADCReading();
  int32_t getmVADCReading(bool computeTomV = false, int16_t rawADCReading = 0);
  // Tools //
  void test(); // class test 
  


  

private:
  // Attributes //
  // These variables hold the actual value that will be written to the ads
  uint8_t _adsAddr{0x48};    
  uint16_t _config{0x00};
  uint16_t _samplingMode{0x00};
  uint16_t _opMode{0x00};
  uint16_t _fsr{0x00};
  uint16_t _samplingRate{0x00};  
  // These variables hold the code used in the config ads state machine
  uint8_t _samplingModeCode{0x00};
  uint8_t _opModeCode{0x00};
  uint8_t _fsrCode{0x00};
  uint8_t _samplingRateCode{0x00}; 
      
  TwoWire* _theWire {new TwoWire}; // I2C object that will be used for communication 
  
  // Methods //  
  uint16_t readFromADS(uint8_t registerPointer);
  void writeToADS(uint8_t registerPointer, uint16_t dataBytes);
  void writeToADS(uint8_t registerPointer);
  void writeAndCheck(uint8_t registerPointer, uint16_t dataBytes);
  int16_t getLatestConversion();
  bool conversionComplete();

  // Tools //
  void breakBytes(uint8_t *buffer, uint8_t registerPointer, uint16_t dataBytes);  
  };

  // OS BIT FOR DONE CONVERSION, remeber this later

  #endif