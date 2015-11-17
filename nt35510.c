#include "lcd_driver_if.h"

#include <stm32f4xx.h>

static void nt35510_open(struct lcd_device *dev);
static void nt35510_clear(struct lcd_device *dev);
static void nt35510_draw_rectangle(struct lcd_device *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c);
static void nt35510_close(struct lcd_device *dev);

struct lcd_opt nt35510_opt = {
	nt35510_open,
	nt35510_clear,
	nt35510_draw_rectangle,
	nt35510_close
};

struct lcd_cmd {
	uint16_t reg_addr;
	uint16_t data;
};

static struct lcd_cmd nt35510_setup_cmdmap[] = {
	{0xF000,0x55},
	{0xF001,0xAA},
	{0xF002,0x52},
	{0xF003,0x08},
	{0xF004,0x01},
	//AVDD Set AVDD 5.2V
	{0xB000,0x0D},
	{0xB001,0x0D},
	{0xB002,0x0D},
	//AVDD ratio
	{0xB600,0x34},
	{0xB601,0x34},
	{0xB602,0x34},
	//AVEE -5.2V
	{0xB100,0x0D},
	{0xB101,0x0D},
	{0xB102,0x0D},
	//AVEE ratio
	{0xB700,0x34},
	{0xB701,0x34},
	{0xB702,0x34},
	//VCL -2.5V
	{0xB200,0x00},
	{0xB201,0x00},
	{0xB202,0x00},
	//VCL ratio
	{0xB800,0x24},
	{0xB801,0x24},
	{0xB802,0x24},
	//VGH 15V (Free pump)
	{0xBF00,0x01},
	{0xB300,0x0F},
	{0xB301,0x0F},
	{0xB302,0x0F},
	//VGH ratio
	{0xB900,0x34},
	{0xB901,0x34},
	{0xB902,0x34},
	//VGL_REG -10V
	{0xB500,0x08},
	{0xB501,0x08},
	{0xB502,0x08},
	{0xC200,0x03},
	//VGLX ratio
	{0xBA00,0x24},
	{0xBA01,0x24},
	{0xBA02,0x24},
	//VGMP/VGSP 4.5V/0V
	{0xBC00,0x00},
	{0xBC01,0x78},
	{0xBC02,0x00},
	//VGMN/VGSN -4.5V/0V
	{0xBD00,0x00},
	{0xBD01,0x78},
	{0xBD02,0x00},
	//VCOM
	{0xBE00,0x00},
	{0xBE01,0x64},
	//Gamma Setting
	{0xD100,0x00},
	{0xD101,0x33},
	{0xD102,0x00},
	{0xD103,0x34},
	{0xD104,0x00},
	{0xD105,0x3A},
	{0xD106,0x00},
	{0xD107,0x4A},
	{0xD108,0x00},
	{0xD109,0x5C},
	{0xD10A,0x00},
	{0xD10B,0x81},
	{0xD10C,0x00},
	{0xD10D,0xA6},
	{0xD10E,0x00},
	{0xD10F,0xE5},
	{0xD110,0x01},
	{0xD111,0x13},
	{0xD112,0x01},
	{0xD113,0x54},
	{0xD114,0x01},
	{0xD115,0x82},
	{0xD116,0x01},
	{0xD117,0xCA},
	{0xD118,0x02},
	{0xD119,0x00},
	{0xD11A,0x02},
	{0xD11B,0x01},
	{0xD11C,0x02},
	{0xD11D,0x34},
	{0xD11E,0x02},
	{0xD11F,0x67},
	{0xD120,0x02},
	{0xD121,0x84},
	{0xD122,0x02},
	{0xD123,0xA4},
	{0xD124,0x02},
	{0xD125,0xB7},
	{0xD126,0x02},
	{0xD127,0xCF},
	{0xD128,0x02},
	{0xD129,0xDE},
	{0xD12A,0x02},
	{0xD12B,0xF2},
	{0xD12C,0x02},
	{0xD12D,0xFE},
	{0xD12E,0x03},
	{0xD12F,0x10},
	{0xD130,0x03},
	{0xD131,0x33},
	{0xD132,0x03},
	{0xD133,0x6D},
	{0xD200,0x00},
	{0xD201,0x33},
	{0xD202,0x00},
	{0xD203,0x34},
	{0xD204,0x00},
	{0xD205,0x3A},
	{0xD206,0x00},
	{0xD207,0x4A},
	{0xD208,0x00},
	{0xD209,0x5C},
	{0xD20A,0x00},

	{0xD20B,0x81},
	{0xD20C,0x00},
	{0xD20D,0xA6},
	{0xD20E,0x00},
	{0xD20F,0xE5},
	{0xD210,0x01},
	{0xD211,0x13},
	{0xD212,0x01},
	{0xD213,0x54},
	{0xD214,0x01},
	{0xD215,0x82},
	{0xD216,0x01},
	{0xD217,0xCA},
	{0xD218,0x02},
	{0xD219,0x00},
	{0xD21A,0x02},
	{0xD21B,0x01},
	{0xD21C,0x02},
	{0xD21D,0x34},
	{0xD21E,0x02},
	{0xD21F,0x67},
	{0xD220,0x02},
	{0xD221,0x84},
	{0xD222,0x02},
	{0xD223,0xA4},
	{0xD224,0x02},
	{0xD225,0xB7},
	{0xD226,0x02},
	{0xD227,0xCF},
	{0xD228,0x02},
	{0xD229,0xDE},
	{0xD22A,0x02},
	{0xD22B,0xF2},
	{0xD22C,0x02},
	{0xD22D,0xFE},
	{0xD22E,0x03},
	{0xD22F,0x10},
	{0xD230,0x03},
	{0xD231,0x33},
	{0xD232,0x03},
	{0xD233,0x6D},
	{0xD300,0x00},
	{0xD301,0x33},
	{0xD302,0x00},
	{0xD303,0x34},
	{0xD304,0x00},
	{0xD305,0x3A},
	{0xD306,0x00},
	{0xD307,0x4A},
	{0xD308,0x00},
	{0xD309,0x5C},
	{0xD30A,0x00},

	{0xD30B,0x81},
	{0xD30C,0x00},
	{0xD30D,0xA6},
	{0xD30E,0x00},
	{0xD30F,0xE5},
	{0xD310,0x01},
	{0xD311,0x13},
	{0xD312,0x01},
	{0xD313,0x54},
	{0xD314,0x01},
	{0xD315,0x82},
	{0xD316,0x01},
	{0xD317,0xCA},
	{0xD318,0x02},
	{0xD319,0x00},
	{0xD31A,0x02},
	{0xD31B,0x01},
	{0xD31C,0x02},
	{0xD31D,0x34},
	{0xD31E,0x02},
	{0xD31F,0x67},
	{0xD320,0x02},
	{0xD321,0x84},
	{0xD322,0x02},
	{0xD323,0xA4},
	{0xD324,0x02},
	{0xD325,0xB7},
	{0xD326,0x02},
	{0xD327,0xCF},
	{0xD328,0x02},
	{0xD329,0xDE},
	{0xD32A,0x02},
	{0xD32B,0xF2},
	{0xD32C,0x02},
	{0xD32D,0xFE},
	{0xD32E,0x03},
	{0xD32F,0x10},
	{0xD330,0x03},
	{0xD331,0x33},
	{0xD332,0x03},
	{0xD333,0x6D},
	{0xD400,0x00},
	{0xD401,0x33},
	{0xD402,0x00},
	{0xD403,0x34},
	{0xD404,0x00},
	{0xD405,0x3A},
	{0xD406,0x00},
	{0xD407,0x4A},
	{0xD408,0x00},
	{0xD409,0x5C},
	{0xD40A,0x00},
	{0xD40B,0x81},

	{0xD40C,0x00},
	{0xD40D,0xA6},
	{0xD40E,0x00},
	{0xD40F,0xE5},
	{0xD410,0x01},
	{0xD411,0x13},
	{0xD412,0x01},
	{0xD413,0x54},
	{0xD414,0x01},
	{0xD415,0x82},
	{0xD416,0x01},
	{0xD417,0xCA},
	{0xD418,0x02},
	{0xD419,0x00},
	{0xD41A,0x02},
	{0xD41B,0x01},
	{0xD41C,0x02},
	{0xD41D,0x34},
	{0xD41E,0x02},
	{0xD41F,0x67},
	{0xD420,0x02},
	{0xD421,0x84},
	{0xD422,0x02},
	{0xD423,0xA4},
	{0xD424,0x02},
	{0xD425,0xB7},
	{0xD426,0x02},
	{0xD427,0xCF},
	{0xD428,0x02},
	{0xD429,0xDE},
	{0xD42A,0x02},
	{0xD42B,0xF2},
	{0xD42C,0x02},
	{0xD42D,0xFE},
	{0xD42E,0x03},
	{0xD42F,0x10},
	{0xD430,0x03},
	{0xD431,0x33},
	{0xD432,0x03},
	{0xD433,0x6D},
	{0xD500,0x00},
	{0xD501,0x33},
	{0xD502,0x00},
	{0xD503,0x34},
	{0xD504,0x00},
	{0xD505,0x3A},
	{0xD506,0x00},
	{0xD507,0x4A},
	{0xD508,0x00},
	{0xD509,0x5C},
	{0xD50A,0x00},
	{0xD50B,0x81},

	{0xD50C,0x00},
	{0xD50D,0xA6},
	{0xD50E,0x00},
	{0xD50F,0xE5},
	{0xD510,0x01},
	{0xD511,0x13},
	{0xD512,0x01},
	{0xD513,0x54},
	{0xD514,0x01},
	{0xD515,0x82},
	{0xD516,0x01},
	{0xD517,0xCA},
	{0xD518,0x02},
	{0xD519,0x00},
	{0xD51A,0x02},
	{0xD51B,0x01},
	{0xD51C,0x02},
	{0xD51D,0x34},
	{0xD51E,0x02},
	{0xD51F,0x67},
	{0xD520,0x02},
	{0xD521,0x84},
	{0xD522,0x02},
	{0xD523,0xA4},
	{0xD524,0x02},
	{0xD525,0xB7},
	{0xD526,0x02},
	{0xD527,0xCF},
	{0xD528,0x02},
	{0xD529,0xDE},
	{0xD52A,0x02},
	{0xD52B,0xF2},
	{0xD52C,0x02},
	{0xD52D,0xFE},
	{0xD52E,0x03},
	{0xD52F,0x10},
	{0xD530,0x03},
	{0xD531,0x33},
	{0xD532,0x03},
	{0xD533,0x6D},
	{0xD600,0x00},
	{0xD601,0x33},
	{0xD602,0x00},
	{0xD603,0x34},
	{0xD604,0x00},
	{0xD605,0x3A},
	{0xD606,0x00},
	{0xD607,0x4A},
	{0xD608,0x00},
	{0xD609,0x5C},
	{0xD60A,0x00},
	{0xD60B,0x81},

	{0xD60C,0x00},
	{0xD60D,0xA6},
	{0xD60E,0x00},
	{0xD60F,0xE5},
	{0xD610,0x01},
	{0xD611,0x13},
	{0xD612,0x01},
	{0xD613,0x54},
	{0xD614,0x01},
	{0xD615,0x82},
	{0xD616,0x01},
	{0xD617,0xCA},
	{0xD618,0x02},
	{0xD619,0x00},
	{0xD61A,0x02},
	{0xD61B,0x01},
	{0xD61C,0x02},
	{0xD61D,0x34},
	{0xD61E,0x02},
	{0xD61F,0x67},
	{0xD620,0x02},
	{0xD621,0x84},
	{0xD622,0x02},
	{0xD623,0xA4},
	{0xD624,0x02},
	{0xD625,0xB7},
	{0xD626,0x02},
	{0xD627,0xCF},
	{0xD628,0x02},
	{0xD629,0xDE},
	{0xD62A,0x02},
	{0xD62B,0xF2},
	{0xD62C,0x02},
	{0xD62D,0xFE},
	{0xD62E,0x03},
	{0xD62F,0x10},
	{0xD630,0x03},
	{0xD631,0x33},
	{0xD632,0x03},
	{0xD633,0x6D},
	//LV2 Page 0 enable
	{0xF000,0x55},
	{0xF001,0xAA},
	{0xF002,0x52},
	{0xF003,0x08},
	{0xF004,0x00},
	//Display control
	{0xB100, 0xCC},
	{0xB101, 0x00},
	//Source hold time
	{0xB600,0x05},
	//Gate EQ control
	{0xB700,0x70},
	{0xB701,0x70},
	//Source EQ control (Mode 2)
	{0xB800,0x01},
	{0xB801,0x03},
	{0xB802,0x03},
	{0xB803,0x03},
	//Inversion mode (2-dot)
	{0xBC00,0x02},
	{0xBC01,0x00},
	{0xBC02,0x00},
	//Timing control 4H w/ 4-delay
	{0xC900,0xD0},
	{0xC901,0x02},
	{0xC902,0x50},
	{0xC903,0x50},
	{0xC904, 0x50},
	{0x3500, 0x00},
	{0x3A00, 0x55},  //16-bit/pixel
	{0x1100, 0xFF},
	{0x2900, 0xFF}
};

void nt35510_open(struct lcd_device *dev)
{
	uint32_t i;
	for (i = 0; i < sizeof(nt35510_setup_cmdmap); i++) {
		dev->dev->cmd_reg = nt35510_setup_cmdmap[i].reg_addr;
		if (0xFF != nt35510_setup_cmdmap[i].data) {
			dev->dev->dat_reg = nt35510_setup_cmdmap[i].data;
		}
	}
	
	GPIOB->ODR |= GPIO_ODR_ODR_15;
}

void nt35510_clear(struct lcd_device *dev)
{
	uint32_t i;
	dev->dev->cmd_reg = 0x2A00;
	dev->dev->dat_reg = 0;
	dev->dev->cmd_reg = 0x2A00+1;
	dev->dev->dat_reg = 0;
	dev->dev->cmd_reg = 0x2A00+2;
	dev->dev->dat_reg = (dev->width - 1) >> 8;
	dev->dev->cmd_reg = 0x2A00+3;
	dev->dev->dat_reg = (dev->width - 1) & 0XFF;
	dev->dev->cmd_reg = 0x2B00;
	dev->dev->dat_reg = 0;
	dev->dev->cmd_reg = 0x2B00+1;
	dev->dev->dat_reg = 0;
	dev->dev->cmd_reg = 0x2B00+2;
	dev->dev->dat_reg = (dev->height - 1) >> 8;
	dev->dev->cmd_reg = 0x2B00+3;
	dev->dev->dat_reg = (dev->height - 1) & 0XFF;
	
	dev->dev->cmd_reg = 0X2C00;
	
	for (i = 0; i < dev->width * dev->height; i++) {
		dev->dev->dat_reg = 0x0000;
	}
}

void nt35510_draw_rectangle(struct lcd_device *dev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t c) 
{ 
 	uint16_t i; 
 	dev->dev->cmd_reg = 0x2A00; 
 	dev->dev->dat_reg = x >> 8; 
 	dev->dev->cmd_reg = 0x2A00+1; 
 	dev->dev->dat_reg = x & 0xFF; 
	dev->dev->cmd_reg = 0x2A00+2; 
	dev->dev->dat_reg = (x + w) >> 8; 
	dev->dev->cmd_reg = 0x2A00+3; 
	dev->dev->dat_reg = (x + w) & 0XFF; 
	dev->dev->cmd_reg = 0x2B00; 
 	dev->dev->dat_reg = y >> 8; 
	dev->dev->cmd_reg = 0x2B00+1; 
	dev->dev->dat_reg = y & 0xFF; 
	dev->dev->cmd_reg = 0x2B00+2; 
	dev->dev->dat_reg = (y + h) >> 8; 
	dev->dev->cmd_reg = 0x2B00+3; 
	dev->dev->dat_reg = (y + h) & 0XFF; 
	 
	dev->dev->cmd_reg = 0X2C00; 
	 
	for (i = 0; i <= (w + 1) * (h + 1); i++) { 
		dev->dev->dat_reg = c; 
	} 
} 


void nt35510_close(struct lcd_device *dev)
{

}
