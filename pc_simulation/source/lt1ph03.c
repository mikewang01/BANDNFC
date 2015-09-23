
#include "main.h"

static I32S ppg_timer = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// Internal sequencer, through a Command Protocol
//   1. COMMAND
//   2. RESPONSE
//   3. PARAM_WR
//   4. PARAM_RD

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// PARAM_QUERY                 8'b100a aaaa
// PARAM_SET                   8'b101a aaaa
// PARAM_AND                   8'b110a aaaa
// PARAM_OR                    8'b111a aaaa
// NOP                         8'b0000 0000
// RESET                       8'b0000 0001
// BUSADDR                     8'b0000 0010
// PS_FORCE                    8'b0000 0101
// ALS_FORCE                   8'b0000 0110
// PSALS_FORCE                 8'b0000 0111
// PS_PAUSE                    8'b0000 1001
// ALS_PAUSE                   8'b0000 1010
// PSALS_PAUSE                 8'b0000 1011
// PS_AUTO                     8'b0000 1101
// ALS_AUTO                    8'b0000 1110
// PSALS_PAUSE                 8'b0000 1111

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define I2C_ADDR               0x00
#define CHLIST                 0x01        // (EN_AUX, EN_ALS_IR, EN_ALS_VIS, EN_PS1)
#define PS_ENCODING            0x05        // (PS1_ALIGN)
#define ALS_ENCODING           0x06        // (ALS_IR_ALIGN, ALS_VIS_ALIGN)
#define PS1_ADCMUX             0x07        // (PS1 ADC Input Selection)
#define PS_ADC_COUNTER         0x0A        // (PS_ADC_REC)
#define PS_ADC_GAIN            0x0B        // (PS_ADC_GAIN)
#define PS_ADC_MISC            0x0C        // (PS_RANGE, PS_ADC_MODE)
#define ALS_IR_ADCMUX          0x0E        // (ALS_IR_ADCMUX)
#define AUX_ADCMUX             0x0F        // (AUX ADC Input Select)
#define ALS_VIS_ADC_COUNTER    0x10        // (VIS_ADC_REC)
#define ALS_VIS_ADC_GAIN       0x11        // (ALS_VIS_ADC_GAIN)
#define ALS_VIS_ADC_MISC       0x12        // (VIS_RANGE)
#define ALS_HYST               0x16        // (ALS Hysteresis)
#define PS_HYST                0x17        // (PS Hysteresis)
#define PS_HISTORY             0x18        // (PS History Setting)
#define ALS_HISTORY            0x19        // (ALS History Setting)
#define ADC_OFFSET             0x1A        // (ADC Offset)
#define LED_REC                0x1C        // (LED Recovery Time)
#define ALS_IR_ADC_COUNTER     0x1D        // (IR_ADC_REC)
#define ALS_IR_ADC_GAIN        0x1E        // (ALS_IR_ADC_GAIN)
#define ALS_IR_ADC_MISC        0x1F        // (IR_RANGE)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

static EN_STATUSCODE LT1PH03_read_reg(I8U cmdID, I8U bytes, I8U *pRegVal)
{
	bool bTransfer;
	
	if (pRegVal==NULL) return STATUSCODE_NULL_POINTER;
	
	bTransfer  = twi_master_transfer(LT1PH03_I2C_ADDR, &cmdID, 1, TWI_DONT_ISSUE_STOP, TWI_MASTER_0);
	bTransfer &= twi_master_transfer(LT1PH03_I2C_ADDR|TWI_READ_BIT, pRegVal, bytes, TWI_ISSUE_STOP, TWI_MASTER_0);

	if (bTransfer) {
		N_SPRINTF("Read PASS: 0x%02x", cmdID);
		return STATUSCODE_SUCCESS;
	} else {
		N_SPRINTF("Read FAIL: 0x%02x", cmdID);
		return STATUSCODE_FAILURE;
	}
}

static BOOLEAN LT1PH03_write_reg(I8U cmdID, I8U regVal)
{
	bool bTransfer;

	I8U acData[2];
	
	acData[0] = cmdID;
	acData[1] = regVal;
	
	bTransfer =  twi_master_transfer(LT1PH03_I2C_ADDR, acData, 2, TWI_ISSUE_STOP, TWI_MASTER_0);

	if (bTransfer) {
		N_SPRINTF("Write PASS: 0x%02x  0x%02x", cmdID, regVal);
		nrf_delay_ms(1);
		return STATUSCODE_SUCCESS;
	} else {
		N_SPRINTF("Write FAIL: 0x%02x  0x%02x", cmdID, regVal);
		return STATUSCODE_FAILURE;
	}
}

/*------------ !!! BASIC SUBROUTINE FOR LT1PH03 DRIVER START !!! ------------*/
void LT1PH03_Send_Command(I8U cmd)      { LT1PH03_write_reg(REGS_COMMAND, cmd); }
void LT1PH03_Send_Parameter(I8U param)  { LT1PH03_write_reg(REGS_PARAM_WR, param); }
void LT1PH03_Send_Command_Parameter(I8U cmd, I8U param)
{
	LT1PH03_Send_Parameter(param);
	LT1PH03_Send_Command(cmd);
}
void LT1PH03_Send_Command_Query(I8U offset, I8U param) { LT1PH03_Send_Command_Parameter((offset+0x80), param); }
void LT1PH03_Send_Command_SET  (I8U offset, I8U param) { LT1PH03_Send_Command_Parameter((offset+0xA0), param); }
void LT1PH03_Send_Command_AND  (I8U offset, I8U param) { LT1PH03_Send_Command_Parameter((offset+0xC0), param); }
void LT1PH03_Send_Command_OR   (I8U offset, I8U param) { LT1PH03_Send_Command_Parameter((offset+0xE0), param); }
void LT1PH03_Send_Command_NOP()    { LT1PH03_Send_Command(0x00); }
void LT1PH03_Send_Command_Reset()  { LT1PH03_Send_Command(0x01); }
void LT1PH03_PS_Force()            { LT1PH03_Send_Command(0x05); }
void LT1PH03_ALS_Force()           { LT1PH03_Send_Command(0x06); }
void LT1PH03_PS_ALS_Force()        { LT1PH03_Send_Command(0x07); }
void LT1PH03_PS_Pause()            { LT1PH03_Send_Command(0x09); }
void LT1PH03_ALS_Pause()           { LT1PH03_Send_Command(0x0a); }
void LT1PH03_PS_ALS_Pause()        { LT1PH03_Send_Command(0x0B); }
void LT1PH03_PS_Auto()             { LT1PH03_Send_Command(0x0D); }
void LT1PH03_ALS_Auto()            { LT1PH03_Send_Command(0x0E); }
void LT1PH03_PS_ALS_Auto()         { LT1PH03_Send_Command(0x0F); }
void LT1PH03_Modify_Command_I2C_Addr(I8U i2c_addr)
{
	LT1PH03_Send_Command_SET(I2C_ADDR, i2c_addr);
	LT1PH03_Send_Command(0x02);
}
/*------------ !!! BASIC SUBROUTINE FOR LT1PH03 DRIVER END   !!! ------------*/

void LT1PH03_Init_Sensor()
{
  I8U reg_defaults[] = {
  	0x00,                                                                 // part id
	  0x00,                                                                 // rev id
	  0x00,                                                                 // seq id
  	B_INT_STICKY      | B_INT_OUTPUT_DISABLE,                             // int cfg
  	B_CMD_INT_ENABLE  | B_PS1_INT_ENBALE       | B_ALS_INT_DISABLE,       // irq enable
  	B_PS1_IM_COMPLETE | B_ALS_IM_NONE,                                    // int mode1
  	B_CMD_INT_ERR,                                                        // int mode2
  	0x17,                                                                 // hw key
	  B_MEAS_RATE_10MS,                                                     // meas rate
  	B_ALS_RATE_FORCE,                                                     // als rate
  	B_PS_RATE_1,                                                          // ps rate
	  0x00,                                                                 // als low  th  7:0
  	0x00,                                                                 // als low  th 15:8
  	0x00,                                                                 // als high th  7:0
  	0x00,                                                                 // als high th 15:8
//  B_PS_LED_NONE,                                                        // ps led drive current
//  B_PS_LED_5_6MA,
//  B_PS_LED_11_2MA,
//  B_PS_LED_22_4MA,
    B_PS_LED_44_8MA,  
  };

	LT1PH03_Send_Command_Reset();	
	nrf_delay_ms(50);

  LT1PH03_write_reg(REGS_HW_KEY, reg_defaults[REGS_HW_KEY]);

	LT1PH03_Send_Command_SET(CHLIST,0x01);        //  enble ps only
	
	LT1PH03_write_reg(REGS_INT_CFG,    reg_defaults[REGS_INT_CFG]);
	LT1PH03_write_reg(REGS_IRQ_ENABLE, reg_defaults[REGS_IRQ_ENABLE]);
	LT1PH03_write_reg(REGS_IRQ_MODE1,  reg_defaults[REGS_IRQ_MODE1]);
	LT1PH03_write_reg(REGS_IRQ_MODE2,  reg_defaults[REGS_IRQ_MODE2]);
	LT1PH03_write_reg(REGS_MEAS_RATE,  reg_defaults[REGS_MEAS_RATE]);
	LT1PH03_write_reg(REGS_ALS_RATE,   reg_defaults[REGS_ALS_RATE]);
	LT1PH03_write_reg(REGS_PS_RATE,    reg_defaults[REGS_PS_RATE]);
	LT1PH03_write_reg(REGS_PS_LED21,   reg_defaults[REGS_PS_LED21]);
	
  SYSCLK_timer_start();
//PS_ALS_Force();
}

void LT1PH03_Enable_Sensor()
{
	LT1PH03_write_reg(REGS_IRQ_STATUS, 0x27);  // Clear all int
	LT1PH03_write_reg(REGS_INT_CFG, B_INT_STICKY|B_INT_OUTPUT_ENABLE);
	
	LT1PH03_Send_Command_SET(CHLIST,0x01);         // Enble ps only
  LT1PH03_PS_Auto();

#if 1
	// TBD: Need Tuning...
//LT1PH03_Send_Command_SET(PS_ENCODING, 0x60);       // LSB 16 bits of 17bit ADC      // 0x60
  LT1PH03_Send_Command_SET(PS_ADC_GAIN, 0x03);       // PS_ADC_GAIN
  LT1PH03_Send_Command_SET(PS1_ADCMUX,  0x03);       // 
  LT1PH03_Send_Command_SET(PS_ADC_MISC, 0x04);       // PS_ADC_GAIN
#endif
	
//LT1PH03_Send_Command_SET(ALS_ENCODING, 0x30);
//LT1PH03_Send_Command_SET(PS1_ADCMUX, 0x00);
}

void LT1PH03_Init()
{
	I8U partid = 0xaa;
	I8U revid  = 0xbb;
	I8U seqid  = 0xcc;

  twi_master_init(TWI_MASTER_0);
	nrf_delay_ms(30);

	LT1PH03_Init_Sensor();
  LT1PH03_Enable_Sensor();

	LT1PH03_read_reg(REGS_PART_ID, 1, &partid);
	LT1PH03_read_reg(REGS_REV_ID,  1, &revid);
	LT1PH03_read_reg(REGS_SEQ_ID,  1, &seqid);
	
	ppg_timer = CLK_get_system_time();
	Y_SPRINTF("LT1PH03 0x%02x  0x%02x  0x%02x", partid, revid, seqid);

	GPIO_twi_disabled(TWI_MASTER_0);
}

void LT1PH03_Measure()
{
	I8U  buf[2];
	I32S proxData;
	I32S t_curr, t_diff;	
	
	twi_master_init(TWI_MASTER_0);
	nrf_delay_ms(30);
	
	N_SPRINTF("test2.");
  SYSCLK_timer_start();

	t_curr = CLK_get_system_time();
	t_diff = t_curr - ppg_timer;
  if (t_diff>20)  {    // 30Hz
		ppg_timer = t_curr;
	N_SPRINTF("test3.");

#if 1
//LT1PH03_Send_Command_NOP();
//LT1PH03_Send_Command_NOP();
//LT1PH03_PS_ALS_Force();
	
  LT1PH03_read_reg(REGS_PS1_DATA0,  1, (buf+0));
  LT1PH03_read_reg(REGS_PS1_DATA1,  1, (buf+1));

//proxData = buf[1];
//proxData <<= 8;
//proxData |= buf[0];

  proxData = ((I32S)buf[1]<<8) | buf[0];	
  I_SPRINTF("%d", proxData);
  N_SPRINTF("%d  0x%04x  0x%02x  0x%02x  ", proxData, proxData, buf[1], buf[0]);
  N_SPRINTF("0x%02x  0x%02x  ", buf[1], buf[0]);
#endif
	}
	GPIO_twi_disabled(TWI_MASTER_0);
}

