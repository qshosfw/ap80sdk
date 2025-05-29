LedCharge工程主要演示了充电指示情况。

硬件环境要求:
    AP80系列开发板

软件使用说明:

   1、CHARGE_DONE_DARK为充电完成时指示灯灭的宏定义：开启该宏定义，当充电完成时指示灯灭；关闭该宏定义，当充电完成时指示灯亮。

   2、本例程中提供给GPIOA0的电流源为1.7mA,可根据实际需要设置成：ICS_0MA0，  /**<pulldown current:0.0mA*/
								ICS_1MA7,   /**<pulldown current:1.7mA*/
    								ICS_2MA4,   /**<pulldown current:2.4mA*/
    								ICS_4MA1，  /**<pulldown current:4.1mA*/
   3、充电过程时指示灯闪烁，闪烁频率由LedWidth决定。
   							        