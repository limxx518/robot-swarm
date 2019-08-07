/**********************************************************************
* ?2007 Microchip Technology Inc.
*
* FileName:        ADDRERR.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       PIC24Fxxxx
* Compiler:        MPLAB?C30 v3.00 or higher
*
* SOFTWARE LICENSE AGREEMENT:
* Microchip Technology Incorporated ("Microchip") retains all 
* ownership and intellectual property rights in the code accompanying
* this message and in all derivatives hereto.  You may use this code,
* and any derivatives created by any person or entity by or on your 
* behalf, exclusively with Microchip's proprietary products.  Your 
* acceptance and/or use of this code constitutes agreement to the 
* terms and conditions of this notice.
*
* CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS". NO 
* WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT 
* NOT LIMITED TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS 
* CODE, ITS INTERACTION WITH MICROCHIP'S PRODUCTS, COMBINATION WITH 
* ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
*
* YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE 
* LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR
* BREACH OF STATUTORY DUTY), STRICT LIABILITY, INDEMNITY, 
* CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
* EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR 
* EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE, HOWSOEVER 
* CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE
* DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW, 
* MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS
* CODE, SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP 
* SPECIFICALLY TO HAVE THIS CODE DEVELOPED.
*
* You agree that you are solely responsible for testing the code and 
* determining its suitability.  Microchip has no obligation to modify,
* test, certify, or support the code.
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author        Date      	Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Albert Z.		04/18/08	First release of source file
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
* Code Tested on:
* Explorer 16 Demo board with PIC24FJ128GA010 controller
* The Processor starts with the External Crystal.
*
**********************************************************************/

#include "p24fxxxx.h"


// JTAG/Code Protect/Write Protect/Clip-on Emulation mode
// Watchdog Timer/ICD pins select
_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx2) 
// Disable CLK switch and CLK monitor, OSCO or Fosc/2, HS oscillator,
// Primary oscillator
_CONFIG2(FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_XT & FNOSC_PRI)

void (*errMaker(void))(void);	// Make An Address Error by CASE d.
void (*errMaker2(void))(void);	// Make An Address Error by CASE e.

/* 
 * Define a pointer which is point to an address.
 * But a misaligned data word fetch is attempted
 * so as to make an address error trap.
 */
unsigned int *addrPtr = (unsigned int *) 0xFFFF;	
unsigned int temp;

int main (void)
{

// Disable Watch Dog Timer
	RCONbits.SWDTEN = 0;

	/* 
	 * Create Address Error Trap.
	 * A misaligned data word fetch is attempted.
 	 */
	*addrPtr = *addrPtr + 1;    // [CASE a]

	/* 
	 * Create Address Error Trap.
	 * A bit manipulation instruction using the Indirect Addressing
	 * mode with the LSb of the effective address set to ¡®1¡¯.
 	 */
// 	temp = (*addrPtr)<<1; 		// [CASE b]

	/* 
	 * Create Address Error Trap.
	 * A data fetch from unimplemented data address space is attempted.
 	 */
//	temp = *(unsigned int *)(0x7000);    // [CASE c]: PIC24FJ128GA010 has 8KB RAM.

	/* 
	 * Create Address Error Trap.
	 * Execution of a "GOTO #literal" instruction, where literal is an  
   	 * unimplemented program memory address.
 	 */
//	errMaker();			// [CASE d]

	/* 
	 * Create Address Error Trap.
	 * Executing instructions after modifying the PC to point to unimplemented 
	 * program memory addresses. The PC may be modified by loading a value into
     * the stack and executing a RETURN instruction.
 	 */
//	errMaker2();		// [CASE e]: Address Trap will not return to correct address.

	while(1);			// stop here if return to upper function

	return 0;
}

