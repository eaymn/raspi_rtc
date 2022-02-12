// initial code provided by Dr. derek M, as part of EE513 moulde assignment

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<linux/i2c-dev.h>
#define BUFFER_SIZE 19      //0x00 to 0x12


class RasRtc {
	public:
	void setDate(int,bType,bType,bType,bType,bType,bType,bType,int);
	void getTemp(int);
	int setYear(int ,int, int);
	int setDayOfWeek(int ,int, int);
	int setDateOfMonth(int ,int, int);
	int setHours(int ,int, int);
	int setSeconds(int, int, int);
	int setMinutes(int ,int, int);
	int setMonth(int ,int, int);
	int getSeconds(int, int);
	int getYear(int ,int);
	int getDayOfWeek(int ,int);
	int getDateOfMonth(int ,int);
	int getHours(int ,int);
	int getMinutes(int ,int);
	int getMonth(int ,int);
	void getDate(int,int);
};

// the time is in the registers in encoded decimal form
int bcdToDec(char bin_Value){
    return ((bin_Value / 16 * 10) + (bin_Value % 16));

}

int decToBcd(char dec_Value){
    return((dec_Value / 10 *16) + (dec_Value % 10));
}

int main() {
    int file;
    printf("Starting the DS3231 test application\n");
    if ((file = open("/dev/i2c-1", O_RDWR)) < 0) {
        perror("failed to open the bus\n");
        return 1;
    }
    if (ioctl(file, I2C_SLAVE, 0x68) < 0) {
        perror("Failed to connect to the sensor\n");
        return 1;
    }
    char writeBuffer[1] = { 0x00 };
    if (write(file, writeBuffer, 1) != 1) {
        perror("Failed to reset the read address\n");
        return 1;
    }
    char buf[BUFFER_SIZE];
    if (read(file, buf, BUFFER_SIZE) != BUFFER_SIZE) {
        perror("Failed to read in the buffer\n");
        return 1;
    }
    printf("The RTC time is %02d:%02d:%02d\n", bcdToDec(buf[2]),
        bcdToDec(buf[1]), bcdToDec(buf[0]));
    close(file);
    return 0;
}
