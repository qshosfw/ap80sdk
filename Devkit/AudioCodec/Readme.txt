AudioPath工程主要演示了各种输入输出路径的配置。

硬件环境要求:
    1. AP80系列开发板
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 A1/A0/GND 引脚
    

软件使用说明:

    1. 上电时，PC UART会输出控制信息:
       /==========================================================================\
       |                   AUDIO PATH TESTBENCH                                   |
       | Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
       \==========================================================================/

        ==============================================
         Please send num to sel audio path
         Path 0: Line in to Line out
         Path 1: Line in to I2s out
         Path 2: Line in to Line out and I2s out
         Path 3: I2s in to I2s out
         Path 4: I2s in to Line out
         Path 5: Line in to I2s out and I2s in to Line out
        ==============================================

    2. 根据提示信息输入对应数字则会配置成对应的输入输出路径。
       例如：通过UART输入1，则会配置成line in --> line out路径，此时会从line out处听到line in
       输入的曲目声音。

        