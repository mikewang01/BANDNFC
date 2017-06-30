
#include "main.h"

void Butterworth_Filter_Init()
{
	I8U i;
	BUTT_CTX *bw_ctx = &cling.butterworth_filter_context;
#if 1
	bw_ctx->firstElement_lp = TRUE;
	bw_ctx->firstElement_hp = TRUE;
	
	for (i=0; i<(BUTTORD+1); i++) {
		bw_ctx->x_lp[i] = 0.0;
		bw_ctx->y_lp[i] = 0.0;
		bw_ctx->x_hp[i] = 0.0;
		bw_ctx->y_hp[i] = 0.0;
	}
#endif
	bw_ctx->pre_guess  = 48;
	bw_ctx->post_guess = 48;
	bw_ctx->pre_error  = 1.0;
	bw_ctx->post_error = 1.0;
	bw_ctx->gain       = 1.0;
}


double Butterworth_Filter_LP(double invar)
{
  I8U i;
	double numerator;
	double denominator;
	
// low pass filter, cut frequency 6 Hz, 50 sample/sec
  const double a[BUTTORD+1] = {1.000000000000000000, -3.782050676232970000, 5.83747836187128000, -4.57774375976233000, 1.819272330918000000, -0.292583915178916000};   // Q13
  const double b[BUTTORD+1] = {0.000136635675470833,  0.000683178377354167, 0.00136635675470833,  0.00136635675470833, 0.000683178377354167,  0.000136635675470833};   // Q20

	BUTT_CTX *bw_ctx = &cling.butterworth_filter_context;
	
	for(i=BUTTORD;i>0;i--) {
		bw_ctx->y_lp[i] = bw_ctx->y_lp[i-1];
		bw_ctx->x_lp[i] = bw_ctx->x_lp[i-1];
	}
	
	bw_ctx->x_lp[0] = invar;

	if(bw_ctx->firstElement_lp) {
		bw_ctx->firstElement_lp = FALSE;		
		bw_ctx->y_lp[0] = invar;
		return invar;
	}

	numerator = 0.0;       // b *x
	for (i=0; i<(BUTTORD+1); i++) {
		 numerator += (b[i] * (bw_ctx->x_lp[i]) );
	}
	
	denominator = 0.0;    // a * y
	for (i=1; i<(BUTTORD+1); i++) {
		denominator += (a[i] * (bw_ctx->y_lp[i]) ) ;
	}
	
	numerator  -= denominator;

	bw_ctx->y_lp[0] = numerator;
	return bw_ctx->y_lp[0];
}

double Butterworth_Filter_HP(double invar)
{
  I8U i;
	double numerator;
	double denominator;
	double a[BUTTORD + 1] = {1.000000000000000, -4.69504062610034, 8.82614592256438, -8.30396669308534, 3.90989399411579, -0.737026189748336};    // 1.0Hz, five orders
	double b[BUTTORD + 1] = {0.858502294550443, -4.29251147275222, 8.58502294550443, -8.58502294550443, 4.29251147275222, -0.858502294550443};
	
	HEARTRATE_CTX *hr_ctx = &cling.hr;
	BUTT_CTX *bw_ctx = &cling.butterworth_filter_context;

#ifndef __YLF__
	if(hr_ctx->b_walkstate){//if(hr_ctx->b_walkstate) {//1.5Hz
		a[1] = -4.390276194261; a[2] = 7.742869540801; a[3] = -6.854349350896; a[4] =  3.04468530918; a[5] = -0.5427513749335;
		b[0] = 0.7367166178147; b[1] = -3.683583089074; b[2] = 7.367166178147; b[3] = -7.367166178147; b[4] = 3.683583089074; b[5] = -0.7367166178147;
	}else if(hr_ctx->b_runstate){//if(hr_ctx->b_runstate){//2Hz
		a[1] = -4.187300047864; a[2] = 7.069722752792; a[3] = -6.009958148187; a[4] =  2.570429302524; a[5] = -0.4422091823996;
		b[0] = 0.6649881073052; b[1] = -3.324940536526; b[2] = 6.649881073052; b[3] = -6.649881073052; b[4] = 3.324940536526; b[5] = -0.6649881073052;
	}
#endif
	for(i=BUTTORD;i>0;i--) {
		bw_ctx->y_hp[i] = bw_ctx->y_hp[i-1];
		bw_ctx->x_hp[i] = bw_ctx->x_hp[i-1];
	}
	
	bw_ctx->x_hp[0] = (double)invar;

	if(bw_ctx->firstElement_hp) {
		bw_ctx->firstElement_hp = FALSE;		
		bw_ctx->y_hp[0] = invar;
		return invar;
	}

	numerator = 0.0;       // b *x
	for (i=0; i<(BUTTORD+1); i++) {
		 numerator += (b[i] * (bw_ctx->x_hp[i]) );
	}
	
	denominator = 0.0;    // a * y
	for (i=1; i<(BUTTORD+1); i++) {
		denominator += (a[i] * (bw_ctx->y_hp[i]) ) ;
	}
	
	numerator  -= denominator;

	bw_ctx->y_hp[0] = numerator;
	return (I16S)bw_ctx->y_hp[0];
}

I8U Kalman_Filter(I8U sample_width)
{
	BUTT_CTX *bw_ctx = &cling.butterworth_filter_context;
	
	bw_ctx->pre_guess = bw_ctx->post_guess;
	bw_ctx->pre_error = bw_ctx->post_error + 0.0004;
	
	bw_ctx->gain = (double)bw_ctx->pre_guess / (bw_ctx->pre_guess + 256.00);
	
	bw_ctx->post_guess = (I8U) ( ( bw_ctx->pre_guess + bw_ctx->gain * (sample_width - bw_ctx->pre_guess) ) + 0.5 );
	bw_ctx->post_error = (1.0 - bw_ctx->gain) * bw_ctx->pre_error;

  return bw_ctx->post_guess;
}





