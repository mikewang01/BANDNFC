
#include "main.h"
#include "nrf_gpiote.h"
#include "spidev_hal.h"
#include "FM1280.h"
#include "ble_pay_app.h"
#include "app_pwm.h"
#include "nrf_drv_timer.h"
#include "iso7816_2.h"
#include "nrf_gpio.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_gpiote.h"
#include "nrf_delay.h"
#include <string.h>
#include "main.h"


#define _ISO7816_LOG N_SPRINTF
#define  FM1280_EEPROM  ISO7816_EEPROM
#define  TIMER_USED_AS_PWM			2
#define  CAPTURE_CMP_CHANNEL_NO  0

typedef enum {
    FM1280B_POWERING_DOWN,
    FM1280B_POWER_DOWN,
    FM1280B_POWERING_UP,
    FM1280B_MONITOR,
    FM1280B_ACTIVE
} fm1280_pwr_state_t;

#if 0
static int __fm1280_enter_power_down_mode()
{
		return 1;
}


static   fm1280_pwr_state_t state =  FM1280B_POWER_DOWN;
#define  UPDATE_FM1280_POWER_STATE(X)  do{state = X;}while(0)
#define  GET_FM1280_POWER_STATE()      state
#endif


#include "iso7816_2.h"
static volatile bool m_sys_rdy = false;
static uint8_t sw1 = 0;
static uint8_t sw2 = 0;
typedef enum {
    FM1280_DEACTIVE,
    FM1280_IDLE,
    FM1280_ACTIVED,
    FM1280_SEND,
    FM1280_ACTIVING,
    FM1280_ACTIVE_END,
    FM1280_RESPONSE,
} fm1280_state_t;

static fm1280_state_t current_state = FM1280_IDLE;

static iso7816_tpdu_t m_tpdu;
static bool data_sended = true;

#define RESPONSE_SW_RETIEVE_DATA 		    0x61
#define RESPONSE_SW_WRONG_LEGNTH        0X6C
#define RESPONSE_SW_CMD_NOT_SUPPORTED   0X6D
#define RESPONSE_SW_CMD_NULL						0x60
#define RESPONSE_SW_SUCESS_MASK1		    0X60
#define RESPONSE_SW_SUCESS_MASK2		    0X90


extern I8U g_spi_tx_buf[];
extern I8U g_spi_rx_buf[];
static uint32_t iso7816_handler (iso7816_evt_t * p_event);

struct pwm_cfg {
    nrf_ppi_channel_t  p_channel;
    nrf_ppi_channel_group_t p_group;
    nrf_drv_timer_t pwm_timer;
    uint32_t     			clk_rate;
    uint8_t 					timer_id;
    uint8_t						cc_channel;
    uint16_t      		gpio_num;

};


struct fm1280_cfg {
    struct {
        uint8_t  irq;
        uint8_t  monitor;
        uint8_t  power_on;
        uint8_t  low_bat;
        uint8_t  clk;
        uint8_t  io_data;
        uint8_t  rst;
    } pin_cfg;

};
#if 0
static const struct fm1280_cfg  fm1280b_cfg_local = {
    .pin_cfg.rst = GPIO_7816_RST,
    .pin_cfg.low_bat = GPIO_FM1280B_LBAT,
    .pin_cfg.clk = GPIO_7816_CLK,
    .pin_cfg.irq = GPIO_FM1280B_IRQ,
    .pin_cfg.monitor = GPIO_FM1280B_MON,
    .pin_cfg.io_data = GPIO_7816_DATA,
    .pin_cfg.power_on = GPIO_7816_PWR_ON
};
#endif


static  const iso7816_init_t iso7816_init_param = {
	  .clock_freq       = ISO7816_FREQ_4MHZ,
		.vcc_pin          = GPIO_7816_PWR_ON,  // 21;
    .clk_pin          = GPIO_7816_CLK,
    .rst_pin          = GPIO_7816_RST,
    .io_pin           = GPIO_7816_DATA,
	  .dbg_activity_pin = GPIO_RESERVE_5,  // ISO7816_INVALID_PIN_NUMBER;
    .dbg_ppi_pin      = GPIO_RESERVE_5,   //ISO7816_INVALID_PIN_NUMBER;
    .evt_cb           = iso7816_handler
};

#if 0
static void fm1280_tx_rx(spi_master_hw_instance_t   spi_master_instance,
                         I8U * tx_data_buf, I16U tx_data_size,
                         I8U * rx_data_buf, I16U rx_data_size, I8U pin_cs)
{
    (void)spi_master_instance;
    (void)pin_cs;
    CLASS(SpiDevHal)* p =  SpiDevHal_get_instance();
    spi_dev_handle_t t = p->fm1280_open(p);
    p->write_read(p, t, tx_data_buf, tx_data_size, rx_data_buf, rx_data_size);
    p->close(p, t);

}
#endif




//#define _CONFIG_BOOST_
#if 0
#ifdef _CONFIG_BOOST_
static void  _fm1280b_write_reg(I8U region, I8U reg_idx, I8U *pConfig)
{
    if ( (region==ISO7816_REGION0) || (region==ISO7816_REGION1) ) {
        if (region == ISO7816_REGION0)                          // write region zero registers.
            g_spi_tx_buf[0] = (reg_idx | 0x00);
        else if (region == ISO7816_REGION1)                     // write region one registers.
            g_spi_tx_buf[0] = (reg_idx | 0x80);
        g_spi_tx_buf[1] = *pConfig;
        fm1280_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 2, g_spi_rx_buf, 0, GPIO_SPI_0_CS_FM1280B);
    } else if (region == ISO7816_EEPROM) {                    // write EEPROM.
        g_spi_tx_buf[0] = 0x40;
        g_spi_tx_buf[1] = (reg_idx<<3);
        g_spi_tx_buf[2] = *(pConfig + 0);
        g_spi_tx_buf[3] = *(pConfig + 1);
        g_spi_tx_buf[4] = *(pConfig + 2);
        g_spi_tx_buf[5] = *(pConfig + 3);
        fm1280_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 6, g_spi_rx_buf, 0, GPIO_SPI_0_CS_FM1280B);
    } else if (region == ISO7816_COMMAND) {                   // write command.
        g_spi_tx_buf[0] = (*pConfig) | 0xE0;
        fm1280_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 1, g_spi_rx_buf, 0, GPIO_SPI_0_CS_FM1280B);
    }
}
#endif

static void _fm1280b_read_reg(I8U region, I8U reg_idx)
{
    if ( (region==ISO7816_REGION0) || (region==ISO7816_REGION1) ) {
        if (region == ISO7816_REGION0)                          // read region zero registers.
            g_spi_tx_buf[0] = (reg_idx | 0x20);
        else if (region == ISO7816_REGION1)                     // read region one registers.
            g_spi_tx_buf[0] = (reg_idx | 0xA0);
        g_spi_tx_buf[1] = 0;
        fm1280_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 2, g_spi_rx_buf, 2, GPIO_SPI_0_CS_FM1280B);
    } else if (region == ISO7816_EEPROM) {                    // read EEPROM.
        g_spi_tx_buf[0] = 0x7F;
        g_spi_tx_buf[1] = (reg_idx<<3);
        g_spi_tx_buf[2] = 0;
        fm1280_tx_rx(SPI_MASTER_0, g_spi_tx_buf, 2, g_spi_rx_buf,  6, GPIO_SPI_0_CS_FM1280B);
    }
}

static void gpiote_event_handler (nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if ((pin == fm1280b_cfg_local.pin_cfg.irq) && (action == NRF_GPIOTE_POLARITY_HITOLO)) {
        // io_fsm (ISO7816_IO_FSM_EVT_RX_START_BIT, 0, 0);
    }
}
#endif


#if 0
static void _fm1280b_init()
{
//  I8U buf[4];
//  I8U b_pin = 0xFF;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef _CONFIG_BOOST_
    //
    // Write EEPROM.
    //
//buf[3] = 0x82;  buf[2] = 0x80;  buf[1] = 0x00;	buf[0] = 0x34;    _fm1280b_write_reg(FM1280_EEPROM, 0x00, buf);
//buf[3] = 0xE0;  buf[2] = 0x1A;  buf[1] = 0x04;	buf[0] = 0x00;    _fm1280b_write_reg(FM1280_EEPROM, 0x01, buf);
//buf[3] = 0x80;  buf[2] = 0xC0;  buf[1] = 0x2C;	buf[0] = 0x2F;    _fm1280b_write_reg(FM1280_EEPROM, 0x02, buf);
//buf[3] = 0x00;  buf[2] = 0xFE;  buf[1] = 0x6C;	buf[0] = 0xE0;    _fm1280b_write_reg(FM1280_EEPROM, 0x03, buf);
    buf[3] = 0x00;
    buf[2] = 0x00;
    buf[1] = 0x10;
    buf[0] = 0x00;
    _fm1280b_write_reg(FM1280_EEPROM, 0x0E, buf);
#endif

#if (1)
    //
    // Read EEPROM.
    //
    _fm1280b_read_reg(FM1280_EEPROM, 0x00);
    _ISO7816_LOG("Register 0x00 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x01);
    _ISO7816_LOG("Register 0x01 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x02);
    _ISO7816_LOG("Register 0x02 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x03);
    _ISO7816_LOG("Register 0x03 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x04);
    _ISO7816_LOG("Register 0x04 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x08);
    _ISO7816_LOG("Register 0x08 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x0E);
    _ISO7816_LOG("Register 0x0E Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x0F);
    _ISO7816_LOG("Register 0x0F Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x10);
    _ISO7816_LOG("Register 0x10 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
    _fm1280b_read_reg(FM1280_EEPROM, 0x11);
    _ISO7816_LOG("Register 0x11 Value: 0x%02x 0x%02x 0x%02x 0x%02x", g_spi_rx_buf[5], g_spi_rx_buf[4], g_spi_rx_buf[3], g_spi_rx_buf[2]);
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if (0)
    //
    // Write Register.
    //
    buf[0] = 0xB4;
    _fm1280b_write_reg(FM1280_REGION0, 0x00, buf);
    buf[0] = 0x00;
    _fm1280b_write_reg(FM1280_REGION0, 0x01, buf);
    buf[0] = 0x80;
    _fm1280b_write_reg(FM1280_REGION0, 0x02, buf);
    buf[0] = 0x82;
    _fm1280b_write_reg(FM1280_REGION0, 0x03, buf);
    buf[0] = 0x00;
    _fm1280b_write_reg(FM1280_REGION0, 0x04, buf);
    buf[0] = 0x04;
    _fm1280b_write_reg(FM1280_REGION0, 0x05, buf);
    buf[0] = 0x1A;
    _fm1280b_write_reg(FM1280_REGION0, 0x06, buf);
    buf[0] = 0xE0;
    _fm1280b_write_reg(FM1280_REGION0, 0x07, buf);
    buf[0] = 0x2F;
    _fm1280b_write_reg(FM1280_REGION0, 0x08, buf);
    buf[0] = 0x2C;
    _fm1280b_write_reg(FM1280_REGION0, 0x09, buf);
    buf[0] = 0xC0;
    _fm1280b_write_reg(FM1280_REGION0, 0x0A, buf);
    buf[0] = 0x80;
    _fm1280b_write_reg(FM1280_REGION0, 0x0B, buf);
    buf[0] = 0xE0;
    _fm1280b_write_reg(FM1280_REGION0, 0x0C, buf);
    buf[0] = 0x6C;
    _fm1280b_write_reg(FM1280_REGION0, 0x0D, buf);
    buf[0] = 0xFE;
    _fm1280b_write_reg(FM1280_REGION0, 0x0E, buf);

    buf[0] = 0x10;
    _fm1280b_write_reg(FM1280_REGION1, 0x19, buf);
    buf[0] = 0x00;
    _fm1280b_write_reg(FM1280_REGION1, 0x1A, buf);
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if (0)
    //
    // Read Register
    //
    _fm1280b_read_reg(FM1280_REGION0, 0x00);
    _ISO7816_LOG("Register 0x00: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x01);
    _ISO7816_LOG("Register 0x01: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x02);
    _ISO7816_LOG("Register 0x02: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x03);
    _ISO7816_LOG("Register 0x03: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x04);
    _ISO7816_LOG("Register 0x04: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x05);
    _ISO7816_LOG("Register 0x05: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x06);
    _ISO7816_LOG("Register 0x06: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x07);
    _ISO7816_LOG("Register 0x07: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x08);
    _ISO7816_LOG("Register 0x08: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x09);
    _ISO7816_LOG("Register 0x09: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x0A);
    _ISO7816_LOG("Register 0x0A: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x0B);
    _ISO7816_LOG("Register 0x0B: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x0C);
    _ISO7816_LOG("Register 0x0C: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x0D);
    _ISO7816_LOG("Register 0x0D: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION0, 0x0E);
    _ISO7816_LOG("Register 0x0E: 0x%02x", g_spi_rx_buf[1]);
//_fm1280b_read_reg(FM1280_REGION0, 0x10);    _ISO7816_LOG("Register 0x10: 0x%02x", g_spi_rx_buf[1]);
//_fm1280b_read_reg(FM1280_REGION0, 0x11);    _ISO7816_LOG("Register 0x11: 0x%02x", g_spi_rx_buf[1]);
//_fm1280b_read_reg(FM1280_REGION0, 0x12);    _ISO7816_LOG("Register 0x12: 0x%02x", g_spi_rx_buf[1]);
//_fm1280b_read_reg(FM1280_REGION0, 0x13);    _ISO7816_LOG("Register 0x13: 0x%02x", g_spi_rx_buf[1]);
//_fm1280b_read_reg(FM1280_REGION0, 0x14);    _ISO7816_LOG("Register 0x14: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION1, 0x19);
    _ISO7816_LOG("Register 0x19: 0x%02x", g_spi_rx_buf[1]);
    _fm1280b_read_reg(FM1280_REGION1, 0x1A);
    _ISO7816_LOG("Register 0x1A: 0x%02x", g_spi_rx_buf[1]);
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#if (0)
    while (1) {
        BASE_delay_msec(1000);

        b_pin = nrf_gpio_pin_read(GPIO_CHARGER_INT);
        _ISO7816_LOG("IRQ Pin                    : 0x%02x", b_pin);
        _fm1280b_read_reg(FM1280_REGION0, 0x10);
        _ISO7816_LOG("Register Interrupt  (0x10) : 0x%02x", g_spi_rx_buf[1]);
        _fm1280b_read_reg(FM1280_REGION0, 0x11);
        _ISO7816_LOG("Register Status     (0x11) : 0x%02x", g_spi_rx_buf[1]);
        _fm1280b_read_reg(FM1280_REGION0, 0x12);
        _ISO7816_LOG("Register Gain_and_Q (0x12) : 0x%02x", g_spi_rx_buf[1]);
        _fm1280b_read_reg(FM1280_REGION0, 0x13);
        _ISO7816_LOG("Register VP_RF      (0x13) : 0x%02x", g_spi_rx_buf[1]);
        _fm1280b_read_reg(FM1280_REGION0, 0x14);
        _ISO7816_LOG("Register ADC        (0x14) : 0x%02x", g_spi_rx_buf[1]);
        _ISO7816_LOG("\r\n");

        Watchdog_Feed();
    }
#endif
}



static int fm1280_power_fsm()
{

    /*if event take place then fm1280 pull 1280 high*/
    if(nrf_gpio_pin_read(fm1280b_cfg_local.pin_cfg.irq) == 1) {
        //if()
    }
    switch(GET_FM1280_POWER_STATE()) {
    case FM1280B_POWERING_DOWN:
        break;
    case FM1280B_POWER_DOWN:
        break;
    case FM1280B_POWERING_UP:
        break;
    case FM1280B_MONITOR:
        break;
    case FM1280B_ACTIVE:
        break;
    default:
        break;
    }
		return 1;
}

int fm1280_power_down()
{
    UPDATE_FM1280_POWER_STATE(FM1280B_POWERING_DOWN);
		return 1;
}

int fm1280_power_up()
{
    UPDATE_FM1280_POWER_STATE(FM1280B_POWERING_UP);
		return 1;
}

#endif



static uint32_t iso7816_handler (iso7816_evt_t * p_event)
{

    N_SPRINTF("[fm1280]fm1280_handler event id= %d", p_event->evt_id);
    switch (p_event->evt_id) {
    case ISO7816_EVT_ID_TPDU_RESPONSE:
        m_sys_rdy = true;
        sw1 = p_event->evt.tpdu_rsp.sw1;
        sw2 = p_event->evt.tpdu_rsp.sw2;
        current_state = FM1280_RESPONSE;
        break;

    case ISO7816_EVT_ID_CLOCK_RESUMED:
        break;

    case ISO7816_EVT_ID_RDY:
        m_sys_rdy = true;
        current_state = FM1280_ACTIVE_END;
        break;

    case ISO7816_EVT_ID_DEACTIVATED:
        m_sys_rdy = false;
        current_state = FM1280_DEACTIVE;
        break;

    default:
        break;
    }
    return NRF_SUCCESS;
}


static uint32_t iso7826_fsm_init()
{

		current_state = FM1280_IDLE;
		return 1;
}


static int wait_process(uint32_t timeout)//ms
{
    int ret = true;
    /*reset fsm to default state*/
    m_sys_rdy = false;
    uint32_t cur_time = CLK_get_system_time();
    while (!m_sys_rdy) {
        if(CLK_get_system_time() > cur_time + timeout) {
            ret = false;
            break;
        }
    }
    return ret;
}
int active_fm1280()
{
		int32_t err_code = NRF_SUCCESS;
		if(current_state == FM1280_IDLE){
			err_code = nrf_drv_ppi_init();
			//APP_ERROR_CHECK(err_code);
			if (!nrf_drv_gpiote_is_init()){
					err_code = nrf_drv_gpiote_init();
					APP_ERROR_CHECK(err_code);	
			}
			iso7816_init ((iso7816_init_t *)&iso7816_init_param);
			current_state  = FM1280_ACTIVING;
			err_code = NRF_ERROR_BUSY;
		}
		return err_code;
}

int deactive_fm1280()
{
		int ret;
		if(current_state != FM1280_IDLE){
				/*put fm1280 into idle mode wich means the chip is in power down mode*/
				ret = iso7816_cleanup();
				nrf_drv_ppi_uninit();
				//nrf_drv_gpiote_uninit();
				current_state = FM1280_IDLE;
		}
    return ret;
}


static int core_fsm(void *data)
{
    static  uint16_t data_length = 0;
    static uint8_t last_state=0xff;
		static uint32_t t_curr = 0; 
    if(current_state != last_state) {
        N_SPRINTF("[fm1280]state change to %d", current_state);
        last_state = current_state;
				t_curr = CLK_get_system_time();
    }else{
				/*when no oepration happened in 30s to fsm 1280 switch to */
				if(CLK_get_system_time() - t_curr > 600000){
					if(current_state != FM1280_IDLE){
							N_SPRINTF("[fm1280] fsm1280 active timeout");
							deactive_fm1280();
							current_state = FM1280_IDLE;
					}
							t_curr = CLK_get_system_time();
				}
		}
		
    uint32_t err_code = NRF_SUCCESS;
    int ret = 0;
    switch(current_state) {
    case  FM1280_IDLE:
				t_curr = CLK_get_system_time();
        break;
    case FM1280_ACTIVING:
        break;
    case FM1280_ACTIVE_END:  {
        static const uint8_t tmp[] = {0x90, 0x00};
        CLASS(cling_pay_app)*p =cling_pay_app_get_instance();
        /*notify app 1280 has been actived sucessfully*/
        p->send_event_from_1280(p, (uint8_t *)tmp, sizeof(tmp));
        current_state = FM1280_ACTIVED;
    }
    break;

    case	FM1280_ACTIVED:
        break;
    case   FM1280_SEND:
        if(data_sended  == false) {
            err_code = iso7816_send_TPDU (&m_tpdu);
            if(err_code == NRF_ERROR_BUSY) {

                N_SPRINTF("NRF_ERROR_BUSY 0");
                iso7816_cleanup ();
                current_state = FM1280_DEACTIVE;
                break;
            } else {
                data_sended = true;
            }
            /*since every transmition cycle starts with ISO7816_TPDU_REQ_PUT, data lenth recorded for echaed
            	cycle should be cleared once a new one get requested*/
            if(m_tpdu.req == ISO7816_TPDU_REQ_PUT) {
                data_length = 0;
            }else{
								data_length = m_tpdu.len;
						}
        } else {
            /*if no data pending, put state machine into active fress state*/
            current_state = FM1280_ACTIVED;
        }
        break;
    case	FM1280_RESPONSE:
        //N_SPRINTF("[fm1280]: sw1=0x%02x,sw2=0x%02x", sw1, sw2);
        switch(sw1) {
        case RESPONSE_SW_WRONG_LEGNTH: {
            m_tpdu.len = sw2;
            m_tpdu.req = ISO7816_TPDU_REQ_GET;
            data_sended = false;
            current_state = FM1280_SEND;
            data_length = sw2;
            /*one more loop is needed here to transmit data*/
            ret = 1;
					  N_SPRINTF("RESPONSE_SW_WRONG_LEGNTH 0");
        }
        break;
        case RESPONSE_SW_CMD_NULL:
            break;
        case RESPONSE_SW_RETIEVE_DATA: {
            //N_SPRINTF("RESPONSE_SW_RETIEVE_DATA");
            iso7816_tpdu_t tpdu;
            tpdu.cla  = 0x00;
            tpdu.ins  = 0xc0;
            tpdu.p1   = 0x00;
            tpdu.p2   = 0x00;
            tpdu.req  = ISO7816_TPDU_REQ_GET;
            tpdu.len  = sw2;
            data_length = sw2;
            tpdu.data = m_tpdu.data;
            memcpy(&m_tpdu, &tpdu, sizeof(tpdu));
            data_sended = false;
            current_state = FM1280_SEND;
            /*one more loop is needed here to transmit data*/
            ret = 1;
						N_SPRINTF("RESPONSE_SW_RETIEVE_DATA 0");
        }
        break;
        default: {
            /*we take it as a successfully recieve*/
            /*	{
            				notify mater we get dat a here
            		}
            	*/
            if((sw1&RESPONSE_SW_SUCESS_MASK1)== RESPONSE_SW_SUCESS_MASK1 || (sw1&RESPONSE_SW_SUCESS_MASK2) == RESPONSE_SW_SUCESS_MASK2) {
                if(sw1 != RESPONSE_SW_SUCESS_MASK2){
										data_length = 0;
								}
							  /*this means  we get data wanted successfuly*/
                m_tpdu.data[data_length++] = sw1;
                m_tpdu.data[data_length++] = sw2;
                N_SPRINTF("[fm1280]: sw1=0x%02x,sw2=0x%02x length = %d", sw1, sw2, data_length);
                CLASS(cling_pay_app)*p =cling_pay_app_get_instance();
                /*send event to 1280 mgr*/
                p->send_event_from_1280(p, m_tpdu.data, data_length);
            }
            data_sended = true; /*clear data stored in buffer*/
            current_state = FM1280_ACTIVED;
            break;
        }
        }
        break;
    case	FM1280_DEACTIVE:{
			  static int retry_times = 0;
        if(retry_times < 3) {
            N_SPRINTF("iso7816_reinit");
            iso7816_init ((iso7816_init_t*)&iso7816_init_param);
            /*if sended sucessfully*/
            if(wait_process(300)== false) {
                retry_times ++;
                iso7816_warm_reset();
                iso7816_cleanup ();
                current_state = FM1280_DEACTIVE;
            } else {
                current_state = FM1280_SEND;
                retry_times = 0;
            }
        }else{
						retry_times = 0;
						current_state = FM1280_IDLE;
				}
			}
        break;
    default :
        break;
    }

    return ret;
}

static struct fm1280_adapter_call_backs callback = {
    .init = NULL,
    .core_fsm = core_fsm,
    .rx_cb = NULL,
};

static int fm1280_send(uint8_t *data, size_t length)
{
#define TPFU_GET_REQ_PACKAGE_SIZE  5
		int ret = 0;
		if(current_state == FM1280_ACTIVED){
			iso7816_tpdu_t tpdu;
			int i = 0;
			tpdu.cla  = data[i++];   // CLA: Proprietary, GSM
			tpdu.ins  = data[i++];   // INS: GET RESPONSE
			tpdu.p1   = data[i++];   // P1 : reserved
			tpdu.p2   = data[i++];  // P2 : reserved
			if(length <= TPFU_GET_REQ_PACKAGE_SIZE){
				tpdu.req  = ISO7816_TPDU_REQ_GET;
			}else{
			 tpdu.req  = ISO7816_TPDU_REQ_PUT;
			}
			//tpdu.req  = ISO7816_TPDU_REQ_PUT;
			tpdu.len  = data[i++];
			tpdu.data = &data[i++];
			memcpy(&m_tpdu, &tpdu, sizeof(tpdu));
			N_SPRINTF("[fm1280]: cla = 0x%02x, ins = 0x%02x REQ = 0X%02X", tpdu.cla, tpdu.ins, tpdu.req);
			current_state = FM1280_SEND;
			data_sended = false;
		}else{
				
				ret = -1;
		}
		return ret;
}







static bool ad_fm1280_prepare_for_sleep(void)
{
			bool ret = true;
			if(current_state != FM1280_IDLE){
					ret = false;
			}
			return ret;
}
static void ad_fm1280_sleep_canceled(void)
{

}

static void ad_fm1280_wake_up_ind(bool arg)
{

}

static const adapter_call_backs_t ad_fm1280_pm_call_backs = {
    .ad_prepare_for_sleep = ad_fm1280_prepare_for_sleep,
    .ad_sleep_canceled = ad_fm1280_sleep_canceled,
    .ad_wake_up_ind = ad_fm1280_wake_up_ind,
    .ad_xtal16m_ready_ind = NULL,
    .ad_sleep_preparation_time = 0
};

void ISO7816_Init()
{
    CLASS(cling_pay_app)*p =cling_pay_app_get_instance();
    p->reg_adaptor(p, (struct ble_pay_adapter_call_backs *)&callback);
    p->reg_iso7816_tx(p, fm1280_send);
		iso7826_fsm_init();
		pm_register_adapter(&ad_fm1280_pm_call_backs);
}


#if 0
/**
 * @brief Function for application main entry.
 */
#define NRF_LOG_INIT
#define  NRF_LOG_PRINTF  N_SPRINTF
int main_1280_test(void)
{
    ret_code_t err_code;
    uint8_t i;
    iso7816_tpdu_t tpdu;

    NRF_LOG_PRINTF("start test\r\n");
    iso7816_init_t iso7816_init_param;

    iso7816_init_param.clock_freq       = ISO7816_FREQ_4MHZ;
    iso7816_init_param.vcc_pin          = GPIO_7816_PWR_ON;  // 21;
    iso7816_init_param.clk_pin          = GPIO_7816_CLK;
    iso7816_init_param.rst_pin          = GPIO_7816_RST;
    iso7816_init_param.io_pin           = GPIO_7816_DATA;
    iso7816_init_param.dbg_activity_pin = GPIO_RESERVE_5;   // ISO7816_INVALID_PIN_NUMBER;
    iso7816_init_param.dbg_ppi_pin      = GPIO_RESERVE_5;   //ISO7816_INVALID_PIN_NUMBER;
    iso7816_init_param.evt_cb           = iso7816_handler;

    //err_code = iso7816_init (&iso7816_init_param);
    //APP_ERROR_CHECK(err_code);

    //while (!m_sys_rdy);
    //NRF_LOG_PRINTF("iso7816 ready\r\n");
    nrf_delay_ms (5);

    uint8_t data[7]= {0xa0, 0x01, 0x00, 0x03, 0x33,0x01, 0x01};
    //uint8_t data[]= {0x10, 0x00, 0x00, 0x00, 0x00,0x00, 0x10,0x00, 0x03};
    //uint8_t data[]= {0x00, 0x00};
    /* send APDU: Get CID */
    tpdu.cla  = 0x00;
    tpdu.ins  = 0xA4;
    tpdu.p1   = 0x04;
    tpdu.p2   = 0x00;
    tpdu.req  = ISO7816_TPDU_REQ_PUT;
    tpdu.len  = 0x00;

    tpdu.data = data;
    sw1  =0;
    sw2 = 0;
    NRF_LOG_PRINTF("send APDU: Get CID\r\n");
    // If radio is active, wait for it to become inactive.
//    while (get_radio_state()) {
//        // Do nothing (just wait for radio to become inactive).
//        (void) sd_app_evt_wait();
//    }
    err_code = iso7816_send_TPDU (&tpdu);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_PRINTF ("start waiting:\n");
    m_sys_rdy = false;
    while (!m_sys_rdy);
    NRF_LOG_PRINTF ("Response from card:\n");
    for (i = 0; i < tpdu.len; i++) {
        NRF_LOG_PRINTF ("%02X ", m_buff[i]);
    }
    NRF_LOG_PRINTF ("[%02X %02X]\n", sw1, sw2);

#if 1
    tpdu.cla  = 0x00;
    tpdu.ins  = 0xA4;
    tpdu.p1   = 0x04;
    tpdu.p2   = 0x00;
    tpdu.req  = ISO7816_TPDU_REQ_GET;
    //tpdu.data = data;
    tpdu.len  = sw2;
    tpdu.data = m_buff;

//    while (get_radio_state()) {
//        // Do nothing (just wait for radio to become inactive).
//        (void) sd_app_evt_wait();
//    }
    err_code = iso7816_send_TPDU (&tpdu);
    APP_ERROR_CHECK(err_code);
    NRF_LOG_PRINTF ("start waiting:\n");
    m_sys_rdy = false;
    while (!m_sys_rdy);
    NRF_LOG_PRINTF ("Response from card:\n");
    for (i = 0; i < tpdu.len; i++) {
        NRF_LOG_PRINTF ("%02X ", m_buff[i]);
    }
    NRF_LOG_PRINTF ("[%02X %02X]\n", sw1, sw2);
#endif
    /*
        for(int t = 0; t< 4; t++) {
            NRF_LOG_PRINTF("send APDU: Get CID\r\n");
            // If radio is active, wait for it to become inactive.
            while (get_radio_state()) {
                // Do nothing (just wait for radio to become inactive).
                (void) sd_app_evt_wait();
            }
            err_code = iso7816_send_TPDU (&tpdu);
            APP_ERROR_CHECK(err_code);
            NRF_LOG_PRINTF ("start waiting:\n");
            m_sys_rdy = false;
            while (!m_sys_rdy);
            NRF_LOG_PRINTF ("Response from card:\n");
            for (i = 0; i < tpdu.len; i++) {
                NRF_LOG_PRINTF ("%02X ", m_buff[i]);
            }
            NRF_LOG_PRINTF ("[%02X %02X]\n", sw1, sw2);
            sw1  =0;
            sw2 = 0;

        }
    */
#if 0
    while (true) {
        // Do Nothing - GPIO can be toggled without software intervention.
        __WFE();
        __SEV();
        __WFE();
    }
#endif
}

void  fm1280_test()
{
   // iso7816_test_init();
    main_1280_test();
}
#endif

