PwmExample工程主要演示了各种输入输出路径的配置。

硬件环境要求:

    1. AP80系列开发板
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 A1/A0/GND 引脚
    

软件使用说明:

    1. 上电时，PC UART会输出控制信息:
    /==========================================================================\
    |                   PWM TESTBENCH                                          |
    | Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
    \==========================================================================/

     ==============================================
     Please send num to sel pwm mode
     1: Pwm mode voltage output
     2: Pwm mode current pulldown1 1.7mA
     3: Pwm mode current pulldown2 2.4mA
     4: Pwm mode current pulldown1+pulldown2 4.1mA

    2. 根据提示信息通过UART输入数字，通过示波器或者驱动Led等来观察效果。

注意事项：

    如果PWM引脚选择的不是直接输出，而是经过下拉电流之后的输出，如果这种情况下您选择通过
    示波器观察输出波形，需要在pwm引脚外部外接上拉电阻。


        