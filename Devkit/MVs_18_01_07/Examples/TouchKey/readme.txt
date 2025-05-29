TouchKey工程主要演示了AP80系列芯片的TouchKey模块的相关功能，一下是对应的代码及硬件环境简介：
硬件环境要求：
	1、请参照“TouchKey驱动应用文档 .docx”中的相关简介。
	2、本示例中使用了AP80系列芯片中对应的GPIOB[31:26]及GPIOC[2:1]共8组GPIO口作为采样通道。
	3、本示例中使用了GPIOC[0]作为Touchkey的输出信号源。

软件使用说明：
	1、软件中使用了GPIOB[7]作为Uart的输出口。
	2、如想使用TOUCH模块的其他IO作为采样通路，可在“TouchKeyPortInit(TouchKeyPin)”中设置相关的参数。
	