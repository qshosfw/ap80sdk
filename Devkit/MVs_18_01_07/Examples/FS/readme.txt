FS工程主要演示了文件系统功能。

硬件环境要求:
    1. AP80系列开发板。
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 B6/B7/GND 引脚(波特率：1115200,数据位：8,停止位：1,奇偶校验：无)。

软件使用说明:
    1. 本示例支持SD卡(含TF卡、MMC卡)和U盘两种存储设备，可以在程序中进行配置来选择其中一种存储设备(程序默认配置为U盘,HardwareInit(DEV_ID_USB)。程序上电会处在检测设备的循环中，直到指定设备初始化完成才会进入到串口命令交互界面。输入相应的串口指令演示对应功能。
       具体命令如下：
	Command   	Discription
	f   		format
	c   		create one file named test1.txt
	C   		create one folder named TEST
	d   		delete test1.txt
	w   		write 'test string' to test1.txt
	s   		show file list of all disk