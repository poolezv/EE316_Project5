/* Interrupts
 * Zander Poole
 * EE316 Project 5
 */

// Includes
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "stdio.h"
#include "xgpio.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xil_exception.h"
#include "xscugic.h"
#include "xtmrctr.h"


// Constants
#define GPIO_DEVICE_ID		XPAR_GPIO_0_DEVICE_ID
#define GPIO_CHANNEL1		1
#define INTC_GPIO_INTERRUPT_ID	XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define GPIO_ALL_BUTTONS	0xFFFF
#define BTN_0 0x0001
#define BTN_1 0x0002
#define BTN_2 0x0004
#define BUTTON_CHANNEL	 1
#define BUTTON_INTERRUPT XGPIO_IR_CH1_MASK
#define INTERRUPT_CONTROL_VALUE 0x7
#define INTC_DEVICE_ID	XPAR_SCUGIC_SINGLE_DEVICE_ID
#define INTC			XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler

#define TMRCTR_DEVICE_ID        XPAR_TMRCTR_0_DEVICE_ID
#define TMRCTR_INTERRUPT_ID     XPAR_FABRIC_TMRCTR_0_VEC_ID

#define PWM_PERIOD              20			 /* PWM period in (20 ns) */
#define TMRCTR_0                0            /* Timer 0 ID */
#define TMRCTR_1                1            /* Timer 1 ID */
#define CYCLE_PER_DUTYCYCLE     10           /* Clock cycles per duty cycle */
#define MAX_DUTYCYCLE           100          /* Max duty cycle */
#define DUTYCYCLE_DIVISOR       4            /* Duty cycle Divisor */
#define WAIT_COUNT              PWM_PERIOD   /* Interrupt wait counter */



// Function Definitions
void GpioHandler(void *CallBackRef);
//int GpioIntrExample(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 DeviceId, u16 IntrId, u16 IntrMask, u32 *DataRead);
//void startIntrSetup(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 DeviceId, u16 IntrId, u16 IntrMask);
int GpioSetupIntrSystem(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 DeviceId, u16 IntrId, u16 IntrMask);
void GpioDisableIntr(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 IntrId, u16 IntrMask);
static void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber);
static int TmrCtrSetupIntrSystem(INTC *IntcInstancePtr, XTmrCtr *InstancePtr, u16 DeviceId, u16 IntrId);
static void TmrCtrDisableIntr(INTC *IntcInstancePtr, u16 IntrId);
void setMotorPWM(int rawData);
void setBuzzerPWM(int rawData);

// Variable Definitions
XGpio Gpio;
INTC Intc;
static u16 GlobalIntrMask;
static volatile u32 IntrFlag;
XTmrCtr TimerCounterInst;  		// Servo
XTmrCtr TimerCounterInst0;			// Motor
XTmrCtr TimerCounterInst1;			// Buzzer
INTC InterruptController;
static int   PeriodTimerHit = FALSE;
static int   HighTimerHit = FALSE;



int main()
{
	init_platform();
	// Initialize xadc
	// HERE
	// Enable Interrupt
	printf("Initializing Timer\n");
	XTmrCtr_Initialize(&InterruptController, TMRCTR_DEVICE_ID);

//	printf("Initializing ");
//	TmrCtrSetupIntrSystem(&Intc, &TimerCounterInst0, TMRCTR_DEVICE_ID, TMRCTR_INTERRUPT_ID);

	printf("Initializing Interrupt\n");
	XGpio_Initialize(&Gpio, GPIO_DEVICE_ID);

	GpioSetupIntrSystem(&Intc, &Gpio, GPIO_DEVICE_ID, INTC_GPIO_INTERRUPT_ID, GPIO_CHANNEL1);

	while(1){

		// rawdata = Get raw data from ADC

//		setMotorPWM(rawData);
//		setBuzzerPWM(rawData);


		printf("Waiting\n");
		usleep(1000000);

	}


	cleanup_platform();
    return 0;
}


int GpioSetupIntrSystem(INTC *IntcInstancePtr, XGpio *InstancePtr,
			u16 DeviceId, u16 IntrId, u16 IntrMask)
{
	int Result;

	GlobalIntrMask = IntrMask;


#ifdef XPAR_INTC_0_DEVICE_ID

#ifndef TESTAPP_GEN
	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 */
	Result = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Result != XST_SUCCESS) {
		return Result;
	}

#endif /* TESTAPP_GEN */

	/* Hook up interrupt service routine */
	XIntc_Connect(IntcInstancePtr, IntrId,
		      (Xil_ExceptionHandler)GpioHandler, InstancePtr);

	/* Enable the interrupt vector at the interrupt controller */
	XIntc_Enable(IntcInstancePtr, IntrId);

#ifndef TESTAPP_GEN
	/*
	 * Start the interrupt controller such that interrupts are recognized
	 * and handled by the processor
	 */
	Result = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Result != XST_SUCCESS) {
		return Result;
	}

#endif /* TESTAPP_GEN */

#else /* !XPAR_INTC_0_DEVICE_ID */

#ifndef TESTAPP_GEN
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Result = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Result != XST_SUCCESS) {
		return XST_FAILURE;
	}
#endif /* TESTAPP_GEN */

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Result = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)GpioHandler, InstancePtr);
	if (Result != XST_SUCCESS) {
		return Result;
	}

	/* Enable the interrupt for the GPIO device.*/
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

	/*
	 * Enable the GPIO channel interrupts so that push button can be
	 * detected and enable interrupts for the GPIO device
	 */
	XGpio_InterruptEnable(InstancePtr, IntrMask);
	XGpio_InterruptGlobalEnable(InstancePtr);
	printf("Interrupt Started\n");
	/*
	 * Initialize the exception table and register the interrupt
	 * controller handler with the exception table
	 */
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)INTC_HANDLER, IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

void GpioHandler(void *CallbackRef)
{
	XGpio *GpioPtr = (XGpio *)CallbackRef;

	IntrFlag = 1;

	/* Clear the Interrupt */
	XGpio_InterruptClear(GpioPtr, GlobalIntrMask);

}

void GpioDisableIntr(INTC *IntcInstancePtr, XGpio *InstancePtr,
			u16 IntrId, u16 IntrMask)
{
	XGpio_InterruptDisable(InstancePtr, IntrMask);
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disable(IntcInstancePtr, IntrId);
#else
	/* Disconnect the interrupt */
	XScuGic_Disable(IntcInstancePtr, IntrId);
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif
	return;
}


static int TmrCtrSetupIntrSystem(INTC *IntcInstancePtr,
			XTmrCtr *TmrCtrInstancePtr, u16 DeviceId, u16 IntrId)
{
	 int Status;

#ifdef XPAR_INTC_0_DEVICE_ID
	/*
	 * Initialize the interrupt controller driver so that
	 * it's ready to use, specify the device ID that is generated in
	 * xparameters.h
	 */
	Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device
	 */
	Status = XIntc_Connect(IntcInstancePtr, IntrId,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				(void *)TmrCtrInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the timer counter can cause interrupts through the interrupt
	 * controller
	 */
	Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Enable the interrupt for the timer counter */
	XIntc_Enable(IntcInstancePtr, IntrId);
#else
	XScuGic_Config *IntcConfig;

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XScuGic_SetPriorityTriggerType(IntcInstancePtr, IntrId,
					0xA0, 0x3);

	/*
	 * Connect the interrupt handler that will be called when an
	 * interrupt occurs for the device.
	 */
	Status = XScuGic_Connect(IntcInstancePtr, IntrId,
				 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
				 TmrCtrInstancePtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/* Enable the interrupt for the Timer device */
	XScuGic_Enable(IntcInstancePtr, IntrId);
#endif /* XPAR_INTC_0_DEVICE_ID */

	/* Initialize the exception table */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception table */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
					(Xil_ExceptionHandler)
					INTC_HANDLER,
					IntcInstancePtr);

	/* Enable non-critical exceptions */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

void TmrCtrDisableIntr(INTC *IntcInstancePtr, u16 IntrId)
{
	/* Disconnect the interrupt for the timer counter */
#ifdef XPAR_INTC_0_DEVICE_ID
	XIntc_Disconnect(IntcInstancePtr, IntrId);
#else
	XScuGic_Disconnect(IntcInstancePtr, IntrId);
#endif
}

static void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	/* Mark if period timer expired */
	if (TmrCtrNumber == TMRCTR_0) {
		PeriodTimerHit = TRUE;
	}

	/* Mark if high time timer expired */
	if (TmrCtrNumber == TMRCTR_1) {
		HighTimerHit = TRUE;
	}
}

setMotorPWM(int rawData){

	u8  DutyCycle;
	u8  NoOfCycles;
	u8  Div;
	u32 Period;
	u32 HighTime;
	u64 WaitCount;
	float DutyCyclef;

	Div = DUTYCYCLE_DIVISOR;

	do{
		XTmrCtr_PwmDisable(&TimerCounterInst);

		Period = PWM_PERIOD;
		HighTime = PWM_PERIOD / Div--;

		DutyCycle = (u8)(((double)(rawData)/65535)*100);

		xil_printf("PWM Configured for Duty Cycle = %d\r\n", DutyCycle);

		/* Enable PWM */
		XTmrCtr_PwmEnable(&TimerCounterInst);

		NoOfCycles = 0;
		WaitCount = WAIT_COUNT;
		while (NoOfCycles < CYCLE_PER_DUTYCYCLE) {
			if (PeriodTimerHit == TRUE && HighTimerHit == TRUE) {
				PeriodTimerHit = FALSE;
				HighTimerHit = FALSE;
				WaitCount = WAIT_COUNT;
				NoOfCycles++;
			}
		}
	} while(DutyCycle < MAX_DUTYCYCLE);

	return 0;

}

setBuzzerPWM(int rawData){

	u8  DutyCycle;
	u8  NoOfCycles;
	u8  Div;
	u32 Period;
	u32 HighTime;
	u64 WaitCount;


	Div = DUTYCYCLE_DIVISOR;

	do{
		XTmrCtr_PwmDisable(&TimerCounterInst);

		Period = (u32)(65535 - rawData);
		HighTime = (u64)(Period / Div--);
		DutyCycle = (u8)(50);
		XTmrCtr_PwmConfigure(&TimerCounterInst, Period, HighTime);

		xil_printf("PWM Configured for Duty Cycle = %d\r\n", DutyCycle);

		/* Enable PWM */
		XTmrCtr_PwmEnable(&TimerCounterInst);

		NoOfCycles = 0;
		WaitCount = WAIT_COUNT;
		while (NoOfCycles < CYCLE_PER_DUTYCYCLE) {
			if (PeriodTimerHit == TRUE && HighTimerHit == TRUE) {
				PeriodTimerHit = FALSE;
				HighTimerHit = FALSE;
				WaitCount = WAIT_COUNT;
				NoOfCycles++;
			}
		}
	} while(DutyCycle < MAX_DUTYCYCLE);

	return;

}
