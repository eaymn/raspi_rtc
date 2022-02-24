// Initial code provided by Dr. Derek Molloy, EE513 , 2021-2022
// Additional references : Exploering RaspberryPi Handbook by Dr. Derek Molloy, http://exploringrpi.com/chapter8/

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>
#include<iomanip>
#include<sys/ioctl.h>
#include<sstream>
#include<iostream>

using namespace std;

#define BUFFER_SIZE 19      //0x00 to 0x12 , 19 register address

// Reference: Figure #1, page #11 DS3231 datasheet
#define ds3231_Add 0x68
#define ds3231_secAdd 0x00
#define ds3231_minAdd 0x01
#define ds3231_hrsAdd 0x02
#define ds3231_dayOfWeekAdd 0x03
#define ds3231_dateOfMonthAdd 0x04
#define ds3231_monthAdd 0x05
#define ds3231_yearAdd 0x06
#define ds3231_tempMSBAdd 0x11
#define ds3231_tempLSBAdd 0x12
#define ds3231_ala1_sec 0x07
#define ds3231_ala1_min 0x08
#define ds3231_ala1_hrs 0x09
#define ds3231_ala1_day 0x0a
#define ds3231_ala2_min 0x0b
#define ds3231_ala2_hrs 0x0c
#define ds3231_ala2_day 0x0d
#define ds3231_control 0x0E
#define ds3231_controlStatus 0x0F
#define ds3231_aging 0x10
#define ds3231_tempMSB 0x11
#define ds3231_tempLSB 0x12

// Control bits info
//control register 0Eh/8Eh
// bit7 EOSC   Enable Oscillator (1 if oscillator must be stopped when on battery)
// bit6 BBSQW  Battery Backed Square Wave
// bit5 CONV   Convert temperature (1 forces a conversion NOW)
// bit4 RS2    Rate select - frequency of square wave output
// bit3 RS1    Rate select
// bit2 INTCN  Interrupt control (1 for use of the alarms and to disable square wave on an alarm match condition)
// bit1 A2IE   Alarm2 interrupt enable (1 to enable)
// bit0 A1IE   Alarm1 interrupt enable (1 to enable)


// SQUARE-WAVE OUTPUT FREQUENCY function preparation --> ongoingnot complet yet
// Control Register (0Eh), Bits 4 and 3: Rate Select (RS2 and RS1). These bits control the frequency of the SQW output when
// the SQW has been enabled using BBIT 2 (INTCN =0), These bits are both set to logic 1 (8.192kHz) when power is first applied
// 
// control register state (Bit7 - Bit0) --> 00011100
//    ds3231_OFF = ,      // Off 
//    ds3231_SQW1Hz = 0x00,   // 1Hz square wave   RS2_RS1 00
//    ds3231_SQW1kHz = 0x08,  //  1kHz square wave RS2_RS1 01
//    ds3231_SQW4kHz = 0x10,  //  4kHz square wave RS2_RS1 10
//    ds3231_SQW8kHz = 0x18   //  8kHz square wave RS2_RS1 11
//

// Creating Main calss with main i2c device chacaterstics and menthods 
class I2CDevice {
private:
    unsigned int bus;
    unsigned int device;
    int file;
    unsigned char *registers;
    int bcdToDec();
    int decTobcd();


public:
    I2CDevice(unsigned int device);
    virtual int open();
    virtual int write(unsigned char value);
    virtual unsigned char readRegister(unsigned int registerAddress);
    virtual unsigned char* readRegisters(unsigned int number, unsigned int fromAddress = 0);
    virtual int writeRegister(unsigned int registerAddress, unsigned char value);
    //virtual void debugDumpRegisters(unsigned int number = 0xff);
    virtual void close();
    virtual ~I2CDevice();

    int bcdToDec(char);
    int decTobcd(char);

    int setSeconds(int);
    int setMinutes(int);
    int setHours(int);
    int setDofWeek(int);
    int setDateOfMonth(int);
    int setMonth(int);
    int setYear(int);

    int getSeconds();
    int getMinutes();
    int getHours();
    int getDofWeek();
    int getDateOfMonth();
    int getMonth();
    int getYear();

    void getTimeDate();
    void setTimeDate(int, int, int, int, int, int);
    
    void getTemperature();

    void getAlarm1();
    void getAlarm2();

    // work in progress ==>
    //void setAlaram1(int, int, int, int);
    //void setAlaram2(int, int, int);

    //void squareWave(); // Square Wave Generator
    
};

    // class constrcutor 
I2CDevice::I2CDevice(unsigned int device) {
    printf("Constrcutor called I2C Device Objetc Created\n");
    this->file = -1;
    this->device = device;
    this->open();
    this->registers = NULL;  
}
int I2CDevice::open() {
    //open the bus connection, Hardcoded  with "/dev/i2c-1",
    if ((this->file = ::open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("I2C: failed to open the bus\n");
        return 1;
    }
    //open the connection toward the device Hardcoded ds3231 or 0x68
    if (ioctl(this->file, I2C_SLAVE, 0x68) < 0) {
        perror("I2C: Failed to connect to the device\n");
        return 1;
    }
    return 0;
}
int I2CDevice::write(unsigned char value) {
    unsigned char buffer[1];
    buffer[0] = value;
    if (::write(this->file, buffer, 1) != 1) {
        perror("I2C: Failed to write to the device.");
        return 1;
    }
    return 0;
}
int I2CDevice::writeRegister(unsigned int registerAddress, unsigned char value) {
    unsigned char buffer[2];
    buffer[0] = registerAddress;
    buffer[1] = value;
    if (::write(this->file, buffer, 2) != 2) {
        perror("I2C: Failed write to the device\n");
        return 1;
    }
    return 0;
}
unsigned char I2CDevice::readRegister(unsigned int registerAddress) {
    this->write(registerAddress);
    unsigned char buffer[1];
    if (::read(this->file, buffer, 1) != 1) {
        perror("I2C: Failed to read in the value.");
        return 1;
    }
    return buffer[0];
}
unsigned char* I2CDevice::readRegisters(unsigned int number, unsigned int fromAddress) {
    this->write(fromAddress);
    unsigned char* data = new unsigned char[number];
    if (::read(this->file, data, number) != (int)number) {
        perror("I2C: Failed to read in the full buffer.\n");
        return NULL;
    }
    return data; // buffer
}
void I2CDevice::close() {
    ::close(this->file);
    this->file = -1;
}

// Class destructor 
I2CDevice::~I2CDevice() {
    if (file != -1) this->close();
}


//Convert binary coded decimal to normal decimal numbers
int I2CDevice::bcdToDec(char binValue) {return ((binValue / 16) * 10) + (binValue % 16);}
// Convert decimal to binary coded decimal
int I2CDevice::decTobcd(char decValue) {return (decValue / 10) * 16 + (decValue % 10);}

//get methods
int I2CDevice::getSeconds() {return bcdToDec(I2CDevice::readRegister(0x00));}
int I2CDevice::getMinutes() {return bcdToDec(I2CDevice::readRegister(0x01));}
int I2CDevice::getHours() {return bcdToDec(I2CDevice::readRegister(0x02));}
int I2CDevice::getDofWeek() {return bcdToDec(I2CDevice::readRegister(0x03));}
int I2CDevice::getDateOfMonth() {return bcdToDec(I2CDevice::readRegister(0x04));}
int I2CDevice::getMonth() {return bcdToDec(I2CDevice::readRegister(0x05));}
int I2CDevice::getYear() {return bcdToDec(I2CDevice::readRegister(0x05));}
void I2CDevice::getTimeDate(){
    	printf("The RTC time is %02d:%02d:%02d\n", getHours(), getMinutes(), getSeconds());
        printf("The RTC date is %02d/%02d/%02d\n", getDateOfMonth(), getMonth(), getYear());
}
void I2CDevice::getTemperature() {
    unsigned char msb = bcdToDec(I2CDevice::readRegister(0x11));
	unsigned char lsb = bcdToDec(I2CDevice::readRegister(0x12));
    float rtcTemperature = msb + ((lsb >> 6) * 0.25);
    printf("The RTC Temperature is %f°C\n" , rtcTemperature);
}

//set methoods
int I2CDevice::setSeconds(int value) {return I2CDevice::writeRegister(0x00, decTobcd(value));}
int I2CDevice::setMinutes(int value) {return I2CDevice::writeRegister(0x01, decTobcd(value));}
int I2CDevice::setHours(int value) {return I2CDevice::writeRegister(0x02, decTobcd(value));}
int I2CDevice::setDofWeek(int value) {return I2CDevice::writeRegister(0x03, decTobcd(value));}
int I2CDevice::setDateOfMonth(int value) {return I2CDevice::writeRegister(0x04, decTobcd(value));}
int I2CDevice::setMonth(int value) {return I2CDevice::writeRegister(0x05, decTobcd(value));}
int I2CDevice::setYear(int value) {return I2CDevice::writeRegister(0x06, decTobcd(value));}

void I2CDevice::setTimeDate(int seconds, int minutes, int hours, int day, int month, int years) {
    setSeconds(seconds);
    setMinutes(minutes);
    setHours(hours);
    setDateOfMonth(day);
    setMonth(month);
    setYear(years);
}

// Reading the RTC Alarm #1, registers 07h to 0Ah
void I2CDevice::getAlarm1() {
    char buffer[4];
    buffer[0] = bcdToDec(I2CDevice::readRegister(0x0a)); // Day
    buffer[1] = bcdToDec(I2CDevice::readRegister(0x09)); // Hours
    buffer[2] = bcdToDec(I2CDevice::readRegister(0x08)); // Minutes
    buffer[3] = bcdToDec(I2CDevice::readRegister(0x07)); // Seconds

    printf("The RTC Alarm1 Time Is %02d:%02d:%02d", (buffer[1]), (buffer[2]), (buffer[3]));
    printf(" The RTC Alarm1 Date is %02d\n", (buffer[0]));
}

// Reading the RTC Alarm #2, registers 0Bh to 0Dh 
void I2CDevice::getAlarm2() {
    char buffer[3];
    buffer[0] = bcdToDec(I2CDevice::readRegister(0x0d)); // Day
    buffer[1] = bcdToDec(I2CDevice::readRegister(0x0c)); // Hours
    buffer[2] = bcdToDec(I2CDevice::readRegister(0xb8)); // Minutes

    printf("The RTC Alarm2 Time Is %02d:%02d:%02d", (buffer[1]), (buffer[2]));
    printf(" The RTC Alarm2 Date Is %02d\n", (buffer[0]));
}

int main() {
    printf("Starting the DS3231 test application\n");
    printf("------------------------------------\n");

    I2CDevice rtc(0x68);
    rtc.readRegisters(BUFFER_SIZE,0x00);
    //rtc.setTimeDate(00, 00, 02, 19, 02, 2022);
    rtc.getTimeDate();
    rtc.getTemperature();
    rtc.getAlarm1();
    rtc.getAlarm2();
    rtc.close();

    return 0;
}