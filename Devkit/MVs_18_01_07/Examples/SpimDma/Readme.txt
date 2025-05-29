SpimDma工程主要演示了各种输入输出路径的配置。

硬件环境要求:
    1. AP80系列开发板
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 A1/A0/GND 引脚
    

软件使用说明:
    1、本例程分为中断方式和查询方式两种；
    2、在两种方式中，都是通过芯片本身的master端与slave端进行通讯。测试时需要将master与slave连接
		     其中将master引脚配置成：b[5]-->spim_mosi，b[4]-->spim_clk, b[3]-->spim_miso
                         将slave引脚配置成： b[20]->spis_mosi, b[21]->spis_clk, b[22]->spis_miso
					     a[9]-->slave cs

    3. 上电时，PC UART会输出控制信息:
       /==========================================================================\
       |                   SPIM DMA EXAMPLE TESTBENCH                                  |
       | Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
       \==========================================================================/

         Please send num to sel test item
         1: Spi master dma --> spi slave
         2: Spi slave      --> spi master dma


    4. 根据提示信息输入对应数字则会配置成对应的输入输出路径。
       例如：通过配置1，则会从master端发送SpimSendBuf数据，从slave端接收数据，并最终将slave接收到的数据
       通过串口打印出来。

      