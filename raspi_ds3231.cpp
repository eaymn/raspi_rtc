// Initial code provided by Dr. Derek Molloy, EE513 , 2021-2022
// Additional references : Exploering RaspberryPi Handbook by Dr. Derek Molloy

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<linux/i2c.h>
#include<linux/i2c-dev.h>


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
    virtual void debugDumpRegisters(unsigned int number = 0xff);
    virtual void close();
    virtual ~I2CDevice();
};

// class constrcutor 
I2CDevice::I2CDevice(unsigned int bus, unsigned int device) {
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

void I2CDevice::close() {
    ::close(this->file);
    this->file = -1;
}

// Class destructor 
I2CDevice::~I2CDevice() {
    if (file != -1) this->close();
}


class RtcDs3231 :public I2CDevice {

public:
    RtcDs3231(unsigned int I2CBus, unsigned int I2CAddress = 0x68);
    int setSeconds(int, int, int);
    int setMinutes(int, int, int);
    int setHours(int, int, int);
    int setDayOfWeek(int, int, int);
    int setDateOfMonth(int, int, int);
    int setMonth(int, int, int);
    int setYear(int, int, int);

    int getSeconds(int, int);
    int getMinutes(int, int);
    int getHours(int, int);
    int getDayOfWeek(int, int);
    int getDateOfMonth(int, int);
    int getMonth(int, int);
    int getYear(int, int);

    void getTemp(int);
    void getDate(int, int);
   // void setDate(int, int, int;
};

int main() {
    //int file;
    //printf("Starting the DS3231 test application\n");
    //if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
    //    perror("failed to open the bus\n");
    //    return 1;
    //}
    //if (ioctl(file, I2C_SLAVE, 0x68) < 0) {
    //    perror("Failed to connect to the sensor\n");
    //    return 1;
    //}
    //char writeBuffer[1] = { 0x00 };
    //if (write(file, writeBuffer, 1) != 1) {
    //    perror("Failed to reset the read address\n");
    //    return 1;
    //}
    //char buf[BUFFER_SIZE];
    //if (read(file, buf, BUFFER_SIZE) != BUFFER_SIZE) {
    //    perror("Failed to read in the buffer\n");
    //    return 1;
    //}
    //printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]),
    //    bcdToDec(buf[1]), bcdToDec(buf[0]));

    //float temperature = buf[0x11] + ((buf[0x12] >> 6) * 0.25);
    //printf("The temperature is %f°C\n", temperature);

    //close(file);

    RtcDs3231 rtc(1, 0x68);

    return 0;
}
