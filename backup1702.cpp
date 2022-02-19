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




#define I2C_0 "/dev/i2c-0"
#define I2C_1 "/dev/i2c-1"

//char used becuase of the 8 bits used in the register

// the time is in the registers in encoded decimal form
// Convert binary coded decimal to normal decimal numbers
// int bcdToDec(char binValue) {
//     return ((binValue / 16) * 10) + (binValue % 16);
// }
// // Convert decimal to binary coded decimal
// int decTobcd(char decValue) {
//     return (decValue / 10) * 16 + (decValue % 10);
// }


// Creating Main calss with main i2c device chacaterstics and menthods 
class I2CDevice {
private:
    unsigned int bus;
    unsigned int device;
    int file;


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

    // void getTemp();
    // void getDate();
    // void setTimeDate();

    // void readAlarm1();
    // void readAlaam2();
    // void setAlaram(unsigned char)

    //void sqwg(); // Square Wave Generator

    void getTimeDate();
};

// class constrcutor 
I2CDevice::I2CDevice(unsigned int device) {
    printf("Constrcutor called\n");
    this->file = -1;
    this->device = device;
    this->open();
    this->readRegisters(BUFFER_SIZE,0x00);
}

int I2CDevice::bcdToDec(char binValue) {
    return ((binValue / 16) * 10) + (binValue % 16);
}
// Convert decimal to binary coded decimal
int I2CDevice::decTobcd(char decValue) {
    return (decValue / 10) * 16 + (decValue % 10);
}

void I2CDevice::getTimeDate(){
        printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]), bcdToDec(buf[1]), bcdToDec(buf[0]));
}

int I2CDevice::open() {
    //open the bus connection
    if ((this->file = ::open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("I2C: failed to open the bus\n");
        return 1;
    }
    //open the connection toward the device e.g. ds3231 or 0x68
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

//get methods
int I2CDevice::getSeconds() {
    return I2CDevice::readRegister(0x00);
}
int I2CDevice::getMinutes() {
    return I2CDevice::readRegister(0x01);
}
int I2CDevice::getHours() {
    return I2CDevice::readRegister(0x02);
}
int I2CDevice::getDateOfMonth() {
    return I2CDevice::readRegister(0x04);
}
int I2CDevice::getMonth() {
    return I2CDevice::readRegister(0x05);
}
int I2CDevice::getYear() {
    return I2CDevice::readRegister(0x05);
}
// int I2CDevice::getTemp() {
//     int tempFrac = 100 * (buf[18] >> 6) / 4; // Shift to extract upper 2 bits, temp is a frac of 100.
//     printf("The Temperature is %02d.%02d\n", buf[17],
//         tempFrac);
//     return;
// }

//set methoods
int I2CDevice::setSeconds(unsigned char value) {
    return I2CDevice::writeRegister(0x00, decTobcd(value));
}
int I2CDevice::setMinutes(unsigned char value) {
    return I2CDevice::writeRegister(0x01, decTobcd(value));
}
int I2CDevice::setHours(unsigned char value) {
    return I2CDevice::writeRegister(0x02, decTobcd(value));
}
int I2CDevice::setDateOfMonth(unsigned char value) {
    return I2CDevice::writeRegister(0x04, decTobcd(value));
}
int I2CDevice::setMonth(unsigned char value) {
    return I2CDevice::writeRegister(0x05, decTobcd(value));
}
int I2CDevice::setYear(unsigned char value) {
    return I2CDevice::writeRegister(0x06, decTobcd(value));
}

// int I2CDevice::setTimeDate(unsigned char value) {
//     setSeconds(unsigned char value);
//     setMinutes(unsigned char value);
//     setHours(unsigned char value);
//     setDateOfMonth(unsigned char value);
//     setMonth(unsigned char value);
//     setYear(unsigned char value);
// }

int main() {
    printf("Starting the DS3231 test application\n");
    printf("------------------------------------\n");

    I2CDevice rtc(0x68);
    //rtc.setHours(17);
    // rtc.getSeconds();
    // rtc.getMinutes();
    // rtc.getHours();
    // rtc.getDateOfMonth();
    // rtc.getMonth();
    // rtc.getYear();
    char buf[BUFFER_SIZE];

    // Reading the RTC time and date info , can be set as method (TBD)
    printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]), bcdToDec(buf[1]), bcdToDec(buf[0]));
    printf("The RTC date is %02d/%02d/%02d\n", bcdToDec(buf[3]), bcdToDec(buf[4]), bcdToDec(buf[5]), bcdToDec(buf[6]));

    // Reading the RTC Alarms info , can be set as method (TBD)
    printf("The RTC Alarm 1 time is %02d:%02d:%02d on %02d\n", bcdToDec(buf[9]), bcdToDec(buf[8]), bcdToDec(buf[7]), bcdToDec(buf[10]));
    printf("The RTC Alarm 2 time is %02d:%02d on %02d\n", bcdToDec(buf[12]), bcdToDec(buf[11]), bcdToDec(buf[13]));

    rtc.close();
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%02x ", buf[i]);
    }
    return 0;
}