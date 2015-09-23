
#include "main.h"

#define SKIN_TEMPERATURE_MEASURING_PERIOD_FOREGROUND  2
#define SKIN_TEMPERATURE_MEASURING_PERIOD_BACKGROUND  600

static void _calc_temperature()
{
	THERMISTOR_CTX *t = &cling.therm;
	I32S volt_diff;
	I32S resistance, resistance_t;
	I16S i, temperature;

#define    FIRST_REST    127080                // resistance at 20.0 Celsius Degreee
#define    FIRST_TEMP    200                   // represent 20.0 Celsius Degreee
#define    TAB_LENGTH    299

	const I8U resistance_inc[TAB_LENGTH] = {
    122, 123, 122, 121, 120, 120, 120, 118, 118, 118, 116, 116, 116, 115, 114, 114, 
    113, 112, 112, 111, 110, 110, 110, 109, 108, 108, 107, 106, 106, 106, 104, 105, 
    103, 104, 102, 102, 102, 101, 100, 100,  99,  99,  99,  97,  98,  96,  97,  95, 
     96,  94,  95,  93,  94,  92,  93,  92,  91,  91,  90,  90,  89,  89,  89,  88, 
     87,  87,  87,  86,  86,  85,  85,  84,  84,  84,  83,  83,  82,  82,  81,  81, 
     80,  81,  79,  79,  79,  79,  78,  78,  77,  77,  76,  76,  76,  75,  75,  74, 
     75,  73,  74,  73,  72,  72,  72,  72,  71,  71,  70,  70,  70,  69,  69,  69, 
     68,  68,  67,  68,  66,  67,  66,  66,  66,  65,  65,  64,  64,  64,  64,  63, 
     63,  63,  62,  62,  61,  62,  61,  61,  60,  60,  60,  59,  60,  58,  59,  58, 
     58,  58,  58,  57,  57,  56,  57,  56,  55,  56,  55,  55,  54,  55,  54,  54, 
     53,  54,  53,  52,  53,  52,  52,  52,  51,  51,  51,  51,  51,  50,  50,  49, 
     50,  49,  49,  49,  48,  49,  48,  47,  48,  47,  47,  47,  47,  46,  46,  46, 
     46,  45,  46,  45,  44,  45,  44,  45,  44,  43,  44,  43,  43,  43,  43,  42, 
     42,  43,  41,  42,  42,  41,  41,  41,  40,  41,  40,  40,  40,  40,  39,  39, 
     40,  38,  39,  39,  38,  38,  38,  38,  38,  37,  37,  37,  37,  37,  37,  36, 
     36,  36,  36,  36,  35,  36,  35,  35,  35,  34,  35,  34,  35,  34,  33,  34, 
     34,  33,  33,  33,  33,  33,  33,  32,  32,  33,  32,  31,  32,  32,  31,  31, 
     31,  31,  31,  31,  30,  31,  30,  30,  30,  30,  29,  30,  29,  30,  29,  29, 
     29,  28,  29,  28,  29,  28,  28,  28,  28,  27,  28};

		// calculate resistance of ntc.
	volt_diff  = ( t->power_volts_reading - t->therm_volts_reading ) * 100000;		 
	resistance = 0;

  for (resistance=0; (volt_diff>0) ; resistance++) {
		volt_diff -= t->therm_volts_reading;
	}
	N_SPRINTF( "resistance: %d ", resistance);

	// calculate temperature.
	resistance_t = FIRST_REST;
	for (i=0; i <(TAB_LENGTH); i++) {
		if (resistance>=resistance_t) { 
			break;
		}
		
		resistance_t -= (resistance_inc[i] * 5);
	}

	temperature = 200 + i;
  Y_SPRINTF( "therm: %d  power: %d  temp: %d", t->therm_volts_reading, t->power_volts_reading, temperature);
	
	t->current_temperature = temperature;
}

void THERMISTOR_init(void)
{
	THERMISTOR_CTX *t = &cling.therm;
	
	// init thermistor measue time base
	t->measure_timebase = cling.time.system_clock_in_sec;
	
	t->state = THERMISTOR_STAT_IDLE;
	t->therm_volts_reading = 0;
	t->power_volts_reading = 0;

	t->current_temperature = 310;
}

BOOLEAN _is_user_viewing_skin_temp()
{
	if (UI_is_idle()) {
		return FALSE;
	}
	
	if (cling.ui.frame_index != UI_DISPLAY_VITAL_SKIN_TEMP) {
		return FALSE;
	}
	
	return TRUE;
}

void THERMISTOR_state_machine()
{
	THERMISTOR_CTX *t = &cling.therm;
	
	I32U t_curr, t_diff;
	
	t_curr = CLK_get_system_time();
	
	N_SPRINTF("[THERM] state: %d", t->state);

	switch (t->state) {

		case THERMISTOR_STAT_IDLE:
		{
			// Jump to duty off to check if measuring time is up.
			t->state = THERMISTOR_STAT_DUTY_OFF;
			N_SPRINTF("[THERM] idle");
			break;
		}
	  case THERMISTOR_STAT_DUTY_ON:
		{
			GPIO_therm_power_on();
			t->measure_timebase = cling.time.system_clock_in_sec;
			t->state = THERMISTOR_STAT_MEASURING;
			t->power_on_timebase = t_curr;
			// Configure ADC
			GPIO_therm_adc_config();
			N_SPRINTF("[THERM] duty on");
			break;
		}
		case THERMISTOR_STAT_MEASURING:
		{
			// add 15 milli-second delay to have power to settle down.
			//
			// The actual measuring takes about 7 ms to finish
			if (t_curr > (t->power_on_timebase + THERMISTOR_POWER_SETTLE_TIME_MS)) {
				N_SPRINTF("[THERM] therm measuring at %d ", CLK_get_system_time());
				cling.therm.therm_volts_reading = nrf_adc_convert_single(NRF_ADC_CONFIG_INPUT_6);
				N_SPRINTF("[THERM] power measuring ");
				cling.therm.power_volts_reading = nrf_adc_convert_single(NRF_ADC_CONFIG_INPUT_2);
				//t->state = THERMISTOR_STAT_START_THERM_PIN_MEASURING;
				t->state = THERMISTOR_STAT_DUTY_OFF;
				GPIO_therm_power_off();
				t->b_start_adc = TRUE;
				N_SPRINTF("[THERM] adc measured at(%d): %d,%d", CLK_get_system_time(), cling.therm.therm_volts_reading, cling.therm.power_volts_reading);
  			_calc_temperature();
			}
			N_SPRINTF("[THERM] therm pin turn on adc");
			
			break;
		}
	  case THERMISTOR_STAT_DUTY_OFF:
		{
			// If skin touch is positive, go ahead to measure PPG
			if (!TOUCH_is_skin_touched()) 
				break;
		
	  	t_diff = cling.time.system_clock_in_sec - t->measure_timebase;
			
	  	if (_is_user_viewing_skin_temp()) {
					N_SPRINTF("[THERM] duty off view -> %d, %d", t_diff, cling.user_data.skin_temp_day_interval);
	  		if (t_diff > SKIN_TEMPERATURE_MEASURING_PERIOD_FOREGROUND) {
	  			t->state = THERMISTOR_STAT_DUTY_ON;
					N_SPRINTF("[THERM] duty off -> ON, view screen");
	  		}
	  	} else {
					N_SPRINTF("[THERM] duty off normal-> %d, %d", t_diff, cling.user_data.skin_temp_day_interval);
    		if ( t_diff >  SKIN_TEMPERATURE_MEASURING_PERIOD_BACKGROUND ) {
					t->state = THERMISTOR_STAT_DUTY_ON;
					N_SPRINTF("[THERM] duty off -> ON, normal");
	  		}
	  	}
	  	break;
		}
		default:
			break;
	}
	
}

BOOLEAN THERMISTOR_is_the_state(THERMISTOR_STATES state_to_check)
{
	THERMISTOR_CTX *t = &cling.therm;
	
	if (t->state == state_to_check)
		return TRUE;
	else
		return FALSE;
}

void THERMISTOR_set_state(THERMISTOR_STATES new_state)
{
	cling.therm.state = new_state;
}


BOOLEAN THERMISTOR_switch_to_duty_on_state()
{
	return TRUE;
}


/*
 * resistance-temperature table of Murata FTN18WF104F01RT NTC
 *
 * temperature: 20.000000 resistance: 127080       127080  451
 * temperature: 20.100196 resistance: 126470               452
 * temperature: 20.200570 resistance: 125855               453
 * temperature: 20.300672 resistance: 125245               454
 * temperature: 20.400493 resistance: 124640               455
 * temperature: 20.500025 resistance: 124040               457
 * temperature: 20.600097 resistance: 123440               458
 * temperature: 20.700714 resistance: 122840               459
 * temperature: 20.800191 resistance: 122250               460
 * temperature: 20.900207 resistance: 121660               462
 * temperature: 21.000766 resistance: 121070       121066  463
 * temperature: 21.100157 resistance: 120490               464
 * temperature: 21.200084 resistance: 119910               465
 * temperature: 21.300553 resistance: 119330               466
 * temperature: 21.400696 resistance: 118755               468
 * temperature: 21.500506 resistance: 118185               469
 * temperature: 21.600855 resistance: 117615               470
 * temperature: 21.700861 resistance: 117050               471
 * temperature: 21.800518 resistance: 116490               473
 * temperature: 21.900711 resistance: 115930               474
 * temperature: 22.000545 resistance: 115375       115368  475
 * temperature: 22.100012 resistance: 114825               476
 * temperature: 22.200012 resistance: 114275               477
 * temperature: 22.300553 resistance: 113725               479
 * temperature: 22.400717 resistance: 113180               480
 * temperature: 22.500496 resistance: 112640               481
 * temperature: 22.600812 resistance: 112100               482
 * temperature: 22.700733 resistance: 111565               484
 * temperature: 22.800251 resistance: 111035               485
 * temperature: 22.900302 resistance: 110505               486
 * temperature: 23.000890 resistance: 109975       109970  487
 * temperature: 23.100110 resistance: 109455               488
 * temperature: 23.200820 resistance: 108930               490
 * temperature: 23.300141 resistance: 108415               491
 * temperature: 23.400963 resistance: 107895               492
 * temperature: 23.500376 resistance: 107385               493
 * temperature: 23.600319 resistance: 106875               495
 * temperature: 23.700797 resistance: 106365               496
 * temperature: 23.800823 resistance: 105860               497
 * temperature: 23.900386 resistance: 105360               498
 * temperature: 24.000479 resistance: 104860       104852  499
 * temperature: 24.100099 resistance: 104365               501
 * temperature: 24.200249 resistance: 103870               502
 * temperature: 24.300935 resistance: 103375               503
 * temperature: 24.400111 resistance: 102890               504
 * temperature: 24.500843 resistance: 102400               505
 * temperature: 24.600044 resistance: 101920               507
 * temperature: 24.700811 resistance: 101435               508
 * temperature: 24.800024 resistance: 100960               509
 * temperature: 24.900814 resistance: 100480               510
 * temperature: 25.000028 resistance: 100010       100000  511
 * temperature: 25.100830 resistance:  99535               513
 * temperature: 25.200032 resistance:  99070               514
 * temperature: 25.300832 resistance:  98600               515
 * temperature: 25.400010 resistance:  98140               516
 * temperature: 25.500797 resistance:  97675               518
 * temperature: 25.601030 resistance:  97215               519
 * temperature: 25.700697 resistance:  96760               520
 * temperature: 25.800891 resistance:  96305               521
 * temperature: 25.900506 resistance:  95855               522
 * temperature: 26.000647 resistance:  95405        95398  524
 * temperature: 26.100197 resistance:  94960               525
 * temperature: 26.200270 resistance:  94515               526
 * temperature: 26.300873 resistance:  94070               527
 * temperature: 26.400871 resistance:  93630               528
 * temperature: 26.500252 resistance:  93195               530
 * temperature: 26.600153 resistance:  92760               531
 * temperature: 26.700581 resistance:  92325               532
 * temperature: 26.800377 resistance:  91895               533
 * temperature: 26.900697 resistance:  91465               534
 * temperature: 27.000372 resistance:  91040        91032  535
 * temperature: 27.100569 resistance:  90615               537
 * temperature: 27.200106 resistance:  90195               538
 * temperature: 27.300164 resistance:  89775               539
 * temperature: 27.400748 resistance:  89355               540
 * temperature: 27.500657 resistance:  88940               541
 * temperature: 27.601089 resistance:  88525               543
 * temperature: 27.700830 resistance:  88115               544
 * temperature: 27.801093 resistance:  87705               545
 * temperature: 27.900651 resistance:  87300               546
 * temperature: 28.000727 resistance:  86895        86889  547
 * temperature: 28.100083 resistance:  86495               549
 * temperature: 28.201207 resistance:  86090               550
 * temperature: 28.300348 resistance:  85695               551
 * temperature: 28.400004 resistance:  85300               552
 * temperature: 28.500178 resistance:  84905               553
 * temperature: 28.600876 resistance:  84510               554
 * temperature: 28.700818 resistance:  84120               556
 * temperature: 28.801281 resistance:  83730               557
 * temperature: 28.900972 resistance:  83345               558
 * temperature: 29.001181 resistance:  82960        82956  559
 * temperature: 29.100601 resistance:  82580               560
 * temperature: 29.200535 resistance:  82200               561
 * temperature: 29.300989 resistance:  81820               563
 * temperature: 29.400636 resistance:  81445               564
 * temperature: 29.500798 resistance:  81070               565
 * temperature: 29.600135 resistance:  80700               566
 * temperature: 29.701337 resistance:  80325               567
 * temperature: 29.800351 resistance:  79960               568
 * temperature: 29.901239 resistance:  79590               570
 * temperature: 30.001282 resistance:  79225        79222  571
 * temperature: 30.100461 resistance:  78865               572
 * temperature: 30.200150 resistance:  78505               573
 * temperature: 30.300353 resistance:  78145               574
 * temperature: 30.401075 resistance:  77785               575
 * temperature: 30.500911 resistance:  77430               577
 * temperature: 30.601262 resistance:  77075               578
 * temperature: 30.700709 resistance:  76725               579
 * temperature: 30.800666 resistance:  76375               580
 * temperature: 30.901139 resistance:  76025               581
 * temperature: 31.000685 resistance:  75680        75675  582
 * temperature: 31.100742 resistance:  75335               583
 * temperature: 31.201315 resistance:  74990               585
 * temperature: 31.300939 resistance:  74650               586
 * temperature: 31.401073 resistance:  74310               587
 * temperature: 31.500240 resistance:  73975               588
 * temperature: 31.601403 resistance:  73635               589
 * temperature: 31.700093 resistance:  73305               590
 * temperature: 31.800790 resistance:  72970               591
 * temperature: 31.900492 resistance:  72640               593
 * temperature: 32.000704 resistance:  72310        72306  594
 * temperature: 32.101431 resistance:  71980               595
 * temperature: 32.201140 resistance:  71655               596
 * temperature: 32.301358 resistance:  71330               597
 * temperature: 32.400536 resistance:  71010               598
 * temperature: 32.500217 resistance:  70690               599
 * temperature: 32.600406 resistance:  70370               600
 * temperature: 32.701107 resistance:  70050               602
 * temperature: 32.800742 resistance:  69735               603
 * temperature: 32.900882 resistance:  69420               604
 * temperature: 33.001534 resistance:  69105        69104  605
 * temperature: 33.101093 resistance:  68795               606
 * temperature: 33.201157 resistance:  68485               607
 * temperature: 33.300104 resistance:  68180               608
 * temperature: 33.401185 resistance:  67870               609
 * temperature: 33.501142 resistance:  67565               611
 * temperature: 33.601607 resistance:  67260               612
 * temperature: 33.700926 resistance:  66960               613
 * temperature: 33.800746 resistance:  66660               614
 * temperature: 33.901071 resistance:  66360               615
 * temperature: 34.000223 resistance:  66065        66061  616
 * temperature: 34.101567 resistance:  65765               617
 * temperature: 34.200027 resistance:  65475               618
 * temperature: 34.300689 resistance:  65180               619
 * temperature: 34.400144 resistance:  64890               620
 * temperature: 34.500101 resistance:  64600               622
 * temperature: 34.600562 resistance:  64310               623
 * temperature: 34.701534 resistance:  64020               624
 * temperature: 34.801267 resistance:  63735               625
 * temperature: 34.901503 resistance:  63450               626
 * temperature: 35.000474 resistance:  63170        63167  627
 * temperature: 35.101719 resistance:  62885               628
 * temperature: 35.201692 resistance:  62605               629
 * temperature: 35.300370 resistance:  62330               630
 * temperature: 35.401346 resistance:  62050               631
 * temperature: 35.501019 resistance:  61775               632
 * temperature: 35.601191 resistance:  61500               633
 * temperature: 35.700033 resistance:  61230               634
 * temperature: 35.801211 resistance:  60955               636
 * temperature: 35.901049 resistance:  60685               637
 * temperature: 36.001388 resistance:  60415        60415  638
 * temperature: 36.100360 resistance:  60150               639
 * temperature: 36.201705 resistance:  59880               640
 * temperature: 36.301673 resistance:  59615               641
 * temperature: 36.400243 resistance:  59355               642
 * temperature: 36.501208 resistance:  59090               643
 * temperature: 36.600765 resistance:  58830               644
 * temperature: 36.700818 resistance:  58570               645
 * temperature: 36.801371 resistance:  58310               646
 * temperature: 36.900481 resistance:  58055               647
 * temperature: 37.000082 resistance:  57800        57797  648
 * temperature: 37.100179 resistance:  57545               649
 * temperature: 37.200775 resistance:  57290               650
 * temperature: 37.301877 resistance:  57035               651
 * temperature: 37.401490 resistance:  56785               652
 * temperature: 37.501598 resistance:  56535               654
 * temperature: 37.600189 resistance:  56290               655
 * temperature: 37.701289 resistance:  56040               656
 * temperature: 37.800862 resistance:  55795               657
 * temperature: 37.900927 resistance:  55550               658
 * temperature: 38.001490 resistance:  55305        55306  659
 * temperature: 38.100488 resistance:  55065               660
 * temperature: 38.202050 resistance:  54820               661
 * temperature: 38.302036 resistance:  54580               662
 * temperature: 38.400419 resistance:  54345               663
 * temperature: 38.501390 resistance:  54105               664
 * temperature: 38.600748 resistance:  53870               665
 * temperature: 38.700594 resistance:  53635               666
 * temperature: 38.800934 resistance:  53400               667
 * temperature: 38.901772 resistance:  53165               668
 * temperature: 39.000951 resistance:  52935        52934  669
 * temperature: 39.100617 resistance:  52705               670
 * temperature: 39.200773 resistance:  52475               671
 * temperature: 39.301424 resistance:  52245               672
 * temperature: 39.400371 resistance:  52020               673
 * temperature: 39.502016 resistance:  51790               674
 * temperature: 39.601944 resistance:  51565               675
 * temperature: 39.700127 resistance:  51345               676
 * temperature: 39.801032 resistance:  51120               677
 * temperature: 39.900180 resistance:  50900               678
 * temperature: 40.002081 resistance:  50675        50677  679
 * temperature: 40.102211 resistance:  50455               680
 * temperature: 40.200542 resistance:  50240               681
 * temperature: 40.301651 resistance:  50020               682
 * temperature: 40.400947 resistance:  49805               683
 * temperature: 40.500726 resistance:  49590               684
 * temperature: 40.600994 resistance:  49375               685
 * temperature: 40.701755 resistance:  49160               686
 * temperature: 40.800653 resistance:  48950               687
 * temperature: 40.900029 resistance:  48740               688
 * temperature: 41.002272 resistance:  48525        48528  689
 * temperature: 41.100237 resistance:  48320               690
 * temperature: 41.201077 resistance:  48110               691
 * temperature: 41.302413 resistance:  47900               692
 * temperature: 41.401821 resistance:  47695               693
 * temperature: 41.501711 resistance:  47490               694
 * temperature: 41.602089 resistance:  47285               695
 * temperature: 41.700491 resistance:  47085               696
 * temperature: 41.801843 resistance:  46880               696
 * temperature: 41.901206 resistance:  46680               697
 * temperature: 42.001049 resistance:  46480        46482  698
 * temperature: 42.101377 resistance:  46280               699
 * temperature: 42.202195 resistance:  46080               700
 * temperature: 42.300968 resistance:  45885               701
 * temperature: 42.400215 resistance:  45690               702
 * temperature: 42.502504 resistance:  45490               703
 * temperature: 42.600149 resistance:  45300               704
 * temperature: 42.700845 resistance:  45105               705
 * temperature: 42.802032 resistance:  44910               706
 * temperature: 42.901102 resistance:  44720               707
 * temperature: 43.000648 resistance:  44530        44533  708
 * temperature: 43.100673 resistance:  44340               709
 * temperature: 43.201183 resistance:  44150               710
 * temperature: 43.302181 resistance:  43960               711
 * temperature: 43.400995 resistance:  43775               712
 * temperature: 43.500281 resistance:  43590               712
 * temperature: 43.600043 resistance:  43405               713
 * temperature: 43.700285 resistance:  43220               714
 * temperature: 43.801012 resistance:  43035               715
 * temperature: 43.902229 resistance:  42850               716
 * temperature: 44.001184 resistance:  42670        42675  717
 * temperature: 44.100610 resistance:  42490               718
 * temperature: 44.200512 resistance:  42310               719
 * temperature: 44.300895 resistance:  42130               720
 * temperature: 44.401763 resistance:  41950               721
 * temperature: 44.500297 resistance:  41775               722
 * temperature: 44.602134 resistance:  41595               722
 * temperature: 44.701619 resistance:  41420               723
 * temperature: 44.801579 resistance:  41245               724
 * temperature: 44.902019 resistance:  41070               725
 * temperature: 45.000052 resistance:  40900        40904  726
 * temperature: 45.101449 resistance:  40725               727
 * temperature: 45.200422 resistance:  40555               728
 * temperature: 45.302795 resistance:  40380               729
 * temperature: 45.402723 resistance:  40210               730
 * temperature: 45.500169 resistance:  40045               730
 * temperature: 45.601043 resistance:  39875               731
 * temperature: 45.702403 resistance:  39705               732
 * temperature: 45.801251 resistance:  39540               733
 * temperature: 45.900565 resistance:  39375               734
 * temperature: 46.000350 resistance:  39210        39213  735
 * temperature: 46.100610 resistance:  39045               736
 * temperature: 46.201349 resistance:  38880               737
 * temperature: 46.302571 resistance:  38715               737
 * temperature: 46.401192 resistance:  38555               738
 * temperature: 46.500276 resistance:  38395               739
 * temperature: 46.602945 resistance:  38230               740
 * temperature: 46.702981 resistance:  38070               741
 * temperature: 46.800345 resistance:  37915               742
 * temperature: 46.901322 resistance:  37755               743
 * temperature: 47.002783 resistance:  37595        37601  743
 * temperature: 47.101539 resistance:  37440               744
 * temperature: 47.200758 resistance:  37285               745
 * temperature: 47.300443 resistance:  37130               746
 * temperature: 47.400599 resistance:  36975               747
 * temperature: 47.501230 resistance:  36820               748
 * temperature: 47.602340 resistance:  36665               749
 * temperature: 47.700649 resistance:  36515               749
 * temperature: 47.802715 resistance:  36360               750
 * temperature: 47.901957 resistance:  36210               751
 * temperature: 48.001665 resistance:  36060        36063  752
 * temperature: 48.101842 resistance:  35910               753
 * temperature: 48.202492 resistance:  35760               754
 * temperature: 48.300241 resistance:  35615               754
 * temperature: 48.401835 resistance:  35465               755
 * temperature: 48.500505 resistance:  35320               756
 * temperature: 48.603059 resistance:  35170               757
 * temperature: 48.702666 resistance:  35025               758
 * temperature: 48.802739 resistance:  34880               758
 * temperature: 48.903284 resistance:  34735               759
 * temperature: 49.000812 resistance:  34595        34595  760
 * temperature: 49.102294 resistance:  34450               761
 * temperature: 49.200735 resistance:  34310               762
 * temperature: 49.303172 resistance:  34165               762
 * temperature: 49.402543 resistance:  34025               763
 * temperature: 49.502377 resistance:  33885               764
 * temperature: 49.602678 resistance:  33745               765
 * temperature: 49.703451 resistance:  33605               766
 * temperature: 49.801074 resistance:  33470               766
 * temperature: 49.902784 resistance:  33330        33195  767
 */
