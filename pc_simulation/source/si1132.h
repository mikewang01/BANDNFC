

#ifndef SI1132_H
#define SI1132_H

#define SI1132_I2C_ADDR                 		(0xC0)    // 0x60

#define SI1132_REGS_PART_ID                 (0x00)
#define SI1132_REGS_REV_ID                  (0x01)
#define SI1132_REGS_SEQ_ID                  (0x02)
#define SI1132_REGS_INT_CFG                 (0x03)    // 0: INT pin is never driven/1: INT pin driven low whenever an IRQ_STATUS and its corresponding IRQ_ENABLE bits match
#define SI1132_REGS_IRQ_ENABLE              (0x04)    // ALS Interrupt Enable
#define SI1132_REGS_HW_KEY                  (0x07)    // System must write value 0x17 to this register for proper Si1132 operation.
#define SI1132_REGS_MEAS_RATE0              (0x08)    // MEAS_RATE1 and MEAS_RATE0 together form a 16-bit value: MEAS_RATE[15:0]
#define SI1132_REGS_MEAS_RATE1              (0x09)
#define SI1132_REGS_MEAS_UCOEF0             (0x13)
#define SI1132_REGS_MEAS_UCOEF1             (0x14)
#define SI1132_REGS_MEAS_UCOEF2             (0x15)
#define SI1132_REGS_MEAS_UCOEF3             (0x16)
#define SI1132_REGS_PARAM_WR                (0x17)    // Mailbox register for passing parameters from the host to the sequencer.
#define SI1132_REGS_COMMAND                 (0x18)    // Mailbox register for command.
#define SI1132_REGS_RESPONSE                (0x20)
#define SI1132_REGS_IRQ_STATUS              (0x21)    // Command Interrupt Status/ALS Interrupt Status.
#define SI1132_REGS_ALS_VIS_DATA0           (0x22)
#define SI1132_REGS_ALS_VIS_DATA1           (0x23)
#define SI1132_REGS_ALS_IR_DATA0            (0x24)
#define SI1132_REGS_ALS_IR_DATA1            (0x25)
#define SI1132_REGS_AUX_DATA0_UVIDX0        (0x2C)
#define SI1132_REGS_AUX_DATA1_UVIDX1        (0x2D)
#define SI1132_REGS_PARAM_RD                (0x2E)
#define SI1132_REGS_CHIP_STAT               (0x30)
#define SI1132_REGS_ANA_IN_KEY3             (0x3B)    // ANA_IN_KEY[31:24]
#define SI1132_REGS_ANA_IN_KEY2             (0x3C)    // ANA_IN_KEY[23:16]
#define SI1132_REGS_ANA_IN_KEY1             (0x3D)    // ANA_IN_KEY[15: 8]
#define SI1132_REGS_ANA_IN_KEY0             (0x3E)    // ANA_IN_KEY[ 7: 0]

#define SI1132_PARAM_I2C_ADDR               (0x00)
#define SI1132_PARAM_CHLIST                 (0x01)
#define SI1132_PARAM_ALS_ENCODING           (0x06)
#define SI1132_PARAM_ALS_IR_ADCMUX          (0x0E)
#define SI1132_PARAM_AUX_ADCMUX             (0x0F)
#define SI1132_PARAM_ALS_VIS_ADC_COUNTER    (0x10)
#define SI1132_PARAM_ALS_VIS_ADC_GAIN       (0x11)
#define SI1132_PARAM_ALS_VIS_ADC_MISC       (0x12)
#define SI1132_PARAM_ALS_IR_ADC_COUNTER     (0x1D)
#define SI1132_PARAM_ALS_IR_ADC_GAIN        (0x1E)
#define SI1132_PARAM_ALS_IR_ADC_MISC        (0x1F)

#define SI1132_RESPONSE_INV_SETTING         (0x80)
#define SI1132_RESPONSE_VIS_OVF             (0x8C)
#define SI1132_RESPONSE_IR_OVF              (0x8D)
#define SI1132_RESPONSE_AUX_OVF             (0x8E)

void SI1132_Init(void);
void SI1132_Measure(void);

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void SI1132_Send_Command(I8U cmd);
void SI1132_Send_Parameter(I8U param);
void SI1132_Send_Command_Parameter(I8U cmd, I8U param);
void SI1132_Send_Command_Query(I8U offset, I8U param);
void SI1132_Send_Command_SET  (I8U offset, I8U param);     /* 
void SI1132_Send_Command_AND  (I8U offset, I8U param);
void SI1132_Send_Command_OR   (I8U offset, I8U param);     */
void SI1132_Send_Command_NOP(void);
void SI1132_Send_Command_Reset(void);
void SI1132_ALS_Force(void);                               // Forces a single ALS measurement
void SI1132_ALS_Pause(void);                               // Pauses autonomous ALS
void SI1132_ALS_Auto(void);                                /* 
void SI1132_PS_Force(void);
void SI1132_PS_ALS_Force(void);
void SI1132_PS_Pause(void);
void SI1132_PS_ALS_Pause(void);
void SI1132_PS_Auto(void);
void SI1132_PS_ALS_Auto(void);                             */
void SI1132_Get_Cal(void);                                 // Reports calibration data to I2C registers 0x22 to 0x2D
void SI1132_Modify_Command_I2C_Addr(I8U i2c_addr);
void SI1132_Reset(void);
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif            // #ifndef SI1132_H


