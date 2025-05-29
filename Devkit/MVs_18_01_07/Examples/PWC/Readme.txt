PwcExample工程主要演示了Pwc模块如果来测量输入脉冲宽度。

硬件环境要求:
    1. AP80系列开发板
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 A1/A0/GND 引脚
    

软件使用说明:
    1. 本次输入波形可以由外接提供并接入GPIOA10，也可以将GPIOB6与GPIOA10短接，因为在本
       次示例中提供了从GPIOB6输出Freq=1khz,Duty=50%的PWM信号。

    2. 上电时，PC UART会输出控制信息:
    /==========================================================================\
    |                   PWC TESTBENCH                                          |
    | Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
    \==========================================================================/

     ==============================================
      Please send num to sel pwc capture pulse mode
      1: posedge to posedge
      2: posedge to negedge
      3: negedge to posedge
      4: negedge to negedge
   

    3. 根据提示信息输入对应数字则会配置成相应的抓取模式，例如输入 1，则会抓取输入
       脉冲信号从上升沿到上升沿的脉冲宽度。
        