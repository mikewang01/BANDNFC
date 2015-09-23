/******************** (C) COPYRIGHT 2012 STMicroelectronics ********************
* File Name          : ek3dh.c
* Author             : MSH Application Team
* Author             : andrea labombarda
* Version            : $Revision:$
* Date               : $Date:$
* Description        : This file provides a set of functions needed to manage the
*                      3dh adapter board.
* HISTORY:
* Date        | Modification                                | Author
* 19/01/2012  | Initial Revision                            | Andrea Labombarda
* 21/02/2012  | Fifo Delay fixed							| Andrea Labombarda
* 28/02/2012  | code make up								| Andrea Labombarda
* 17/04/2012  |	USB TX flow fixed. No need of delay			| Andrea Labombarda

********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*
* THIS SOFTWARE IS SPECIFICALLY DESIGNED FOR EXCLUSIVE USE WITH ST PARTS.
*
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ek3dh.h"
#include "led.h"
#include "command_interpreter.h"
#include "string.h"
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "spi_mems.h"
#include "ctype.h"
#include "utility.h"
#include "interruptHandler.h"
#include <stdio.h>
#include "hw_config.h"
#include "fw_selector.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define ValBit(VAR,Place)         (VAR & (1<<Place))
/* Private variables ---------------------------------------------------------*/
uint8_t usbBuff[MAX_COMMAND_LENGTH];
static uint8_t state = STOP;
static uint8_t stateOld = STOP;

static uint8_t interruptOne = 0;
static uint8_t interruptTwo = 0;

static uint8_t zoffFlag = 0;
static uint8_t echoOnFlag = 0;

extern uint8_t dbSelected;

// *** FIFO VARIABLES *** //
#define FIFO_DEPTH 32

static char FIFO_Src = 0, Old_FIFO_Src = 0;
static char FIFO_Ovr = 0;
static char FIFO_Trig = 0;
static char FIFO_interrupt = 0;
static char FIFO_count = 0;
static char FIFO_case = STOP;
static unsigned char state_machine = 0;
static unsigned char Power_Mode;
// *** END FIFO VARIABLES *** //

/* Private function prototypes -----------------------------------------------*/
void ek3dh_AppTick(void);
static uint8_t Decod();
static void StateMachine(void);
static void Version(void);
static void Device(void);
static void Zoff(void);
static void Zon(void);
static void Start(void);
static void Stop(void);
static void ReadReg(void);
static void WriteReg(void);
static void Debug(void);
static void InterruptReceived(uint8_t interrupt);
static void DbReset(void);

//fifo functions
static void FifoBypass(void);
static void FifoStream(void);
static void FifoMode(void);
static void FifoRun(void);
static void FifoStopTrig(void);
static void FifoRead(void);
static void SendFiFoSrc(void); 
static void SendFiFoDataVector(void);

/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : ek3dh_AppTick
* Description    : lis3dh Application Tick Function
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void ek3dh_AppTick(void) {
  EKSTM32_LEDOn(LED3);
  if(newCommandAvailable(usbBuff)) {
   
    state = Decod();
  }  
  
  StateMachine();
  
}


/*******************************************************************************
* Function Name	: Decod
* Description	: decode the incoming USB data
**				:  decodification of RTX vector
**				:  *start/r:  	      :starts sending data
**				:  *stop/r:   	      :stops sending data
**				:  *ver/r:	  	      :sends firmware version number
**				:  *dev/r:            :sends device name
**				:  *wxxdd/r:          :writes in register at address xx:(0xadr) --> the value data dd:(0xdata) 
**				:  *rxx/r:     	      :reads register at address xx:(0xadr)
**				:  *zon/r:		        :activates tristate mode
**				:  *zoff/r:           :activates connections with devices    
**
**   Realizes the command interpreter (for general purpose commands).
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static uint8_t Decod(){
  
  if(strcmp((char*)&usbBuff[1],"VER")==0) {
    stateOld = state;
    state = VER;
  }
  else if(strcmp((char*)&usbBuff[1],"DEV")==0) {
    stateOld = state;
    state = DEV;    
  }
  else if(strcmp((char*)&usbBuff[1],"ZOFF")==0) {
    stateOld = state;
    state = ZOFF;    
  }
  else if(strcmp((char*)&usbBuff[1],"ZON")==0) {
    state = ZON;    
  }
  else if(strcmp((char*)&usbBuff[1],"START")==0) {
    stateOld = state;
    state = START;    
  }
  else if(strcmp((char*)&usbBuff[1],"STOP")==0) {
    stateOld = state;
    state = STOP;    
  } 
  else if(usbBuff[1] == 'R') {
    stateOld = state;
    state = R;    
  }
  else if(usbBuff[1] == 'W') {
    stateOld = state;
    state = W;    
  }
  else if(strcmp((char*)&usbBuff[1],"DEBUG")==0) {
    state = DEBUG;    
  }  
  else if(strcmp((char*)&usbBuff[1],"FIFOSTR")==0) {
    state = FIFO_BYPASS;
    FIFO_case = FIFO_STREAM;
  }
  else if(strcmp((char*)&usbBuff[1],"FIFOMDE")==0) {
    state = FIFO_BYPASS;
    FIFO_case = FIFO_MODE;
  }
  else if(strcmp((char*)&usbBuff[1],"FIFOTRG")==0) {
    state = FIFO_BYPASS;
    FIFO_case = FIFO_STOP_TRIG;
  }
  else if(strcmp((char*)&usbBuff[1],"FIFORST")==0) {
    state = FIFO_BYPASS;
    FIFO_case = STOP;
  }
  else if(strcmp((char*)&usbBuff[1],"LIST")==0) {
    stateOld = state;    
    state = LIST;    
  }
  else if(strcmp((char*)&usbBuff[1],"SINGLE")==0) {
    stateOld = state;    
    state = SINGLE;    
  }  
  else if(strcmp((char*)&usbBuff[1],"ECHOON")==0) {
    stateOld = state;    
    state = ECHOON;    
  }
  else if(strcmp((char*)&usbBuff[1],"ECHOOFF")==0) {
    stateOld = state;
    state = ECHOOFF;
  }
  else if(strcmp((char*)&usbBuff[1],"LISTDEV")==0) {
    stateOld = state;
    state = LISTDEV;
  } 
  else if(strcmp((char*)&usbBuff[1],"DBRESET")==0) {
    stateOld = state;
    state = DBRESET;    
  }   
    
 return state; 
}


/*******************************************************************************
* Function Name  : StateMachine
* Description    : StateMachiine is called from the aAppTick main loop
*                  It calls functions that implement commands selected
*                  It implements state machine for commands interpreter.
*                  If Set of available commands is extended with device specific commands
* Input          : None 
* Output         : None
* Return         : None
*******************************************************************************/
static void StateMachine(void) {
  
  switch(state) {
  case VER:
    Version();
    state = stateOld;
    break;
  case DEV:
    Device();
    state = stateOld;
    break;
  case ZOFF:
    Zoff();
    state = STOP;
    stateOld = STOP;
    break;
  case ZON:
    Zon();
    state = STOP;
    stateOld = STOP;
    break;
  case START:
    if(zoffFlag) {
      Start();
    }
    break;
  case STOP:
    if(zoffFlag) {
      Stop();
    }
    break;
  case R:
    if(zoffFlag) {
      ReadReg();
      state = stateOld;
    }
    break;
  case W:
    if(zoffFlag) {
      WriteReg();
      state = stateOld;
    }
    break;
  case DEBUG:
    if(zoffFlag) {
      Debug();
    }
    break;
  case FIFO_BYPASS:
    if(zoffFlag) {
      FifoBypass();
    }
    break;
  case FIFO_STREAM:
    if(zoffFlag) {
      FifoStream();
    }
    break;
  case FIFO_MODE:
    if(zoffFlag) {
      FifoMode();
    }
    break;
  case FIFO_RUN:
    if(zoffFlag) {
      FifoRun();
    }
    break;
  case FIFO_STOP_TRIG:
    if(zoffFlag) {
      FifoStopTrig();
    }
    break;
  case FIFO_READ:
    if(zoffFlag) {
      FifoRead();
    }
    break;   
  case LIST:
    //It Prints the list of firmwares implemented
    PrintListOfFirmware();
    state = stateOld;
    break; 
  case SINGLE:
    if(zoffFlag) {
      Debug();
      state = stateOld;
    }
    break; 
  case ECHOON:
      echoOnFlag = 1;
      state = stateOld;
    break;
  case ECHOOFF:
      echoOnFlag = 0;
      state = stateOld;
    break; 
  case LISTDEV:
    //It Prints the list of device supported
    PrintListOfDevice();
    state = stateOld;
    break;
  case DBRESET:
    //unselect the db   
    DbReset();
    state = STOP;
    break;
  }
  
}


/*******************************************************************************
* Function Name  : Version
* Description    : Function to send Firmware version to host by USB    
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Version(void) {
 //eMotion firmware version
  EmotionVer();   

}

/*******************************************************************************
* Function Name  : Device
* Description    : Function to send the Device Product Code
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Device(void) {
  uint8_t buffer[]="LIS3DH    \r\n";
    
  MEMS_Print(buffer, sizeof(buffer));
  
}

/*******************************************************************************
* Function Name  : Zoff
* Description    : It initializes the peripherals according to the device
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Zoff(void) {
  EKSTM32_LEDOn(LED2);  
  RegisterInterrupt(InterruptReceived);
  SPI_Mems_Init();
  zoffFlag = 1;
}

/*******************************************************************************
* Function Name  : Zon
* Description    : It DeInitializes the peripherals according to the device
*                  Now the device is disconnected from the board
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Zon(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  
  GPIO_InitStructure.GPIO_Pin = SPI_MEMS_PIN_SCK | SPI_MEMS_PIN_MISO | SPI_MEMS_PIN_MOSI;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SPI_MEMS_GPIO, &GPIO_InitStructure);  
  
  GPIO_InitStructure.GPIO_Pin = SPI_MEMS_CS;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(SPI_MEMS_CS_GPIO, &GPIO_InitStructure);
  
  UnRegisterInterrupt();
  
  EKSTM32_LEDOff(LED2);  
  
  zoffFlag = 0;
    
}

/*******************************************************************************
* Function Name  : Start
* Description    : Function to Send the standard output data to USB (associated to '*start' command).
*                  The system send continuosly through the USB interface
*                  a char vector composed by acceleration, interrupt and
*                  switch status.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Start(void) {
  
  uint8_t dataReady = 0;
  uint8_t buffer[13];
  
  dataReady = SPI_Mems_Read_Reg(STATUS_REG);
  if(ValBit(dataReady, DATAREADY_BIT)) {
    EKSTM32_LEDToggle(LED1);
	
    buffer[0]   = 's';
    buffer[1]   = 't';
    
    buffer[3]   = SPI_Mems_Read_Reg( OUT_X_L );	//reads X LSB
	buffer[2]   = SPI_Mems_Read_Reg( OUT_X_H );	//reads X MSB	
    
    buffer[5]   = SPI_Mems_Read_Reg( OUT_Y_L );	//reads Y LSB
	buffer[4]   = SPI_Mems_Read_Reg( OUT_Y_H );	//reads Y MSB	
    
    buffer[7]   = SPI_Mems_Read_Reg( OUT_Z_L );	//reads Z LSB                
	buffer[6]   = SPI_Mems_Read_Reg( OUT_Z_H );	//reads Z MSB
    
    //interrupt 1
    if(interruptOne == 1) {
      //interruptOne = 0;
      buffer[8]   = 0x40;
    }
    else {
      buffer[8]   = 0x00;
    }
    
    //interrupt 2
    if(interruptTwo == 1) {
      //interruptTwo = 0;
      buffer[9]   = 0x40;
    }
    else {
      buffer[9]   = 0x00;
    } 
    
    buffer[10] = 0;
    
    if(SW1_BUTTON_PIN)
      buffer[10] |= 0x01;
    
    if(SW2_BUTTON_PIN)
      buffer[10] |= 0x02;
    
    buffer[11]  = '\r';
    buffer[12]  = '\n';
    
	MEMS_Print(buffer, sizeof(buffer));
	
  }  
    
}

/*******************************************************************************
* Function Name  : Stop
* Description    : It stops the state machine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Stop(void){
  state = STOP;
}

/*******************************************************************************
* Function Name  : WriteReg
* Description    : write a value in a specific sensor register
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void WriteReg() {
  uint8_t data = 0;
  uint8_t address = 0;
  
  if( isxdigit(usbBuff[2]) && isxdigit(usbBuff[3]) && isxdigit(usbBuff[4]) && isxdigit(usbBuff[5]) ) {
    address = StringToChar(&usbBuff[2]);
    data = StringToChar(&usbBuff[4]);
    SPI_Mems_Write_Reg(address, data);
  }
  
  if(echoOnFlag) {    
      uint8_t buffer[9];
      uint8_t response[2];

      address = StringToChar(&usbBuff[2]);

      data = SPI_Mems_Read_Reg(address);
      CharToString(data, response);

      buffer[0]  = 'R';
      buffer[1]  = usbBuff[2];
      buffer[2]  = usbBuff[3];
      buffer[3]  = 'h';
      buffer[4]  = response[0];
      buffer[5]  = response[1];
      buffer[6]  = 'h';
      buffer[7]  = '\r';
      buffer[8]  = '\n';

	  MEMS_Print(buffer, sizeof(buffer));
	  
  } 

}

/*******************************************************************************
* Function Name  : ReadReg
* Description    : read a value in a specific sensor register
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void ReadReg(void) {
  uint8_t data = 0;
  uint8_t address = 0;
  uint8_t response[2];
  uint8_t buffer[9];
  
  address = StringToChar(&usbBuff[2]);  
  data = SPI_Mems_Read_Reg(address);
  CharToString(data, response);
  
  buffer[0]  = 'R';
  buffer[1]  = usbBuff[2];
  buffer[2]  = usbBuff[3];
  buffer[3]  = 'h';
  buffer[4]  = response[0];
  buffer[5]  = response[1];
  buffer[6]  = 'h';
  buffer[7]  = '\r';
  buffer[8] = '\n';
  
  MEMS_Print(buffer, sizeof(buffer));
    
}

/*******************************************************************************
* Function Name  : Debug
* Description    : Function to Send the debug output data to USB (associated to '*debug' command).
*                  The system send continuosly through the USB interface
*                  a readable string containg acquired data
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Debug(void) {  
  uint8_t buffer[27];  
  uint8_t dataReady = 0;
  uint8_t datal, datah;
  int16_t data;
  
  dataReady = SPI_Mems_Read_Reg(STATUS_REG);
  
  if(ValBit(dataReady, DATAREADY_BIT)) {
    //blink the led  
    EKSTM32_LEDToggle(LED1);
    
	datal = SPI_Mems_Read_Reg(OUT_X_L);
	datah = SPI_Mems_Read_Reg(OUT_X_H);
	data = (int16_t) ( (datah<<8) | datal );    
    data>>=4;
    sprintf((char*)buffer,"X=%+05d ",data);

	datal = SPI_Mems_Read_Reg(OUT_Y_L);
	datah = SPI_Mems_Read_Reg(OUT_Y_H);
	data = (int16_t) ( (datah<<8) | datal );        
    data>>=4;
    sprintf((char*)&buffer[8],"Y=%+05d ",data);
	
	datal = SPI_Mems_Read_Reg(OUT_Z_L);
	datah = SPI_Mems_Read_Reg(OUT_Z_H);
	data = (int16_t) ( (datah<<8) | datal );     
    data>>=4;
    sprintf((char*)&buffer[16],"Z=%+05d\r\n",data);
    
	MEMS_Print(buffer, strlen((char*)buffer));
	
  }

}

/*******************************************************************************
* Function Name  : InterruptReceived
* Description    : Interrupt One and Two Handler
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void InterruptReceived(uint8_t interrupt) {
  uint8_t data;
  
  if(interrupt == INTERRUPTONE) { //int1
    //read pin state
    data = GPIO_ReadInputDataBit(INT1_PORT, INT1_PIN);
    if(data == 0x01)
    interruptOne = 1;
    else
      interruptOne = 0;
  }
  
  else if(interrupt == INTERRUPTTWO) { //int2
    //read pin state
    data = GPIO_ReadInputDataBit(INT2_PORT, INT2_PIN);
    if(data == 0x01)
    interruptTwo = 1;
    else
      interruptTwo = 0;        
  }  
  
}


/*******************************************************************************
* Function Name  : FifoBypass
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void FifoBypass(void) {
  
  //Read Trigger bit 
  FIFO_Trig = (SPI_Mems_Read_Reg(FIFO_CONTROL) & FIFO_CONTROL_TRIG);
  
  // Clear Interrupt if LIR enabled
  if (!FIFO_Trig) 
     while(SPI_Mems_Read_Reg(INT1_SRC) & INT1_SRC_IA );
  else 
    while(SPI_Mems_Read_Reg(INT2_SRC) & INT2_SRC_IA );
    
  // Reset FIFO_STOP_Trigger State Machine
  state_machine = 0;
  
  // BYPASS Mode
  SPI_Mems_Write_Reg(FIFO_CONTROL, ((SPI_Mems_Read_Reg(FIFO_CONTROL) & FIFO_CONTROL_WTMSAMP)|DEF_FIFO_CTRL_BYPASS));
  
  // Disable FIFO
  SPI_Mems_Write_Reg(CTRL_REG5, (SPI_Mems_Read_Reg(CTRL_REG5)&(~CTRL_REG5_FIFO_EN)));		
  
  // Send FIFO Source
  SendFiFoSrc();
    
  // Return to previous FIFO CASE
  state = FIFO_case;
}


/*******************************************************************************
* Function Name  : FifoStream
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void FifoStream(void) {
  
  SPI_Mems_Write_Reg(CTRL_REG5, (SPI_Mems_Read_Reg(CTRL_REG5)|CTRL_REG5_FIFO_EN));
  SPI_Mems_Write_Reg(FIFO_CONTROL, ((SPI_Mems_Read_Reg(FIFO_CONTROL) & FIFO_CONTROL_WTMSAMP)|DEF_FIFO_CTRL_STREAM));
  
  // Send FIFO Source
  SendFiFoSrc();
    
  // In FIFO MODE, case FIFO STREAM is executed only one time!!
  state = FIFO_RUN;
				
}

/*******************************************************************************
* Function Name  : FifoMode
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void FifoMode(void) {
  SPI_Mems_Write_Reg(CTRL_REG5, (SPI_Mems_Read_Reg(CTRL_REG5)|CTRL_REG5_FIFO_EN));
  SPI_Mems_Write_Reg(FIFO_CONTROL, ((SPI_Mems_Read_Reg(FIFO_CONTROL) & FIFO_CONTROL_WTMSAMP)|DEF_FIFO_CTRL_FIFO_MODE));
  
  // Send FIFO Source
  SendFiFoSrc();
    
  state = FIFO_RUN;
  
}


/*******************************************************************************
* Function Name  : FifoRun
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void FifoRun(void) {
 
  FIFO_Src = SPI_Mems_Read_Reg(FIFO_SOURCE);
  FIFO_Ovr = FIFO_Src & FIFO_SOURCE_OVRN;
  
  // If new sample in FIFO
  if (FIFO_Src != Old_FIFO_Src) {
    // Send FIFO Source 
    SendFiFoSrc();
	Old_FIFO_Src = FIFO_Src;          			
  }        
  
  // if FIFO is full, read FIFO content
  if (FIFO_Ovr) {
    // Set next FIFO case
    FIFO_case = FIFO_RUN;
    state = FIFO_READ;
    
    // reset samples counter
    FIFO_count = 0;
  }

}


/*******************************************************************************
* Function Name  : FifoStopTrig
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void FifoStopTrig(void) {
  
  switch (state_machine) {
    case 0:
      // Clear Interrupt if LIR enabled
      SPI_Mems_Read_Reg(INT1_SRC);
      SPI_Mems_Read_Reg(INT2_SRC);
      
      // Interrupt spend 1/2 samples to go low!!!
      SPI_Mems_Write_Reg(CTRL_REG5, (SPI_Mems_Read_Reg(CTRL_REG5)|CTRL_REG5_FIFO_EN));
      SPI_Mems_Write_Reg(FIFO_CONTROL, ((SPI_Mems_Read_Reg(FIFO_CONTROL) & FIFO_CONTROL_WTMSAMP)|DEF_FIFO_CTRL_STOPTRIG));
      
      // Send FIFO Source
      SendFiFoSrc();
	       
      //Read Trigger bit 
      FIFO_Trig = (SPI_Mems_Read_Reg(FIFO_CONTROL) & FIFO_CONTROL_TRIG);
      
      state_machine = 1;
      break;
  
  case 1:
    FIFO_Src = SPI_Mems_Read_Reg(FIFO_SOURCE);
    FIFO_Ovr = FIFO_Src & FIFO_SOURCE_OVRN;
    
    // If FIFO Overrun and Interrupt
    if (FIFO_Ovr) {      
      // Interrupt selection
      if (!FIFO_Trig) 
        FIFO_interrupt = SPI_Mems_Read_Reg(INT1_SRC) & INT1_SRC_IA;
      else 
        FIFO_interrupt = SPI_Mems_Read_Reg(INT2_SRC) & INT2_SRC_IA;
      
      if (FIFO_interrupt) {
        state_machine = 2;           
      }          
    }
    
    FIFO_Src = SPI_Mems_Read_Reg(FIFO_SOURCE);
    // If new sample in FIFO
    if (FIFO_Src != Old_FIFO_Src) {
      // Send FIFO Source 
      SendFiFoSrc();
	  Old_FIFO_Src = FIFO_Src;          
                     
    }      
    
  break;
  
  case 2:
    // FIFO needs 1 ODR after TRIGGER
    // ODR 5KHz is selected in order to have fast ODR generation.
    // After 1 ODR FIFO is stopped. 
    Power_Mode = SPI_Mems_Read_Reg(CTRL_REG1);
    SPI_Mems_Write_Reg(CTRL_REG1, 0x97);	// 5KHz
    wait(1000);
    SPI_Mems_Write_Reg(CTRL_REG1, Power_Mode); // Restore previous ODR
    
    // Set next FIFO case
    FIFO_case = FIFO_STOP_TRIG;
    state = FIFO_READ;
    
    // reset samples counter
    FIFO_count = 0;
    state_machine = 3;
  break;
  
  case 3:
    state = STOP;  
  break;
  
  default: break;
  
  } 

}

/*******************************************************************************
* Function Name  : FifoRead
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void FifoRead(void) {
  //FIFO READ : The system send 32 accelerations samples 
  //through the USB interface
  //(FIFO content)
  
  if (FIFO_count < FIFO_DEPTH) {
    SendFiFoDataVector();
	FIFO_count++;      
  }
  else 
    state = FIFO_case; // Return to previous FIFO CASE  
  
  
  wait(1000);
}

/*******************************************************************************
* Function Name  : SendFiFoSrc
* Description    : Function to Send the standard output data to USB (associated to '*FIFO' command).
*                  The system send continuosly through the USB interface
*                  a char vector composed by, interrupt,
*                  FIFO_CONTROL and FIFO_SOURCE
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void SendFiFoSrc(void) {
  uint8_t FIFO_Vector[14];
  
  FIFO_Vector[0] = 's';
  FIFO_Vector[1] = 't';
  
  FIFO_Vector[2] = 0;
  FIFO_Vector[3] = 0;
  FIFO_Vector[4] = 0;  
  FIFO_Vector[5] = 0;
  FIFO_Vector[6] = 0;
  FIFO_Vector[7] = 0;
  
  if(interruptOne == 1) {
      //interruptOne = 0;
      FIFO_Vector[8]   = 0x40;
  }
  else {
      FIFO_Vector[8]   = 0x00;
  }  
  
  if(interruptTwo == 1) {
      //interruptTwo = 0;
      FIFO_Vector[9]   = 0x40;
  }
  else {
      FIFO_Vector[9]   = 0x00;
  } 

	
  FIFO_Vector[10] = SPI_Mems_Read_Reg(FIFO_CONTROL);
  FIFO_Vector[11] = SPI_Mems_Read_Reg(FIFO_SOURCE);
  
  FIFO_Vector[12] = '\r';
  FIFO_Vector[13] = '\n';
      
  MEMS_Print(FIFO_Vector, sizeof(FIFO_Vector));
 
}



/*******************************************************************************
* Function Name  : SendFiFoDataVector
* Description    : Function to Send the standard output data to USB (associated to '*FIFO' command).
*                  The system send continuosly through the USB interface
*                  a char vector composed by, FIFO Data, interrupt,
*                  FIFO_CONTROL and FIFO_SOURCE
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void SendFiFoDataVector(void) {
  uint8_t FIFO_Data_Vector[14];  
  
  FIFO_Data_Vector[0]	=	's';
  FIFO_Data_Vector[1]	=	't';
    
  FIFO_Data_Vector[3]	=	SPI_Mems_Read_Reg( OUT_X_L );	//reads X LSB
  FIFO_Data_Vector[2]	=	SPI_Mems_Read_Reg( OUT_X_H );	//reads X MSB
  
  FIFO_Data_Vector[5]	=	SPI_Mems_Read_Reg( OUT_Y_L );	//reads Y LSB
  FIFO_Data_Vector[4]	=	SPI_Mems_Read_Reg( OUT_Y_H );	//reads Y MSB  
  
  FIFO_Data_Vector[7]	=	SPI_Mems_Read_Reg( OUT_Z_L );	//reads Z LSB
  FIFO_Data_Vector[6]	=	SPI_Mems_Read_Reg( OUT_Z_H );	//reads Z MSB
  
  if(interruptOne == 1) {
      //interruptOne = 0;
      FIFO_Data_Vector[8]   = 0x40;
  }
  else {
      FIFO_Data_Vector[8]   = 0x00;
  }  
  
  if(interruptTwo == 1) {
      //interruptTwo = 0;
      FIFO_Data_Vector[9]   = 0x40;
  }
  else {
      FIFO_Data_Vector[9]   = 0x00;
  } 

  
  FIFO_Data_Vector[10] = SPI_Mems_Read_Reg(FIFO_CONTROL);
  FIFO_Data_Vector[11] = SPI_Mems_Read_Reg(FIFO_SOURCE);
  
  FIFO_Data_Vector[12]	=	'\r';
  FIFO_Data_Vector[13]	=	'\n';
  
  MEMS_Print(FIFO_Data_Vector, sizeof(FIFO_Data_Vector));

}


/*******************************************************************************
* Function Name  : DbReset
* Description    : It reset the firmware selectet. 
*                : A new firmware can be now selected
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DbReset(void) {  
  dbSelected = 0;
  Zon();
  EKSTM32_LEDOff(LED1);
  EKSTM32_LEDOff(LED2);
  EKSTM32_LEDOff(LED3);   
}

/******************* (C) COPYRIGHT 2012 STMicroelectronics *****END OF FILE****/