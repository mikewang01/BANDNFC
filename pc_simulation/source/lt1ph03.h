#ifndef LT1PH03_H
#define LT1PH03_H

#define LT1PH03_I2C_ADDR    		(0xB4)            // 0x5A

#define REGS_PART_ID            (0x00)
#define REGS_REV_ID             (0x01)
#define REGS_SEQ_ID             (0x02)
#define REGS_INT_CFG            (0x03)
#define REGS_IRQ_ENABLE		      (0x04)
#define REGS_IRQ_MODE1		      (0x05)
#define REGS_IRQ_MODE2		      (0x06)
#define REGS_HW_KEY             (0x07)
#define REGS_MEAS_RATE		      (0x08)
#define REGS_ALS_RATE           (0x09)
#define REGS_PS_RATE            (0x0A)
#define REGS_ALS_LOW_TH0	      (0x0B)
#define REGS_ALS_LOW_TH1	      (0x0C)
#define REGS_ALS_HI_TH0		      (0x0D)
#define REGS_ALS_HI_TH1		      (0x0E)
#define REGS_PS_LED21		        (0x0F)
#define REGS_PS_LED3		        (0x10)
#define REGS_PS1_TH0		        (0x11)
#define REGS_PS1_TH1		        (0x12)
#define REGS_PS2_TH			        (0x13)
#define REGS_PS3_TH			        (0x15)
#define REGS_PARAM_WR		        (0x17)
#define REGS_COMMAND		        (0x18)
#define REGS_RESPONSE		        (0x20)
#define REGS_IRQ_STATUS		      (0x21)
#define REGS_ALS_VIS_DATA0	    (0x22)
#define REGS_ALS_VIS_DATA1	    (0x23)
#define REGS_ALS_IR_DATA0	      (0x24)
#define REGS_ALS_IR_DATA1	      (0x25)
#define REGS_PS1_DATA0		      (0x26)
#define REGS_PS1_DATA1		      (0x27)
#define REGS_PS2_DATA0		      (0x28)
#define REGS_PS2_DATA1		      (0x29)
#define REGS_PS3_DATA0		      (0x2A)
#define REGS_PS3_DATA1		      (0x2B)
#define REGS_AUX_DATA0		      (0x2C)
#define REGS_AUX_DATA1		      (0x2D)
#define REGS_PARAM_RD		        (0x2E)
#define REGS_CHIP_STAT		      (0x30)

#define B_INT_STICKY			      (0x00)
#define B_INT_AUTO_CLEAR		    (0x02)
#define B_INT_OUTPUT_DISABLE    (0x00)
#define B_INT_OUTPUT_ENABLE	    (0x01)
                                
#define B_CMD_INT_ENABLE		    (0x20)
#define B_PS1_INT_ENBALE		    (0x04)
#define B_ALS_INT_DISABLE		    (0x00)
                                
#define B_PS1_IM_COMPLETE		    (0x00)
#define B_PS1_IM_CROSS_TH		    (0x10)                  // (1UL<<4)
#define B_PS1_IM_GT_TH			    (0x30)                  // (3UL<<4)
#define B_ALS_IM_NONE			      (0x00)
                                
#define B_CMD_INT_ANY			      (0x00)
#define B_CMD_INT_ERR			      (0x04)
                                
																
#define B_MEAS_RATE_FORCE       (0x00)
#define B_MEAS_RATE_3_375MS	    (0x6B)
#define B_MEAS_RATE_10MS		    (0x84)
#define B_MEAS_RATE_20MS		    (0x94)
#define B_MEAS_RATE_100MS		    (0xB9)
#define B_MEAS_RATE_496MS		    (0xDF)
#define B_MEAS_RATE_1984MS	    (0xFF)
                                
#define B_ALS_RATE_FORCE		    (0x00)
#define B_ALS_RATE_1		  	    (0x08)
#define B_ALS_RATE_10			      (0x32)
#define B_ALS_RATE_100			    (0x69)
                                
#define B_PS_RATE_FORCE			    (0x00)
#define B_PS_RATE_1			  	    (0x08)
#define B_PS_RATE_10			      (0x32)
#define B_PS_RATE_100			      (0x69)

#define B_PS_LED_NONE			      (0x00)
#define B_PS_LED_5_6MA			    (0x01)
#define B_PS_LED_11_2MA			    (0x02)
#define B_PS_LED_22_4MA			    (0x03)
#define B_PS_LED_44_8MA			    (0x04)

#define B_IRQ_CMD_FLAG			    (0x20)
#define B_IRQ_PS1_FLAG			    (0x04)
#define B_IRQ_ALS0_FLAG			    (0x01)
#define B_IRQ_ALS1_FLAG			    (0x02)

#define B_CHIP_RUNNING			    (0x04)
#define B_CHIP_SUSPEND			    (0x02)
#define B_CHIP_SLEEP			      (0x01)

void LT1PH03_Init_Sensor(void);
void LT1PH03_Enable_Sensor(void);
void LT1PH03_Init(void);
void LT1PH03_Measure(void);

void LT1PH03_Send_Command            (I8U cmd);
void LT1PH03_Send_Parameter          (I8U param);
void LT1PH03_Send_Command_Parameter  (I8U cmd, I8U param);
void LT1PH03_Send_Command_Query      (I8U offset, I8U param);
void LT1PH03_Send_Command_SET        (I8U offset, I8U param);
void LT1PH03_Send_Command_AND        (I8U offset, I8U param);
void LT1PH03_Send_Command_OR         (I8U offset, I8U param);
void LT1PH03_Modify_Command_I2C_Addr (I8U i2c_addr);

void LT1PH03_Send_Command_NOP        (void);
void LT1PH03_Send_Command_Reset      (void);
void LT1PH03_PS_Force                (void);
void LT1PH03_ALS_Force               (void);
void LT1PH03_PS_ALS_Force            (void);
void LT1PH03_PS_Pause                (void);
void LT1PH03_ALS_Pause               (void);
void LT1PH03_PS_ALS_Pause            (void);
void LT1PH03_PS_Auto                 (void);
void LT1PH03_ALS_Auto                (void);
void LT1PH03_PS_ALS_Auto             (void);

#endif


