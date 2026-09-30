/*================================================================================================*/
// Part 1: Libraries and fixed parameter macro definitions
#include "INKPTR_OLEDspi.h"
#include "INKPTR_SoftSPI.h"
#include "ch32v00x.h"

#define OLED_ADD                        0x78

#define OLED_Model_Index_MultiplexRatio 0
#define OLED_Model_Index_COMPins        1
#define OLED_Model_Index_PageMax        2
#define OLED_Model_Index_ListMax        3

#define OLED_042ListAdjust              0x1c

#define OLED_MultiplexRatioCmd          0xa8
#define OLED_COMPinsCmd                 0xda
#define OLED_AddressingModeCmd          0x20
#define OLED_ListAreaCmd                0x21
#define OLED_PageAreaCmd                0x22
#define OLED_BrightnessCmd              0x81

#define OLED_PointerPageCmd             0xb0
#define OLED_PointerListCmd_LSN         0x0f
#define OLED_PointerListCmd_MSN         0x10

#define OLED_RollDerictionLeftFlag      0x80
#define OLED_RollDerictionCmd_Left      0x27
#define OLED_RollDerictionCmd_Right     0x26


/*================================================================================================*/
// Part 2: OLED mode parameters
static const uint8_t OLED_Model_Dat[][4] =
{
    {0x3f, 0x12, 7, 127}, // 128*64 Model
    {0x27, 0x12, 4, 71}   // 72*40 Model
};
static const uint8_t OLED_InitCmd[] =
{
    0xd5,0x80,  // Clock
    0xd3,0x00,  // Column offset
    0x40,       // Page offset
    0x8d,0x14,  // Use the internal charge pump
    0xd9,0xf1,  // Pre-charge cycle
    0xdB,0x40,  // VCOMH voltage
    0xa4        // RAM display
};


/*================================================================================================*/
// Part 3: Boundary checking function
static uint8_t OLED_ValueCheck(uint8_t Page_min, uint8_t Page_max, uint8_t List_min, uint8_t List_max)
{
    if(Page_min > Page_max)                                             {return 1;}
    if(List_min > List_max)                                             {return 1;}
    if(Page_max > OLED_Model_Dat[OLED_Model][OLED_Model_Index_PageMax]) {return 1;}
    if(List_max > OLED_Model_Dat[OLED_Model][OLED_Model_Index_ListMax]) {return 1;}
    return 0;
}


/*================================================================================================*/
// Part 4: Data header packages
static void OLED_Cmd(void)
{
    GPIO_WriteBit(OLED_CmdPin_Port, OLED_CmdPin_Pin, Bit_RESET);
    SoftSPI_Start();
}
static void OLED_Dat(void)
{
    GPIO_WriteBit(OLED_CmdPin_Port, OLED_CmdPin_Pin, Bit_SET);
    SoftSPI_Start();
}


/*================================================================================================*/
// Part 5: Functional function
/**
 * @fn      OLED_Set
 * 
 * @brief   Edit OLED display settings.
 * 
 * @param   | SetMode                   | Dat
 *          -----------------------------------------------------------------------------------------------------------
 *          | OLED_SetMode_Brightness   | (Value: 0 ~ 255)
 *          |                           | 
 *          | OLED_SetMode_OtherOptions | OLED_SetMode_X_Flip_Normal / OLED_SetMode_X_Flip_Mirror
 *          |                           | OLED_SetMode_Y_Flip_Normal / OLED_SetMode_Y_Flip_Mirror
 *          |                           | OLED_SetMode_Color_Normal / OLED_SetMode_Color_Invert
 *          |                           | OLED_SetMode_Roll_ENABLE / OLED_SetMode_Roll_DISABLE
 *          |                           | OLED_SetMode_Show_ENABLE / OLED_SetMode_Show_DISABLE
 * 
 * @return  none
 */
void OLED_Set(OLED_SetMode SetMode, uint8_t Dat)
{
    OLED_Cmd();
    if(SetMode == OLED_SetMode_Brightness)
    {
        SoftSPI_SwapBytes(OLED_BrightnessCmd);
        SoftSPI_SwapBytes(Dat);
    }
    else
    {
        SoftSPI_SwapBytes(Dat);
    }
    SoftSPI_Stop();
}

/**
 * @fn      OLED_Brush
 * 
 * @brief   Display a block in a byte style.
 * 
 * @param   Page_Begin  - the beginning of page add.
 *          Page_End    - the ending of page add.
 *          List_Begin  - the beginning of list add.
 *          List_End    - the ending of list add.
 *          Style_Byte  - a byte as the style of brush.
 * 
 * @return  none
 */
void OLED_Brush(uint8_t Page_Begin, uint8_t Page_End, uint8_t List_Begin, uint8_t List_End, uint8_t Style_Byte)
{
    uint8_t x, y;

    if(OLED_ValueCheck(Page_Begin, Page_End, List_Begin, List_End)) {return;}

    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Roll_DISABLE);

    for(y = Page_Begin ; y < (Page_End+1) ; y++)
    {
        OLED_Cmd();
        SoftSPI_SwapBytes(OLED_PointerPageCmd + y);
        SoftSPI_SwapBytes(OLED_PointerListCmd_LSN & List_Begin);
        SoftSPI_SwapBytes(OLED_PointerListCmd_MSN | (List_Begin>>4));
        SoftSPI_Stop();

        OLED_Dat();
        for(x = List_Begin ; x < (List_End+1) ; x++) {SoftSPI_SwapBytes(Style_Byte);}
        SoftSPI_Stop();
    }
}

/**
 * @fn      OLED_Clear
 *
 * @brief   Clear the content displayed on the screen.
 *
 * @param   none
 *
 * @return  none
 */
void OLED_Clear(void)
{
    OLED_Brush(0, OLED_Model_Dat[OLED_Model][OLED_Model_Index_PageMax], 0, OLED_Model_Dat[OLED_Model][OLED_Model_Index_ListMax], 0);
}

/**
 * @fn      OLED_Draw_CmdHead
 * 
 * @brief   Command header for drawing the screen.
 *          Later, data will be sent using "OLED_Draw_Data", and it must end with "OLED_Draw_CmdTail"!
 * 
 * @param   Page_Begin  - the beginning of page add.
 *          List_Begin  - the beginning of list add.
 * 
 * @return  none
 */
void OLED_Draw_CmdHead(uint8_t Page_Begin, uint8_t List_Begin)
{
    if(OLED_ValueCheck(0, Page_Begin, 0, List_Begin))    {return;}

    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Roll_DISABLE);

    OLED_Cmd();
    SoftSPI_SwapBytes(OLED_PointerPageCmd + Page_Begin);
    SoftSPI_SwapBytes(OLED_PointerListCmd_LSN & List_Begin);
    SoftSPI_SwapBytes(OLED_PointerListCmd_MSN | (List_Begin>>4));
    SoftSPI_Stop();

    OLED_Dat();
}

/**
 * @fn      OLED_Draw_CmdHead_WholeArea
 *
 * @brief   Command header for drawing the whole screen, draw starting from the origin of the refresh area.
 *          Suitable for addressing mode: OLED_AddressingMode_HorizontalMode / OLED_AddressingMode_VerticalMode.
 *          Later, data will be sent using "OLED_Draw_Data", and it must end with "OLED_Draw_CmdTail"!
 *
 * @param   none
 *
 * @return  none
 */
void OLED_Draw_CmdHead_WholeArea(void)
{
    OLED_Cmd();
    SoftSPI_SwapBytes(OLED_ListAreaCmd);
    if(OLED_Model == 2)
    {
        SoftSPI_SwapBytes(OLED_042ListAdjust);
        SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][OLED_Model_Index_ListMax] + OLED_042ListAdjust);
    }
    else
    {
        SoftSPI_SwapBytes(0);
        SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][OLED_Model_Index_ListMax]);
    }
    SoftSPI_SwapBytes(OLED_PageAreaCmd);
    SoftSPI_SwapBytes(0);
    SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][OLED_Model_Index_PageMax]);
    SoftSPI_Stop();

    OLED_Dat();
}

/**
 * @fn      OLED_Draw_CmdData
 *
 * @brief   Send the drawing data.
 *          This function is used in conjunction with "OLED_Draw_CmdHead".
 *
 * @param   Dat  - the drawing data.
 *
 * @return  none
 */
void OLED_Draw_CmdData(uint8_t Dat)
{
    SoftSPI_SwapBytes(Dat);
}

/**
 * @fn      OLED_Draw_CmdTail
 *
 * @brief   Command tail for drawing the screen.
 *          This function is used in conjunction with "OLED_Draw_CmdHead".
 *
 * @param   none
 *
 * @return  none
 */
void OLED_Draw_CmdTail(void)
{
    SoftSPI_Stop();
}

/**
 * @fn      OLED_Roll
 * 
 * @brief   Set up and enable the scrolling.
 * 
 * @param   Page_Begin  - the beginning of page add.
 *          Page_End    - the ending of page add.
 *          List_Begin  - the beginning of list add.
 *          List_End    - the ending of list add.
 *          RollMode    - select one from the scrolling modes.
 *              Scroll Left:    OLED_RollMode_Left_1 ~ OLED_RollMode_Left_8
 *              Scroll Right:   OLED_RollMode_Right_1 ~ OLED_RollMode_Right_8
 *              (The bigger the suffix value, the faster the speed.)
 * 
 * @return  none
 */
void OLED_Roll(uint8_t Page_Begin, uint8_t Page_End, uint8_t List_Begin, uint8_t List_End, OLED_RollMode RollMode)
{
    uint8_t SpeedTable[] = {3, 2, 1, 0, 6, 5, 4, 7};

    if(OLED_ValueCheck(Page_Begin, Page_End, List_Begin, List_End))  {return;}

    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Roll_DISABLE);

    OLED_Cmd();
    if(RollMode & OLED_RollDerictionLeftFlag)   {SoftSPI_SwapBytes(OLED_RollDerictionCmd_Left);   }
    else                                        {SoftSPI_SwapBytes(OLED_RollDerictionCmd_Right);  }
    SoftSPI_SwapBytes(0);
    SoftSPI_SwapBytes(Page_Begin);
    SoftSPI_SwapBytes(SpeedTable[RollMode & (~0x80)]);    
    SoftSPI_SwapBytes(Page_End);
    SoftSPI_SwapBytes(List_Begin);
    SoftSPI_SwapBytes(List_End);
    SoftSPI_Stop();

    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Roll_ENABLE);
}


/*================================================================================================*/
// Part 6: Initialization function
/**
 * @fn      OLED_Init
 * 
 * @brief   Complete the display setup initialization.
 * 
 * @param   AddressingMode  - select one to set addressing mode for display.
 *              OLED_AddressingMode_HorizontalMode
 *              OLED_AddressingMode_VerticalMode
 *              OLED_AddressingMode_PageMode
 *          X_FlipMode      - select one to set initial horizontal mirror flip.
 *              OLED_SetMode_X_Flip_Normal / OLED_SetMode_X_Flip_Mirror
 *          Y_FlipMode      - select one to set initial vertical mirror flip.
 *              OLED_SetMode_Y_Flip_Normal / OLED_SetMode_Y_Flip_Mirror
 *          ColorMode       - select one to set initial color mode.
 *              OLED_SetMode_Color_Normal / OLED_SetMode_Color_Invert
 *          Brightness      - input one value of 0~255 to set initial brightness value.
 * 
 * @return  none
 */
void OLED_Init(OLED_AddressingMode AddressingMode, OLED_SetMode_X_Flip X_FlipMode, OLED_SetMode_Y_Flip Y_FlipMode, OLED_SetMode_Color ColorMode, uint8_t Brightness)
{
    uint8_t i;

    RCC_APB2PeriphClockCmd(OLED_CmdPin_RCC_APB2Periph, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin		= OLED_CmdPin_Pin;
    GPIO_InitStructure.GPIO_Mode	= GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed	= OLED_CmdPin_Speed;
    GPIO_Init(OLED_CmdPin_Port, &GPIO_InitStructure);

    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Show_DISABLE);
    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Roll_DISABLE);

    OLED_Cmd();
    for(i = 0 ; i < sizeof(OLED_InitCmd) ; i++) {SoftSPI_SwapBytes(OLED_InitCmd[i]);  }
    SoftSPI_SwapBytes(OLED_MultiplexRatioCmd);
    SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][0]);
    SoftSPI_SwapBytes(OLED_COMPinsCmd);
    SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][1]);
    SoftSPI_SwapBytes(OLED_AddressingModeCmd);
    SoftSPI_SwapBytes(AddressingMode);
    SoftSPI_SwapBytes(OLED_ListAreaCmd);
    if(OLED_Model == 2)
    {
        SoftSPI_SwapBytes(OLED_042ListAdjust);
        SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][OLED_Model_Index_ListMax] + OLED_042ListAdjust);
    }
    else
    {
        SoftSPI_SwapBytes(0);
        SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][OLED_Model_Index_ListMax]);
    }
    SoftSPI_SwapBytes(OLED_PageAreaCmd);
    SoftSPI_SwapBytes(0);
    SoftSPI_SwapBytes(OLED_Model_Dat[OLED_Model][OLED_Model_Index_PageMax]);
    SoftSPI_Stop();

    OLED_Set(OLED_SetMode_Brightness, Brightness);
    OLED_Set(OLED_SetMode_OtherOptions, X_FlipMode);
    OLED_Set(OLED_SetMode_OtherOptions, Y_FlipMode);
    OLED_Set(OLED_SetMode_OtherOptions, ColorMode);
    OLED_Clear();

    OLED_Set(OLED_SetMode_OtherOptions, OLED_SetMode_Show_ENABLE);
}
