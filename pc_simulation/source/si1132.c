
#include "main.h"

static EN_STATUSCODE SI1132_read_reg(I8U cmdID, I8U bytes, I8U *pRegVal)
{
	bool bTransfer;
	
	if (pRegVal==NULL) return STATUSCODE_NULL_POINTER;
	
	bTransfer  = twi_master_transfer(SI1132_I2C_ADDR, &cmdID, 1, TWI_DONT_ISSUE_STOP, TWI_MASTER_0);
	bTransfer &= twi_master_transfer(SI1132_I2C_ADDR|TWI_READ_BIT, pRegVal, bytes, TWI_ISSUE_STOP, TWI_MASTER_0);

	if (bTransfer) {
		N_SPRINTF("Read PASS: 0x%02x", cmdID);
		return STATUSCODE_SUCCESS;
	} else {
		N_SPRINTF("Read FAIL: 0x%02x", cmdID);
		return STATUSCODE_FAILURE;
	}
}

static EN_STATUSCODE SI1132_write_reg(I8U cmdID, I8U regVal)
{
	bool bTransfer;

	I8U acData[2];
	
	acData[0] = cmdID;
	acData[1] = regVal;
	
	bTransfer =  twi_master_transfer(SI1132_I2C_ADDR, acData, 2, TWI_ISSUE_STOP, TWI_MASTER_0);

	if (bTransfer) {
		N_SPRINTF("Write PASS: 0x%02x  0x%02x", cmdID, regVal);
		return STATUSCODE_SUCCESS;
	} else {
		N_SPRINTF("Write FAIL: 0x%02x  0x%02x", cmdID, regVal);
		return STATUSCODE_FAILURE;
	}
}

/*------------ !!! BASIC SUBROUTINE FOR SI1132 DRIVER START !!! ------------*/
void SI1132_Send_Command(I8U cmd)      { SI1132_write_reg(SI1132_REGS_COMMAND, cmd); }
void SI1132_Send_Parameter(I8U param)  { SI1132_write_reg(SI1132_REGS_PARAM_WR, param); }
void SI1132_Send_Command_Parameter(I8U cmd, I8U param)
{
	SI1132_Send_Parameter(param);
	SI1132_Send_Command(cmd);
}
void SI1132_Send_Command_Query(I8U offset, I8U param) { SI1132_Send_Command_Parameter((offset+0x80), param); }
void SI1132_Send_Command_SET  (I8U offset, I8U param) { SI1132_Send_Command_Parameter((offset+0xA0), param); }    /* 
void SI1132_Send_Command_AND  (I8U offset, I8U param) { SI1132_Send_Command_Parameter((offset+0xC0), param); }
void SI1132_Send_Command_OR   (I8U offset, I8U param) { SI1132_Send_Command_Parameter((offset+0xE0), param); }    */
void SI1132_Send_Command_NOP()                        { SI1132_Send_Command(0x00); }
void SI1132_Send_Command_Reset()                      { SI1132_Send_Command(0x01); }
void SI1132_ALS_Force()                               { SI1132_Send_Command(0x06); }    // Forces a single ALS measurement
void SI1132_ALS_Pause()                               { SI1132_Send_Command(0x0a); }    // Pauses autonomous ALS
void SI1132_ALS_Auto()                                { SI1132_Send_Command(0x0E); }    /* 
void SI1132_PS_Force()                                { SI1132_Send_Command(0x05); }
void SI1132_PS_ALS_Force()                            { SI1132_Send_Command(0x07); }
void SI1132_PS_Pause()                                { SI1132_Send_Command(0x09); }
void SI1132_PS_ALS_Pause()                            { SI1132_Send_Command(0x0B); }
void SI1132_PS_Auto()                                 { SI1132_Send_Command(0x0D); }
void SI1132_PS_ALS_Auto()                             { SI1132_Send_Command(0x0F); }    */
void SI1132_Get_Cal()                                 { SI1132_Send_Command(0x12); }    // Reports calibration data to I2C registers 0x22 to 0x2D
void SI1132_Modify_Command_I2C_Addr(I8U i2c_addr)
{
  SI1132_Send_Command_SET(SI1132_PARAM_I2C_ADDR, i2c_addr);
  SI1132_Send_Command(0x02);
}
/*------------ !!! BASIC SUBROUTINE FOR SI1132 DRIVER END   !!! ------------*/

void SI1132_Reset()
{
  SI1132_write_reg(SI1132_REGS_MEAS_RATE0, 0x00);
  SI1132_write_reg(SI1132_REGS_MEAS_RATE1, 0x00);
	
	SI1132_ALS_Pause();

  SI1132_write_reg(SI1132_REGS_MEAS_RATE0, 0x00);
  SI1132_write_reg(SI1132_REGS_MEAS_RATE1, 0x00);
	SI1132_write_reg(SI1132_REGS_IRQ_ENABLE, 0x00);
	SI1132_write_reg(SI1132_REGS_INT_CFG,    0x00);
	SI1132_write_reg(SI1132_REGS_IRQ_STATUS, 0xFF);
	
	SI1132_Send_Command_Reset();
	nrf_delay_ms(10);

	/* Hardware Reset. */
  SI1132_write_reg(SI1132_REGS_HW_KEY, 0x17);
}

static I32S si1132_timer = 0;
void SI1132_Init()
{
	I8U partid = 0xaa;
	I8U revid  = 0xbb;
	I8U seqid  = 0xcc;
	
	I8U coef0 = 0x11;
	I8U coef1 = 0x22;
	I8U coef2 = 0x33;
	I8U coef3 = 0x44;

	twi_master_init(TWI_MASTER_0);
	nrf_delay_ms(30);

	SI1132_Reset();

//SI1132_write_reg(SI1132_REGS_HW_KEY, 0x17);
	
	/* Set CHLIST. */
//SI1132_Send_Command_SET(SI1132_PARAM_CHLIST, 0x80);      // Enable UV Index only.
  SI1132_Send_Command_SET(SI1132_PARAM_CHLIST, 0x90);

	/* Set measure rate.                                                          */
	/* MEAS_RATE1 and MEAS_RATE0 together form a 16-bit value: MEAS_RATE[15:0].   */
	/* The 16-bit value, when multiplied by 31.25us, represents the time duration */
	/* between wake-up periods where measurements are made. Once the device wakes */
	/* up, all measurements specified in CHLIST are made                          */
  SI1132_write_reg(SI1132_REGS_MEAS_RATE0, 0x80);        // 50 samples per second
  SI1132_write_reg(SI1132_REGS_MEAS_RATE1, 0x02);
//SI1132_write_reg(SI1132_REGS_MEAS_RATE0, 0x00);
//SI1132_write_reg(SI1132_REGS_MEAS_RATE1, 0x00);

	/* Verify SI1132 ID registers. */
	SI1132_read_reg(REGS_PART_ID, 1, &partid);
	SI1132_read_reg(REGS_REV_ID,  1, &revid);
	SI1132_read_reg(REGS_SEQ_ID,  1, &seqid);	
  Y_SPRINTF("SI1132  0x%02x  0x%02x  0x%02x", partid, revid, seqid);
	nrf_delay_ms(10);

	/* Set Defualt Coefficient. */
	SI1132_read_reg(SI1132_REGS_MEAS_UCOEF0, 1, &coef0);
	SI1132_read_reg(SI1132_REGS_MEAS_UCOEF1, 1, &coef1);
	SI1132_read_reg(SI1132_REGS_MEAS_UCOEF2, 1, &coef2);
	SI1132_read_reg(SI1132_REGS_MEAS_UCOEF3, 1, &coef3);
  Y_SPRINTF("SI1132  0x%02x  0x%02x  0x%02x  0x%02x", coef0, coef1, coef2, coef3);

	
	SI1132_write_reg(SI1132_REGS_MEAS_UCOEF0, 0x7B);
	SI1132_write_reg(SI1132_REGS_MEAS_UCOEF1, 0x6B);
	SI1132_write_reg(SI1132_REGS_MEAS_UCOEF2, 0x01);
	SI1132_write_reg(SI1132_REGS_MEAS_UCOEF3, 0x00);

// TBD: debuggging...
	SI1132_write_reg(SI1132_PARAM_ALS_VIS_ADC_MISC, 0x20);
	SI1132_write_reg(SI1132_PARAM_ALS_IR_ADC_MISC,  0x20);

	/* Set SI1132 autonomous mode. */
  SI1132_ALS_Auto();
//SI1132_ALS_Force();
	SYSCLK_timer_start();
	
	si1132_timer = CLK_get_system_time();

#if 0
  I8U vis_d0, vis_d1;
  I8U ir_d0,  ir_d1;
	I8U uv_d0,  uv_d1;
	
	I16S vis;
	I16S ir;
	I16S uv;

  SI1132_Get_Cal();
	nrf_delay_ms(10);

	SI1132_read_reg(SI1132_REGS_ALS_VIS_DATA0, 1, &vis_d0);
	SI1132_read_reg(SI1132_REGS_ALS_VIS_DATA1, 1, &vis_d1);
	SI1132_read_reg(SI1132_REGS_ALS_IR_DATA0,  1, &ir_d0);
	SI1132_read_reg(SI1132_REGS_ALS_IR_DATA1,  1, &ir_d1);
	SI1132_read_reg(SI1132_REGS_AUX_DATA0_UVIDX0,  1, &uv_d0);
	SI1132_read_reg(SI1132_REGS_AUX_DATA1_UVIDX1,  1, &uv_d1);

	vis = vis_d1 * 256 + vis_d0;
	ir  = ir_d1  * 256 + ir_d0;
	uv  = uv_d1  * 256 + uv_d0;
	
	Y_SPRINTF("%04x %02x %02x", vis, vis_d1, vis_d0);
	Y_SPRINTF("%04x %02x %02x", ir,  ir_d1,  ir_d0);
	Y_SPRINTF("%04x %02x %02x", uv,  uv_d1,  uv_d0);
#endif

  GPIO_twi_disabled(TWI_MASTER_0);
}

void SI1132_Measure()
{
	I8U  buf[4];
	I8U  chip_stat;
	I16S proxData;
	I32S t_curr, t_diff;
	
	buf[0] = 0x11;
	buf[1] = 0x22;
	buf[2] = 0x33;
	buf[3] = 0x44;

	twi_master_init(TWI_MASTER_0);
	nrf_delay_ms(30);

	t_curr = CLK_get_system_time();
	t_diff = t_curr - si1132_timer;

	if (t_diff>333) {
		si1132_timer = t_curr;

#if 1
		SI1132_read_reg(SI1132_REGS_AUX_DATA0_UVIDX0,  1, (buf+0));
		SI1132_read_reg(SI1132_REGS_AUX_DATA0_UVIDX0,  1, (buf+1));
		SI1132_read_reg(SI1132_REGS_AUX_DATA1_UVIDX1,  1, (buf+2));

		proxData = buf[2] * 256 + buf[1];
		N_SPRINTF("0x%02x  0x%02x    0x%02x", buf[2], buf[1], buf[0]);
		N_SPRINTF("%d", proxData);
#endif


#if 0
		SI1132_read_reg(SI1132_REGS_ALS_VIS_DATA0,  1, (buf+0));
		SI1132_read_reg(SI1132_REGS_ALS_VIS_DATA0,  1, (buf+1));
		SI1132_read_reg(SI1132_REGS_ALS_VIS_DATA1,  1, (buf+2));
		SI1132_read_reg(SI1132_REGS_RESPONSE,       1, (buf+3));
		
		proxData = buf[2] * 256 + buf[1];
		
		N_SPRINTF("0x%02x  0x%02x  0x%02x  0x%02x", buf[3], buf[2], buf[1], buf[0]);
		N_SPRINTF("%d", proxData);
#endif
		
// 	SI1132_ALS_Force();
		
		chip_stat = 0x55;
		SI1132_read_reg(SI1132_REGS_CHIP_STAT,  1, &chip_stat);
		N_SPRINTF("0x%02x", chip_stat);
	}

	GPIO_twi_disabled(TWI_MASTER_0);
	SYSCLK_timer_start();
}


