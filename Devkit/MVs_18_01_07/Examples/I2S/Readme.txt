I2sExample工程主要演示了各种输入输出路径的配置。

硬件环境要求:
    1. AP80系列开发板
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 A1/A0/GND 引脚
    3. 外接其他I2S设备
    

软件使用说明:

    1. AP80系列开发板I2s对应引脚配置为： i2s_lrck(io) - b[3], i2s_bclk(io)- b[4], i2s_din(i)- b[6], i2s_do(o) - b[5]
    2. 上电启动之后，默认情况下会打印如下信息：
	/==========================================================================\
    	|                   I2S EXAMPLE TESTBENCH                                  |
   	| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
        \==========================================================================/

    	  Please send num to sel i2s mode: 
    	  1: Line in --> I2s Out:
             Master, Mclk = 12MHZ, SampleRate = 48000, i2s justified format
    
          2: Line in --> I2s Out:
             Slave,  Mclk=12.288/11.2896Mhz, SampleRate = 48000,I2s justified format
    
          3: I2s in --> STA Classd Out:
             Slave,  Mclk=12.288/11.2896Mhz, SampleRate = 48000, I2s justified format
    
          4: I2s  in --> Dac Out
             Master,  Mclk=12.288/11.2896Mhz, SampleRate = 48000, I2s justified format

    3. 通过UART输入对应数字，选择I2S一种配置方式，例如输入 1，则AP80系列被配置成Line in输入到I2S out输出，并且I2S被配置成       
       Master,Mclk=12M, SampleRate=48000, I2S Justified format.

注意事项：

    1、如果AP80系列被选择成选择成I2S in 到I2S out路径，I2s in外接的芯片支持9种采样率，但是I2S out外接的芯片却只支持其中的某几种，
       这样AP80系列的I2S不能够同时输出2种采样率时钟，所以此情况下不支持。
       例如：I2S in外接的芯片支持9种采样率，而I2S out外接ST系列CLASSD功放芯片只能够支持32Khz及以上采样率，所以此种组合下I2S in输入
       的采样率只能够支持32Khz及以上采样率。

    2、如果AP系列为其他输入到I2s out，由于对于I2S来说同一时刻只有一种采样率，所以如果I2S OUT外接的功放芯片不能支持所有采样率时，
       AP80系列可进行重采样转化成功放芯片支持的采样率。