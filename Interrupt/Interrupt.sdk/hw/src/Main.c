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
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID
#define INTC XScuGic
#define INTC_HANDLER XIntc_InterruptHandler
// Function Definitions
void GpioHandler(void *CallBackRef);
int GpioIntrExample(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 DeviceId, u16 IntrId, u16 IntrMask, u32 *DataRead);
int GpioSetupIntrSystem(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 DeviceId, u16 IntrId, u16 IntrMask);
void GpioDisableIntr(INTC *IntcInstancePtr, XGpio *InstancePtr, u16 IntrId, u16 IntrMask);

XGpio Gpio;
INTC Intc;
static u16 GlobalIntrMask;

int main()
{



    return 0;
}
