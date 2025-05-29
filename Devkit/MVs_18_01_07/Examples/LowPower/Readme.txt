LowPower例程用于最小系统进入相关运行模式，并可以通过串接电流表测量得到实际的功耗值。
由于本示例使用uart交互的形式进行测试，所以开启了uart clock，这将将使系统额外增加400uA左右的功耗。

硬件环境要求:
    1. AP80系列开发板。
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 B6/B7/GND 引脚(波特率：57600,数据位：8,停止位：1,奇偶校验：无)。

命令行语法：
Test normal run:--------n dpll(rc) clk_division.	e.g. n dpll 8 means free running at dpll clock, divided by 8
Test sleep:-------------s dpll(rc) clk_divison.		e.g. s rc 128 means sleeping at rc clock, divided by 128
Test deep sleep:--------d
Test powerdwon:---------For Powerdown tesging, just use the powerkey!
Help:-------------------h(H/?)

系统启动后运行在DPLL 96MHz的时钟频率上。

1， 进入normal run的测试
可以发送命令如： n rc 2
表示系统将运行在RC时钟源，2分频的时钟频率下
当需要退出该模式时，可以发送exit命令

2. 进入sleep 或deep sleep
通过在命令行上发送相应命令后，如： s rc 128，将使得系统运行在RC时钟源，128分频的系统时钟下，并在该系统时钟下进入sleep。
当从sleep模式唤醒后，系统又默认在DPLL 96MHz的时钟频率上运行。从sleep唤醒后，系统在进入sleep的地方接着往下运行、
而从deep sleep 唤醒后，系统将重头开始运行。

3. 进入powerdown
可以通过使用powerkey来使系统进入powerdown模式。

