Sleep工程主要演示了如何进入Sleep模式和如何从Sleep唤醒。

硬件环境要求:
    1. AP80系列开发板
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 B7/B6/GND 引脚
    

软件使用说明:
    1. 上电时，PC UART会输出控制信息:
	/==========================================================================\
	|                   Sleep TESTBENCH                                        |
	| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
	\==========================================================================/

	uart tx:GPIOB7
	uart rx:GPIOB6
	you can send 1 to o18 to sleep the system,the system will wake up when GPIOA10 is low
   

    2. 此时向O18发送数字1，系统会进入Sleep模式，当GPIOA10发生GPIO中断时(下降沿)从Sleep模式唤醒。
        