SpiBasic工程主要演示了山景推荐的一套SPI传输协议。

硬件环境要求:
    1. AP80系列开发板两块互联：
	spis端引脚定义：spis_mosi b[20], spis_clk  b[21], spis_miso b[22], spis_cs b[23]
	spim端引脚定义：spim_mosi b[20], spim_clk  b[21], spim_miso b[22], spim_cs b[23]
			即两块开发板按照上述对应关系相连，共地
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 B6/B7/GND 引脚(波特率：1115200,数据位：8,停止位：1,奇偶校验：无)。

软件使用说明:
	此工程包含两个子工程：master、slaver 分别下载
	分别启动两块开发板，可以从串口看到响应命令包，双方各完成一次数据发送和接收。
	（协议详细分析待续…）