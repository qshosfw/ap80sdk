#ifndef __LED_EXAMPLE_H__
#define __LED_EXAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#define SEGA_BITNO	0
#define SEGB_BITNO	1
#define SEGC_BITNO	2
#define SEGD_BITNO	3
#define SEGE_BITNO	4
#define SEGF_BITNO	5
#define SEGG_BITNO	6
#define SEGH_BITNO	7

#define	SEG_A	(1 << SEGA_BITNO)
#define	SEG_B	(1 << SEGB_BITNO)
#define	SEG_C	(1 << SEGC_BITNO)
#define	SEG_D	(1 << SEGD_BITNO)
#define	SEG_E	(1 << SEGE_BITNO)
#define	SEG_F	(1 << SEGF_BITNO)
#define	SEG_G	(1 << SEGG_BITNO)
#define	SEG_H	(1 << SEGH_BITNO)

#define	NUM_0	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define	NUM_1	(SEG_B | SEG_C)
#define	NUM_2	(SEG_A | SEG_B | SEG_D | SEG_E | SEG_G)
#define	NUM_3	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_G)
#define	NUM_4	(SEG_B | SEG_C | SEG_F | SEG_G)
#define	NUM_5	(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define	NUM_6	(SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define	NUM_7	(SEG_A | SEG_B | SEG_C)
#define	NUM_8	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define	NUM_9	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)

#define	CHR_A	(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define	CHR_B	(SEG_C | SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_C	(SEG_A | SEG_D | SEG_E | SEG_F)
#define	CHR_D	(SEG_B | SEG_C | SEG_D | SEG_E | SEG_G)

#define	CHR_E	(SEG_A | SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_F	(SEG_A | SEG_E | SEG_F | SEG_G)
#define	CHR_G	(SEG_A | SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_H	(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
//#define	CHR_H	(SEG_C | SEG_E | SEG_F | SEG_G)			//为了与X(显示位H)区别开，H显示位'h'
#define	CHR_I	(SEG_E | SEG_F)
#define	CHR_J	(SEG_B | SEG_C | SEG_D)
#define	CHR_K	(SEG_D | SEG_E | SEG_F | SEG_G)
#define	CHR_L	(SEG_D | SEG_E | SEG_F)
#define	CHR_M	(SEG_A | SEG_B | SEG_E | SEG_F)
#define	CHR_N	(SEG_A | SEG_B | SEG_C | SEG_E | SEG_F)
#define	CHR_O	(SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_P	(SEG_A | SEG_B | SEG_E | SEG_F | SEG_G)
#define	CHR_Q	(SEG_A | SEG_B | SEG_C | SEG_F | SEG_G)
#define	CHR_R	(SEG_E | SEG_F | SEG_G)
#define	CHR_S	(SEG_A | SEG_C | SEG_D | SEG_F | SEG_G)
#define	CHR_T	(SEG_A | SEG_E | SEG_F)
#define	CHR_U	(SEG_B | SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_V	(SEG_C | SEG_D | SEG_E)
#define	CHR_W	(SEG_C | SEG_D | SEG_E | SEG_F)
#define	CHR_X	(SEG_B | SEG_C | SEG_E | SEG_F | SEG_G)
#define	CHR_Y	(SEG_B | SEG_C | SEG_D | SEG_F | SEG_G)
#define	CHR_Z	(SEG_A | SEG_D)
#define	CHR_	(SEG_G)
#define	CHR_NUL 0

//7只脚段码LED屏示例
#define LED_PIN1_PORT_PU 		GPIO_A_PU
#define LED_PIN1_PORT_PD 		GPIO_A_PD
#define LED_PIN1_PORT_OE 		GPIO_A_OE
#define LED_PIN1_PORT_IE 		GPIO_A_IE
#define LED_PIN1_PORT_ICS_1MA7 	GPIO_A_PULLDOWN1
#define LED_PIN1_PORT_ICS_2MA4 	GPIO_A_PULLDOWN2
#define LED_PIN1_PORT_OUT 		GPIO_A_OUT
#define LED_PIN1_BIT			GPIOA25

#define LED_PIN2_PORT_PU 		GPIO_B_PU
#define LED_PIN2_PORT_PD 		GPIO_B_PD
#define LED_PIN2_PORT_OE 		GPIO_B_OE
#define LED_PIN2_PORT_IE 		GPIO_B_IE
#define LED_PIN2_PORT_ICS_1MA7 	GPIO_B_PULLDOWN1
#define LED_PIN2_PORT_ICS_2MA4 	GPIO_B_PULLDOWN2
#define LED_PIN2_PORT_OUT 		GPIO_B_OUT
#define LED_PIN2_BIT			GPIOB2

#define LED_PIN3_PORT_PU 		GPIO_B_PU
#define LED_PIN3_PORT_PD 		GPIO_B_PD
#define LED_PIN3_PORT_OE 		GPIO_B_OE
#define LED_PIN3_PORT_IE 		GPIO_B_IE
#define LED_PIN3_PORT_ICS_1MA7 	GPIO_B_PULLDOWN1
#define LED_PIN3_PORT_ICS_2MA4 	GPIO_B_PULLDOWN2
#define LED_PIN3_PORT_OUT 		GPIO_B_OUT
#define LED_PIN3_BIT			GPIOB3

#define LED_PIN4_PORT_PU 		GPIO_B_PU
#define LED_PIN4_PORT_PD 		GPIO_B_PD
#define LED_PIN4_PORT_OE 		GPIO_B_OE
#define LED_PIN4_PORT_IE 		GPIO_B_IE
#define LED_PIN4_PORT_ICS_1MA7 	GPIO_B_PULLDOWN1
#define LED_PIN4_PORT_ICS_2MA4 	GPIO_B_PULLDOWN2
#define LED_PIN4_PORT_OUT 		GPIO_B_OUT
#define LED_PIN4_BIT			GPIOB4

#define LED_PIN5_PORT_PU 		GPIO_B_PU
#define LED_PIN5_PORT_PD 		GPIO_B_PD
#define LED_PIN5_PORT_OE 		GPIO_B_OE
#define LED_PIN5_PORT_IE 		GPIO_B_IE
#define LED_PIN5_PORT_ICS_1MA7 	GPIO_B_PULLDOWN1
#define LED_PIN5_PORT_ICS_2MA4 	GPIO_B_PULLDOWN2
#define LED_PIN5_PORT_OUT 		GPIO_B_OUT
#define LED_PIN5_BIT			GPIOB5

#define LED_PIN6_PORT_PU 		GPIO_B_PU
#define LED_PIN6_PORT_PD 		GPIO_B_PD
#define LED_PIN6_PORT_OE 		GPIO_B_OE
#define LED_PIN6_PORT_IE 		GPIO_B_IE
#define LED_PIN6_PORT_ICS_1MA7 	GPIO_B_PULLDOWN1
#define LED_PIN6_PORT_ICS_2MA4 	GPIO_B_PULLDOWN2
#define LED_PIN6_PORT_OUT 		GPIO_B_OUT
#define LED_PIN6_BIT			GPIOB6

#define LED_PIN7_PORT_PU 		GPIO_B_PU
#define LED_PIN7_PORT_PD 		GPIO_B_PD
#define LED_PIN7_PORT_OE 		GPIO_B_OE
#define LED_PIN7_PORT_IE 		GPIO_B_IE
#define LED_PIN7_PORT_ICS_1MA7 	GPIO_B_PULLDOWN1
#define LED_PIN7_PORT_ICS_2MA4 	GPIO_B_PULLDOWN2
#define LED_PIN7_PORT_OUT 		GPIO_B_OUT
#define LED_PIN7_BIT			GPIOB7

//设置LED 各个引脚输出高电平
#define LED_PIN1_OUT_HIGH	GpioSetRegOneBit(LED_PIN1_PORT_OE, LED_PIN1_BIT),\
                                                 GpioSetRegOneBit(LED_PIN1_PORT_OUT, LED_PIN1_BIT)

#define LED_PIN2_OUT_HIGH	GpioSetRegOneBit(LED_PIN2_PORT_OE, LED_PIN2_BIT),\
                                                 GpioSetRegOneBit(LED_PIN2_PORT_OUT, LED_PIN2_BIT)

#define LED_PIN3_OUT_HIGH	GpioSetRegOneBit(LED_PIN3_PORT_OE, LED_PIN3_BIT),\
                                                 GpioSetRegOneBit(LED_PIN3_PORT_OUT, LED_PIN3_BIT)

#define LED_PIN4_OUT_HIGH	GpioSetRegOneBit(LED_PIN4_PORT_OE, LED_PIN4_BIT),\
                                                 GpioSetRegOneBit(LED_PIN4_PORT_OUT, LED_PIN4_BIT)

#define LED_PIN5_OUT_HIGH	GpioSetRegOneBit(LED_PIN5_PORT_OE, LED_PIN5_BIT),\
                                                 GpioSetRegOneBit(LED_PIN5_PORT_OUT, LED_PIN5_BIT)

#define LED_PIN6_OUT_HIGH	GpioSetRegOneBit(LED_PIN6_PORT_OE, LED_PIN6_BIT),\
                                                 GpioSetRegOneBit(LED_PIN6_PORT_OUT, LED_PIN6_BIT)

#define LED_PIN7_OUT_HIGH	GpioSetRegOneBit(LED_PIN7_PORT_OE, LED_PIN7_BIT),\
                                                 GpioSetRegOneBit(LED_PIN7_PORT_OUT, LED_PIN7_BIT)


#define LED_PIN1_IN_ON	 GpioSetRegOneBit(LED_PIN1_PORT_ICS_2MA4, LED_PIN1_BIT),\
                                           GpioSetRegOneBit(LED_PIN1_PORT_ICS_1MA7, LED_PIN1_BIT)

#define LED_PIN2_IN_ON	 GpioSetRegOneBit(LED_PIN2_PORT_ICS_2MA4, LED_PIN2_BIT),\
                                           GpioSetRegOneBit(LED_PIN2_PORT_ICS_1MA7, LED_PIN2_BIT)

#define LED_PIN3_IN_ON	 GpioSetRegOneBit(LED_PIN3_PORT_ICS_2MA4, LED_PIN3_BIT),\
                                           GpioSetRegOneBit(LED_PIN3_PORT_ICS_1MA7, LED_PIN3_BIT)

#define LED_PIN4_IN_ON	 GpioSetRegOneBit(LED_PIN4_PORT_ICS_2MA4, LED_PIN4_BIT),\
                                           GpioSetRegOneBit(LED_PIN4_PORT_ICS_1MA7, LED_PIN4_BIT)

#define LED_PIN5_IN_ON	 GpioSetRegOneBit(LED_PIN5_PORT_ICS_2MA4, LED_PIN5_BIT),\
                                           GpioSetRegOneBit(LED_PIN5_PORT_ICS_1MA7, LED_PIN5_BIT)

#define LED_PIN6_IN_ON	GpioSetRegOneBit(LED_PIN6_PORT_ICS_2MA4, LED_PIN6_BIT),\
                                           GpioSetRegOneBit(LED_PIN6_PORT_ICS_1MA7, LED_PIN6_BIT)

#define LED_PIN7_IN_ON	GpioSetRegOneBit(LED_PIN7_PORT_ICS_2MA4, LED_PIN7_BIT),\
                                           GpioSetRegOneBit(LED_PIN7_PORT_ICS_1MA7, LED_PIN7_BIT)

//禁止LED 各个引脚电流输入
#define LED_PIN1_IN_OFF	 GpioClrRegOneBit(LED_PIN1_PORT_ICS_2MA4, LED_PIN1_BIT),\
                                           GpioClrRegOneBit(LED_PIN1_PORT_ICS_1MA7, LED_PIN1_BIT)

#define LED_PIN2_IN_OFF	 GpioClrRegOneBit(LED_PIN2_PORT_ICS_2MA4, LED_PIN2_BIT),\
                                           GpioClrRegOneBit(LED_PIN2_PORT_ICS_1MA7, LED_PIN2_BIT)

#define LED_PIN3_IN_OFF	 GpioClrRegOneBit(LED_PIN3_PORT_ICS_2MA4, LED_PIN3_BIT),\
                                           GpioClrRegOneBit(LED_PIN3_PORT_ICS_1MA7, LED_PIN3_BIT)

#define LED_PIN4_IN_OFF	 GpioClrRegOneBit(LED_PIN4_PORT_ICS_2MA4, LED_PIN4_BIT),\
                                           GpioClrRegOneBit(LED_PIN4_PORT_ICS_1MA7, LED_PIN4_BIT)

#define LED_PIN5_IN_OFF	 GpioClrRegOneBit(LED_PIN5_PORT_ICS_2MA4, LED_PIN5_BIT),\
                                           GpioClrRegOneBit(LED_PIN5_PORT_ICS_1MA7, LED_PIN5_BIT)

#define LED_PIN6_IN_OFF	 GpioClrRegOneBit(LED_PIN6_PORT_ICS_2MA4, LED_PIN6_BIT),\
                                           GpioClrRegOneBit(LED_PIN6_PORT_ICS_1MA7, LED_PIN6_BIT)

#define LED_PIN7_IN_OFF	 GpioClrRegOneBit(LED_PIN7_PORT_ICS_2MA4, LED_PIN7_BIT),\
                                           GpioClrRegOneBit(LED_PIN7_PORT_ICS_1MA7, LED_PIN7_BIT)

void LedPinGpioInit(void);
void LedAllPinGpioInput(void);
void LedFlushDisp(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
