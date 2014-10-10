#include <unistd.h>

#include "bcm2835.h"

#define RC522_SPI_RESET RPI_V2_GPIO_P1_22 /* GPIO 25 pin 22  */

//MF522 command
#define PCD_IDLE              0x00
#define PCD_AUTHENT           0x0E
#define PCD_RECEIVE           0x08
#define PCD_TRANSMIT          0x04
#define PCD_TRANSCEIVE        0x0C
#define PCD_RESETPHASE        0x0F
#define PCD_CALCCRC           0x03

//Mifare_One
#define PICC_REQIDL           0x26
#define PICC_REQALL           0x52
#define PICC_ANTICOLL1        0x93
#define PICC_ANTICOLL2        0x95
#define PICC_ANTICOLL3        0x97
#define PICC_AUTHENT1A        0x60
#define PICC_AUTHENT1B        0x61
#define PICC_READ             0x30
#define PICC_WRITE            0xA0
#define PICC_DECREMENT        0xC0
#define PICC_INCREMENT        0xC1
#define PICC_RESTORE          0xC2
#define PICC_TRANSFER         0xB0
#define PICC_HALT             0x50

//MF522 FIFO
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte
#define MAXRLEN  18

//MF522 registers
#define     CommandReg            0x01
#define     ComIEnReg             0x02
#define     DivlEnReg             0x03
#define     ComIrqReg             0x04
#define     DivIrqReg             0x05
#define     ErrorReg              0x06
#define     Status1Reg            0x07
#define     Status2Reg            0x08
#define     FIFODataReg           0x09
#define     FIFOLevelReg          0x0A
#define     WaterLevelReg         0x0B
#define     ControlReg            0x0C
#define     BitFramingReg         0x0D
#define     CollReg               0x0E

#define     ModeReg               0x11
#define     TxModeReg             0x12
#define     RxModeReg             0x13
#define     TxControlReg          0x14
#define     TxASKReg              0x15
#define     TxSelReg              0x16
#define     RxSelReg              0x17
#define     RxThresholdReg        0x18
#define     DemodReg              0x19
#define     MifareReg             0x1C
#define     SerialSpeedReg        0x1F

#define     CRCResultRegM         0x21
#define     CRCResultRegL         0x22
#define     ModWidthReg           0x24
#define     RFCfgReg              0x26
#define     GsNReg                0x27
#define     CWGsCfgReg            0x28
#define     ModGsCfgReg           0x29
#define     TModeReg              0x2A
#define     TPrescalerReg         0x2B
#define     TReloadRegH           0x2C
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E
#define     TCounterValueRegL     0x2F

#define     TestSel1Reg           0x31
#define     TestSel2Reg           0x32
#define     TestPinEnReg          0x33
#define     TestPinValueReg       0x34
#define     TestBusReg            0x35
#define     AutoTestReg           0x36
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39
#define     TestDAC2Reg           0x3A
#define     TestADCReg            0x3B

#define MAX_RLEN 16

enum tag_status_t {
	TAG_OK,
	TAG_NOTAG,
	TAG_ERROR
};

enum interface_t {
	IF_SPI,
	IF_I2C,
	IF_NOT_SET
};

class Reader2
{
	public:
		Reader2();
		int init_spi(int8_t RST = RC522_SPI_RESET);
		int init_i2c();
		int read_card();
	private:
		interface_t _if_type;

		int init();
		interface_t get_interface();
		uint8_t request(uint8_t req_mode, uint8_t *tag_type);
		uint8_t anticoll(uint8_t *);
		uint8_t send_to_card(uint8_t command, uint8_t *data, uint8_t data_len, uint8_t *buf, uint8_t *buf_len);
		void antenna(bool on);
		void write(uint8_t addr, uint8_t data);
		uint8_t read(uint8_t addr);
		void set_bitmask(uint8_t addr, uint8_t mask);
		void clear_bitmask(uint8_t addr, uint8_t mask);
};
