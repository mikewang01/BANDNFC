
#include "main.h"

void Butterworth_Filter_Init()
{
	I8U i;
	
	HEARTRATE_CTX *hr_ctx = &cling.hr;
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)
	hr_ctx->butterworth_filter_context.firstElement_hp = TRUE;
	for (i=0; i<(BUTTORD+1); i++) {
		hr_ctx->butterworth_filter_context.x_hp[i] = 0.0;
		hr_ctx->butterworth_filter_context.y_hp[i] = 0.0;
	}
#else
	hr_ctx->butterworth_filter_context.firstElement_lp = TRUE;
	hr_ctx->butterworth_filter_context.firstElement_hp = TRUE;
	
	for (i=0; i<(BUTTORD+1); i++) {
		hr_ctx->butterworth_filter_context.x_lp[i] = 0.0;
		hr_ctx->butterworth_filter_context.y_lp[i] = 0.0;
		hr_ctx->butterworth_filter_context.x_hp[i] = 0.0;
		hr_ctx->butterworth_filter_context.y_hp[i] = 0.0;
	}
#endif
	hr_ctx->butterworth_filter_context.pre_guess  = 48;
	hr_ctx->butterworth_filter_context.post_guess = 48;
	hr_ctx->butterworth_filter_context.pre_error  = 1.0;
	hr_ctx->butterworth_filter_context.post_error = 1.0;
	hr_ctx->butterworth_filter_context.gain       = 1.0;
}

#if 0
I16S Butterworth_Filter_LP(I16S invar)
{
  I8U i;
	I32S numerator;
	I32S denominator;

// low pass filter, cut frequency 6 Hz, 50 sample/sec
  const I16S a[BUTTORD+1] = {8192, -21090, 24526, -15161,  4921, -662};   // Q13
  const I16S b[BUTTORD+1] = {2901,  14507, 29013,  29013, 14507, 2901};   // Q20
	
	HEARTRATE_CTX *hr_ctx = &cling.hr;
	
	for(i=BUTTORD;i>0;i--) {
		hr_ctx->butterworth_filter_context.y[i] = hr_ctx->butterworth_filter_context.y[i-1];
		hr_ctx->butterworth_filter_context.x[i] = hr_ctx->butterworth_filter_context.x[i-1];
	}
	
	hr_ctx->butterworth_filter_context.x[0] = invar;

	if(hr_ctx->butterworth_filter_context.firstElement) {
		hr_ctx->butterworth_filter_context.firstElement = FALSE;		
		hr_ctx->butterworth_filter_context.y[0] = invar;
		return invar;
	}

	numerator = 0;       // b *x
	for (i=0; i<(BUTTORD+1); i++) {
		 numerator += (b[i] * (hr_ctx->butterworth_filter_context.x[i]) );
	}
	
#if 1
	if (numerator > MAX_INT)
		numerator = MAX_INT;
	else if (numerator < MIN_INT)
		numerator = MIN_INT;
#endif
	
	denominator = 0;    // a * y
	for (i=1; i<(BUTTORD+1); i++) {
		denominator += (a[i] * (hr_ctx->butterworth_filter_context.y[i]) ) ;
	}

#if 1
	if (denominator > MAX_INT)
		denominator = MAX_INT;
	else if (denominator < MIN_INT)
		denominator = MIN_INT;
#endif
	
	numerator >>= 7;
	numerator  -= denominator;
	numerator >>= 13;

	hr_ctx->butterworth_filter_context.y[0] = numerator;
	return hr_ctx->butterworth_filter_context.y[0];
}
#endif
#if defined(_CLINGBAND_UV_MODEL_) || defined(_CLINGBAND_NFC_MODEL_)	|| defined(_CLINGBAND_VOC_MODEL_)
double Butterworth_Filter_BP(double invar)
{
  I8U i;
	double numerator;
	double denominator;
	double a[BUTTORD + 1] = {1.00000000000000, -4.528663218653,  8.73235856322200, -9.23466783907, 5.67242341648200, -1.919026050973,  0.27805991763450};    // 1.0Hz, five orders
	double b[BUTTORD + 1] = {0.01809893300751,  0.000000000000, -0.05429679902254,  0.00000000000, 0.05429679902254,  0.000000000000, -0.01809893300751};
	
	HEARTRATE_CTX *hr_ctx = &cling.hr;
	if(hr_ctx->b_walkstate) {//1.5Hz~6Hz
		a[1] = -4.52937616203500; a[2] = 8.831451542839; a[3] = -9.52325009792; a[4] =  6.004100852292; a[5] = -2.098658103308; a[6] = 0.3174254774301;
		b[0] = 0.01382007388379; b[2] = -0.04146022165138; b[4] = 0.04146022165138; b[6] = -0.01382007388379;
	}else if(hr_ctx->b_runstate){//2Hz~6Hz
		a[1] = -4.52708794761300; a[2] =  8.92424207998700; a[3] = -9.80821347628600; a[4] =  6.34460209714600; a[5] = -2.291177215215;a[6] = 0.3617959282279;
		b[0] =  0.01018257673644; b[2] = -0.03054773020931; b[4] =  0.03054773020931; b[6] = -0.01018257673644;
	}
	for(i=BUTTORD;i>0;i--) {
		hr_ctx->butterworth_filter_context.y_hp[i] = hr_ctx->butterworth_filter_context.y_hp[i-1];
		hr_ctx->butterworth_filter_context.x_hp[i] = hr_ctx->butterworth_filter_context.x_hp[i-1];
	}
	
	hr_ctx->butterworth_filter_context.x_hp[0] = (double)invar;

	if(hr_ctx->butterworth_filter_context.firstElement_hp) {
		hr_ctx->butterworth_filter_context.firstElement_hp = FALSE;		
		hr_ctx->butterworth_filter_context.y_hp[0] = invar;
		return invar;
	}

	numerator = 0.0;       // b *x
	for (i=0; i<(BUTTORD+1); i++) {
		 numerator += (b[i] * (hr_ctx->butterworth_filter_context.x_hp[i]) );
	}
	
	denominator = 0.0;    // a * y
	for (i=1; i<(BUTTORD+1); i++) {
		denominator += (a[i] * (hr_ctx->butterworth_filter_context.y_hp[i]) ) ;
	}
	
	numerator  -= denominator;

	hr_ctx->butterworth_filter_context.y_hp[0] = numerator;
	return (I16S)hr_ctx->butterworth_filter_context.y_hp[0];
}
#else
double Butterworth_Filter_LP(double invar)
{
  I8U i;
	double numerator;
	double denominator;
	
// low pass filter, cut frequency 6 Hz, 50 sample/sec
  const double a[BUTTORD+1] = {1.000000000000000000, -3.782050676232970000, 5.83747836187128000, -4.57774375976233000, 1.819272330918000000, -0.292583915178916000};   // Q13
  const double b[BUTTORD+1] = {0.000136635675470833,  0.000683178377354167, 0.00136635675470833,  0.00136635675470833, 0.000683178377354167,  0.000136635675470833};   // Q20

	HEARTRATE_CTX *hr_ctx = &cling.hr;
	
	for(i=BUTTORD;i>0;i--) {
		hr_ctx->butterworth_filter_context.y_lp[i] = hr_ctx->butterworth_filter_context.y_lp[i-1];
		hr_ctx->butterworth_filter_context.x_lp[i] = hr_ctx->butterworth_filter_context.x_lp[i-1];
	}
	
	hr_ctx->butterworth_filter_context.x_lp[0] = invar;

	if(hr_ctx->butterworth_filter_context.firstElement_lp) {
		hr_ctx->butterworth_filter_context.firstElement_lp = FALSE;		
		hr_ctx->butterworth_filter_context.y_lp[0] = invar;
		return invar;
	}

	numerator = 0.0;       // b *x
	for (i=0; i<(BUTTORD+1); i++) {
		 numerator += (b[i] * (hr_ctx->butterworth_filter_context.x_lp[i]) );
	}
	
	denominator = 0.0;    // a * y
	for (i=1; i<(BUTTORD+1); i++) {
		denominator += (a[i] * (hr_ctx->butterworth_filter_context.y_lp[i]) ) ;
	}
	
	numerator  -= denominator;

	hr_ctx->butterworth_filter_context.y_lp[0] = numerator;
	return hr_ctx->butterworth_filter_context.y_lp[0];
}

double Butterworth_Filter_HP(double invar)
{
  I8U i;
	double numerator;
	double denominator;
	double a[BUTTORD + 1] = {1.000000000000000, -4.69504062610034, 8.82614592256438, -8.30396669308534, 3.90989399411579, -0.737026189748336};    // 1.0Hz, five orders
	double b[BUTTORD + 1] = {0.858502294550443, -4.29251147275222, 8.58502294550443, -8.58502294550443, 4.29251147275222, -0.858502294550443};
	
	HEARTRATE_CTX *hr_ctx = &cling.hr;
#ifndef __YLF__
	if(hr_ctx->b_walkstate) {//1.5Hz
		a[1] = -4.390276194261; a[2] = 7.742869540801; a[3] = -6.854349350896; a[4] =  3.04468530918; a[5] = -0.5427513749335;
		b[0] = 0.7367166178147; b[1] = -3.683583089074; b[2] = 7.367166178147; b[3] = -7.367166178147; b[4] = 3.683583089074; b[5] = -0.7367166178147;
	}else if(hr_ctx->b_runstate){//2Hz
		a[1] = -4.187300047864; a[2] = 7.069722752792; a[3] = -6.009958148187; a[4] =  2.570429302524; a[5] = -0.4422091823996;
		b[0] = 0.6649881073052; b[1] = -3.324940536526; b[2] = 6.649881073052; b[3] = -6.649881073052; b[4] = 3.324940536526; b[5] = -0.6649881073052;
	}
#endif
	for(i=BUTTORD;i>0;i--) {
		hr_ctx->butterworth_filter_context.y_hp[i] = hr_ctx->butterworth_filter_context.y_hp[i-1];
		hr_ctx->butterworth_filter_context.x_hp[i] = hr_ctx->butterworth_filter_context.x_hp[i-1];
	}
	
	hr_ctx->butterworth_filter_context.x_hp[0] = (double)invar;

	if(hr_ctx->butterworth_filter_context.firstElement_hp) {
		hr_ctx->butterworth_filter_context.firstElement_hp = FALSE;		
		hr_ctx->butterworth_filter_context.y_hp[0] = invar;
		return invar;
	}

	numerator = 0.0;       // b *x
	for (i=0; i<(BUTTORD+1); i++) {
		 numerator += (b[i] * (hr_ctx->butterworth_filter_context.x_hp[i]) );
	}
	
	denominator = 0.0;    // a * y
	for (i=1; i<(BUTTORD+1); i++) {
		denominator += (a[i] * (hr_ctx->butterworth_filter_context.y_hp[i]) ) ;
	}
	
	numerator  -= denominator;

	hr_ctx->butterworth_filter_context.y_hp[0] = numerator;
	return (I16S)hr_ctx->butterworth_filter_context.y_hp[0];
}
#endif

I8U Kalman_Filter(I8U sample_width)
{
	HEARTRATE_CTX *hr_ctx = &cling.hr;
	
	hr_ctx->butterworth_filter_context.pre_guess = hr_ctx->butterworth_filter_context.post_guess;
	hr_ctx->butterworth_filter_context.pre_error = hr_ctx->butterworth_filter_context.post_error + 0.0004;
	
	hr_ctx->butterworth_filter_context.gain = (double)hr_ctx->butterworth_filter_context.pre_guess / (hr_ctx->butterworth_filter_context.pre_guess + 256.00);
	
	hr_ctx->butterworth_filter_context.post_guess = (I8U) ( ( hr_ctx->butterworth_filter_context.pre_guess + hr_ctx->butterworth_filter_context.gain * (sample_width - hr_ctx->butterworth_filter_context.pre_guess) ) + 0.5 );
	hr_ctx->butterworth_filter_context.post_error = (1.0 - hr_ctx->butterworth_filter_context.gain) * hr_ctx->butterworth_filter_context.pre_error;

  return hr_ctx->butterworth_filter_context.post_guess;
}





