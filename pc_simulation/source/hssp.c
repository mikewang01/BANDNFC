#include "main.h"
#include "issp_extern.h"
#include "issp_directives.h"
#include "issp_errorS.h"
#include "issp_defs.h"

unsigned int  iBlockCounter;
unsigned int  iChecksumData;
unsigned int  iChecksumTarget;
extern unsigned char *abTargetDataOUT;

/* ========================================================================= */
// ErrorTrap()
// Return is not valid from main for PSOC, so this ErrorTrap routine is used.
// For some systems returning an error code will work best. For those, the 
// calls to ErrorTrap() should be replaced with a return(bErrorNumber). For
// other systems another method of reporting an error could be added to this
// function -- such as reporting over a communcations port.
/* ========================================================================= */
I8U ErrorTrap(char bErrorNumbers)
{
	return (I8U)bErrorNumbers;
}

I8U HSSP_program_main()
{
    // -- This example section of commands show the high-level calls to -------
    // -- perform Target Initialization, SilcionID Test, Bulk-Erase, Target ---
    // -- RAM Load, FLASH-Block Program, and Target Checksum Verification. ----
		//
		// Initialize the buffer pointer
		abTargetDataOUT = (unsigned char *)cling.ui.p_oled_up;
	
#ifdef USE_TP
    InitTP();
#endif
    SetSDATAHiZ();
    SetSCLKStrong();
    SCLKLow();
    SetXRESStrong();
    DeassertXRES();

    // >>>> ISSP Programming Starts Here <<<<
    
    // Acquire the device through reset or power cycle
#ifdef RESET_MODE
    // Initialize the Host & Target for ISSP operations
		fIsError = fXRESInitializeTargetForISSP();
    if (fIsError) {
        return ErrorTrap(fIsError);
    }
#else
    // Initialize the Host & Target for ISSP operations
		fIsError = fPowerCycleInitializeTargetForISSP();
    if (fIsError) {
        return ErrorTrap(fIsError);
    }
#endif
        
    // Run the SiliconID Verification, and proceed according to result.
		fIsError = fVerifySiliconID();
    if (fIsError) { 
        return ErrorTrap(fIsError);
    }
		//  debugging code
		//BASE_delay_msec(10);
#ifdef USE_TP
    SetTPHigh();    // Only used of Test Points are enabled  
#endif
    
    // Bulk-Erase the Device.
		fIsError = fEraseTarget();
		if (fIsError) {
        return ErrorTrap(fIsError);
    }
    		//  debugging code
		//BASE_delay_msec(10);

#ifdef USE_TP
    SetTPLow();    // Only used of Test Points are enabled
#endif
	
    //==============================================================//
    // Program Flash blocks with predetermined data. In the final application
    // this data should come from the HEX output of PSoC Designer.
    iChecksumData = 0;     // Calculte the device checksum as you go
    for (iBlockCounter=0; iBlockCounter<BLOCKS_PER_BANK; iBlockCounter++) {

        LoadProgramData((I8U)iBlockCounter);			//this loads CY8C24894-24LTXI (the programmer) with test data
        iChecksumData += iLoadTarget();											
        																		
#ifdef USE_TP
        SetTPHigh();    // Only used of Test Points are enabled 
#endif

			fIsError = fProgramTargetBlock((I8U)iBlockCounter);
				if (fIsError) {
						return ErrorTrap(fIsError);
				}
        
#ifdef USE_TP
				SetTPLow();    // Only used of Test Points are enabled  
#endif
		
				fIsError = fReadStatus();
				if (fIsError) { 
					return ErrorTrap(fIsError);
				}
    }    
    
    //=======================================================//
    // Doing Verify
		// Verify included for completeness in case host desires to do a stand-alone verify at a later date.
    for (iBlockCounter=0; iBlockCounter<BLOCKS_PER_BANK; iBlockCounter++) {
      	LoadProgramData((I8U) iBlockCounter);
			
			fIsError = fVerifySetup((I8U)iBlockCounter);
        if (fIsError) {
            return ErrorTrap(fIsError);
        }

				fIsError = fReadStatus();
				if (fIsError) { 
						return ErrorTrap(fIsError);
				}

				fIsError = fReadByteLoop();
				if (fIsError) {
					return ErrorTrap(fIsError);
				}			
    }

		//=======================================================//
    // Program security data into target PSoC. In the final application this 
    // data should come from the HEX output of PSoC Designer.
		        
    // Load one bank of security data from hex file into buffer
		fIsError = fLoadSecurityData();
    if (fIsError) {
        return ErrorTrap(fIsError);
    }
    
		// Secure one bank of the target flash
		fIsError = fSecureTargetFlash();
    if (fIsError) {
        return ErrorTrap(fIsError);
    }

    //==============================================================//
    //Do READ-SECURITY after SECURE    
    //Load one bank of security data from hex file into buffer
    //loads abTargetDataOUT[] with security data that was used in secure bit stream
		fIsError = fLoadSecurityData();
		if (fIsError) {
				return ErrorTrap(fIsError);
		}

		fIsError = fReadSecurity();
		if (fIsError) { 
				return ErrorTrap(fIsError);
		}
    
    //=======================================================//     
    //Doing Checksum after READ-SECURITY
    iChecksumTarget = 0;

		fIsError = fAccTargetBankChecksum(&iChecksumTarget);
		if (fIsError) {
        return ErrorTrap(fIsError);
    }
    
    if ((iChecksumTarget & 0xFFFF) != (iChecksumData & 0xFFFF)){
        return ErrorTrap(READ_SECURITY_ERROR);
    }
     
    // *** SUCCESS *** 
    // You may want to restart Your Target PSoC Here.
    ReStartTarget();
		
		return PASS;
}
