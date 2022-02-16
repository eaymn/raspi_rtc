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

#define BUFFER_SIZE 19      //0x00 to 0x12 , 19 register address


#define I2C_0 "/dev/i2c-0"
#define I2C_1 "/dev/i2c-1"

//char used becuase of the 8 bits used in the register

// the time is in the registers in encoded decimal form
// Convert binary coded decimal to normal decimal numbers
int bcdToDec(char binValue) {
    return (binValue / 16) * 10 + (binValue % 16);
}
// Convert decimal to binary coded decimal
int decTobcd(char decValue) {
    return (decValue / 10) * 16 + (decValue % 10);
}


// Creating Main calss with main i2c device chacaterstics and menthods 
class I2CDevice {
private:
    unsigned int bus;
    unsigned int device;
    int file;
public:
    I2CDevice(unsigned int bus, unsigned int device);
    virtual int open();
    virtual int write(unsigned char value);
    virtual unsigned char readRegister(unsigned int registerAddress);
    virtual unsigned char* readRegisters(unsigned int number, unsigned int fromAddress = 0);
    virtual int writeRegister(unsigned int registerAddress, unsigned char value);
    //virtual void debugDumpRegisters(unsigned int number = 0xff);
    virtual void close();
    virtual ~I2CDevice();

    int setSeconds(unsigned char value);
    int setMinutes(unsigned char value);
    int setHours(unsigned char value);
    //int setDayOfWeek(unsigned char value);
    int setDateOfMonth(unsigned char value);
    int setMonth(unsigned char value);
    int setYear(unsigned char value);

    int getSeconds();
    int getMinutes();
    int getHours();
    //int getDayOfWeek();
    int getDateOfMonth();
    int getMonth();
    int getYear();

    void getTemp(int);
    void getDate(int, int);
   // void setDate(int, int, int;
};

// class constrcutor 
I2CDevice::I2CDevice(unsigned int bus, unsigned int device) {
    printf("Constrcutor called\n");
    this->file = -1;
    this->bus = bus;
    this->device = device;
    this->open();
}

int I2CDevice::open() {
    string name;
    if (this->bus == 0) name = I2C_0;
    else name = I2C_1;

    if ((this->file = ::open(name.c_str(), O_RDWR)) < 0) {
        perror("I2C: failed to open the bus\n");
        return 1;
    }
    if (ioctl(this->file, I2C_SLAVE, this->device) < 0) {
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
unsigned char* I2CDevice::readRegisters(unsigned int number, unsigned int fromAddress){
        this->write(fromAddress);
        unsigned char* data = new unsigned char[number];
    if(::read(this->file, data, number)!=(int)number){
        perror("I2C: Failed to read in the full buffer.\n");
        return NULL;
    }
        return data;
    
}

void I2CDevice::close() {
    ::close(this->file);
    this->file = -1;
}

// Class destructor 
I2CDevice::~I2CDevice() {
    if (file != -1) this->close();
}

//get methods
int I2CDevice::getSeconds(){
     return I2CDevice::readRegister(0x00);
}
int I2CDevice::getMinutes(){
     return I2CDevice::readRegister(0x01);
}
int I2CDevice::getHours(){
     return I2CDevice::readRegister(0x02);
}
int I2CDevice::getDateOfMonth(){
     return I2CDevice::readRegister(0x04);
}
int I2CDevice::getMonth(){
     return I2CDevice::readRegister(0x05);
}
int I2CDevice::getYear(){
      return I2CDevice::readRegister(0x05);
}
//set methoods
int I2CDevice::setSeconds(unsigned char value){
     return I2CDevice::writeRegister(0x00, decTobcd(value));
}
int I2CDevice::setMinutes(unsigned char value){
     return I2CDevice::writeRegister(0x01, decTobcd(value));
}
int I2CDevice::setHours(unsigned char value){
     return I2CDevice::writeRegister(0x02, decTobcd(value));
}
int I2CDevice::setDateOfMonth(unsigned char value){
     return I2CDevice::writeRegister(0x04, decTobcd(value));
}
int I2CDevice::setMonth(unsigned char value){
     return I2CDevice::writeRegister(0x05, decTobcd(value));
}
int I2CDevice::setYear(unsigned char value){
     return I2CDevice::writeRegister(0x06, decTobcd(value));
}

int main() {
    I2CDevice rtc(1, 0x68);
    rtc.setHours(17);
    rtc.getSeconds();
    rtc.getMinutes();
    rtc.getHours();
    rtc.getDateOfMonth();
    rtc.getMonth();
    rtc.getYear();
    char buf[BUFFER_SIZE];
    printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]),
        bcdToDec(buf[1]), bcdToDec(buf[0]));
    printf("The RTC date is %02d:%02d:%02d\n", bcdToDec(buf[4]),
        bcdToDec(buf[5]), bcdToDec(buf[6]));
    rtc.close();
    for (int i=0; i< BUFFER_SIZE; i++){
        printf("%02x ", buf[i]);
    }
    return 0;
}
