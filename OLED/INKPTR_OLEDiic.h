#ifndef INKPTR_OLEDiic_H
#define INKPTR_OLEDiic_H

/*================================================================================================*/
// Part 1: Library, enumerations, and interface function declarations
#include "ch32v00x.h"

typedef enum {OLED_AddressingMode_HorizontalMode = 0, OLED_AddressingMode_VerticalMode, OLED_AddressingMode_PageMode} OLED_AddressingMode;
typedef enum {OLED_SetMode_Brightness = 0, OLED_SetMode_OtherOptions} OLED_SetMode;
typedef enum {OLED_SetMode_X_Flip_Mirror = 0xa1, OLED_SetMode_X_Flip_Normal = 0xa0} OLED_SetMode_X_Flip;
typedef enum {OLED_SetMode_Y_Flip_Mirror = 0xc8, OLED_SetMode_Y_Flip_Normal = 0xc0} OLED_SetMode_Y_Flip;
typedef enum {OLED_SetMode_Color_Invert = 0xa7, OLED_SetMode_Color_Normal = 0xa6} OLED_SetMode_Color;
typedef enum {OLED_SetMode_Roll_DISABLE = 0x2e, OLED_SetMode_Roll_ENABLE = 0x2f} OLED_SetMode_Roll;
typedef enum {OLED_SetMode_Show_DISABLE = 0xae, OLED_SetMode_Show_ENABLE = 0xaf} OLED_SetMode_Show;
typedef enum
{
    OLED_RollMode_Right_1 = 0x00, OLED_RollMode_Right_2 = 0x01, OLED_RollMode_Right_3 = 0x02, OLED_RollMode_Right_4 = 0x03, OLED_RollMode_Right_5 = 0x04, OLED_RollMode_Right_6 = 0x05, OLED_RollMode_Right_7 = 0x06, OLED_RollMode_Right_8 = 0x07,
    OLED_RollMode_Left_1 = 0x80, OLED_RollMode_Left_2 = 0x81, OLED_RollMode_Left_3 = 0x82, OLED_RollMode_Left_4 = 0x83, OLED_RollMode_Left_5 = 0x84, OLED_RollMode_Left_6 = 0x85, OLED_RollMode_Left_7 = 0x86, OLED_RollMode_Left_8 = 0x87
} OLED_RollMode;

void OLED_Set(OLED_SetMode SetMode, uint8_t Dat);
void OLED_Brush(uint8_t Page_Begin, uint8_t Page_End, uint8_t List_Begin, uint8_t List_End, uint8_t Style_Byte);
void OLED_Clear(void);
void OLED_Draw_CmdHead(uint8_t Page_Begin, uint8_t List_Begin);
void OLED_Draw_CmdHead_WholeArea(void);
void OLED_Draw_CmdData(uint8_t Dat);
void OLED_Draw_CmdTail(void);
void OLED_Roll(uint8_t Page_Begin, uint8_t Page_End, uint8_t List_Begin, uint8_t List_End, OLED_RollMode RollMode);
void OLED_Init(OLED_AddressingMode AddressingMode, OLED_SetMode_X_Flip X_FlipMode, OLED_SetMode_Y_Flip Y_FlipMode, OLED_SetMode_Color ColorMode, uint8_t Brightness);


/*================================================================================================*/
// Part 2: Adjustable parameter macro definition
#define OLED_Model 0
// 128*64   > 0
// 128*32   > 1
// 72*40    > 2

#endif
