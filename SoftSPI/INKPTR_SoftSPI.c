/*================================================================================================*/
// Part 1: Libraries
#include "INKPTR_SoftSPI.h"
#include "ch32v00x.h"


/*================================================================================================*/
// Part 2: Time delay function
static void SoftSPI_CSDelay(void)
{
}
static void SoftSPI_SCKDelay(void)
{
}


/*================================================================================================*/
// Part 3: Pin packages
static void SoftSPI_EditSCK(uint8_t Dat)
{
	if(Dat) {GPIO_WriteBit(SoftSPI_SCK_Port, SoftSPI_SCK_Pin, Bit_SET);}
    else    {GPIO_WriteBit(SoftSPI_SCK_Port, SoftSPI_SCK_Pin, Bit_RESET);}
	SoftSPI_SCKDelay();
}
static void SoftSPI_EditMOSI(uint8_t Dat)
{
	if(Dat) {GPIO_WriteBit(SoftSPI_MOSI_Port, SoftSPI_MOSI_Pin, Bit_SET);}
    else    {GPIO_WriteBit(SoftSPI_MOSI_Port, SoftSPI_MOSI_Pin, Bit_RESET);}
}
static uint8_t SoftSPI_ReadMISO(void)
{
    if(GPIO_ReadInputDataBit(SoftSPI_MISO_Port, SoftSPI_MISO_Pin) == Bit_SET)	{return 1;}
    else																	    {return 0;}
}


/*================================================================================================*/
// Part 4: Timing modules

/**
 * @fn      SoftSPI_SwapBytes
 * 
 * @brief   Initialize the corresponding pins.
 * 
 * @param   SendByte	- byte data to send.
 * 
 * @return  The received acknowledgment.
 */
uint8_t SoftSPI_SwapBytes(uint8_t SendByte)
{
	uint8_t i, ReceiveByte = 0x00;

	#if		(SoftSPI_Mode == 0)
	for(i = 0 ; i < 8 ; i++)
	{
		SoftSPI_EditMOSI(SendByte & (0x80>>i));
		SoftSPI_EditSCK(1);
		if(SoftSPI_ReadMISO())	{ReceiveByte |= 0x80>>i;}
		SoftSPI_EditSCK(0);
	}
	#elif	(SoftSPI_Mode == 1)
	for(i = 0 ; i < 8 ; i++)
	{
		SoftSPI_EditSCK(1);
		SoftSPI_EditMOSI(SendByte & (0x80>>i));
		SoftSPI_EditSCK(0);
		if(SoftSPI_ReadMISO())	{ReceiveByte |= 0x80>>i;}
	}
	#elif	(SoftSPI_Mode == 2)
	for(i = 0 ; i < 8 ; i++)
	{
		SoftSPI_EditMOSI(SendByte & (0x80>>i));
		SoftSPI_EditSCK(0);
		if(SoftSPI_ReadMISO())	{ReceiveByte |= 0x80>>i;}
		SoftSPI_EditSCK(1);
	}
	#elif	(SoftSPI_Mode == 3)
	for(i = 0 ; i < 8 ; i++)
	{
		SoftSPI_EditSCK(0);
		SoftSPI_EditMOSI(SendByte&(0x80>>i));
		SoftSPI_EditSCK(1);
		if(SoftSPI_ReadMISO())	{ReceiveByte |= 0x80>>i;}
	}
	#else
	#error	"SoftSPI_Mode must be 0, 1, 2 or 3"
	#endif

	return ReceiveByte;
}

/**
 * @fn      SoftSPI_Stop
 * 
 * @brief   SoftSPI stop timing.
 * 
 * @param   none
 * 
 * @return  none
 */
void SoftSPI_Stop(void)
{
	GPIO_WriteBit(SoftSPI_CS_Port, SoftSPI_CS_Pin, Bit_SET);
	SoftSPI_CSDelay();
}

/**
 * @fn      SoftSPI_Start
 * 
 * @brief   SoftSPI start timing.
 * 
 * @param   none
 * 
 * @return  none
 */
void SoftSPI_Start(void)
{
	GPIO_WriteBit(SoftSPI_CS_Port, SoftSPI_CS_Pin, Bit_RESET);
	SoftSPI_CSDelay();
}


/*================================================================================================*/
// Part 5: Initialization function
/**
 * @fn      SoftSPI_Init
 * 
 * @brief   Initialize the corresponding pins.
 * 
 * @param   none
 * 
 * @return  none
 */
void SoftSPI_Init(void)
{
    RCC_APB2PeriphClockCmd(SoftSPI_RCC_APB2Periph, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    // CS
    GPIO_InitStructure.GPIO_Pin		= SoftSPI_CS_Pin;
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed	= SoftSPI_GPIO_Speed;
    GPIO_Init(SoftSPI_CS_Port, &GPIO_InitStructure);
	SoftSPI_Stop();
    // SCK
    GPIO_InitStructure.GPIO_Pin		= SoftSPI_SCK_Pin;
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed	= SoftSPI_GPIO_Speed;
    GPIO_Init(SoftSPI_SCK_Port, &GPIO_InitStructure);
    // MOSI
    GPIO_InitStructure.GPIO_Pin		= SoftSPI_MOSI_Pin;
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed	= SoftSPI_GPIO_Speed;
    GPIO_Init(SoftSPI_MOSI_Port, &GPIO_InitStructure);
    // MISO
    GPIO_InitStructure.GPIO_Pin		= SoftSPI_MISO_Pin;
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed	= SoftSPI_GPIO_Speed;
    GPIO_Init(SoftSPI_MISO_Port, &GPIO_InitStructure);

	SoftSPI_Stop();
	if(SoftSPI_Mode < 2)	{SoftSPI_EditSCK(0);}
	else					{SoftSPI_EditSCK(1);}
}
