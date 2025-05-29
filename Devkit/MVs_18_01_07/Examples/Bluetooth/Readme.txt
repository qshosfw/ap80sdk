Bluetooth工程主要演示了BT A2DP功能和BT HF功能。

硬件环境要求:
    1. AP80系列开发板。
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 B6/B7/GND 引脚。
    3. 蓝牙芯片为MTK662X系列，蓝牙邮票板为O18_BT_2014.08.12_V1.0，引脚对应关系为BUART_RTS~GP_B31,32K_CLK~~GP_B30,BUART_TXD~~GP_B29,BUART_RXD~GP_B28,PCM_MISO~GP_B27,PCM_MOSI~GP_B26,PCM_CLK~GP_B25,PCM_SYNC~GP_B24,LDO_EN~GP_B23。
	
软件使用说明:
    1. 程序运行起来后，用其他蓝牙设备可以搜索到名称为BT_112358的设备，该设备即为示例程序设备。如果需要修改蓝牙设备名称和地址，可以修改task_bt.c中的BtDevName和BtAddr。搜索到此设备后可以进行配对和连接等操作。
    2. 配对和连接后，另一端如手机可以播放歌曲、切歌、调节音量等操作，此时示例程序BT A2DP功能工作正常。
	3. 配对和连接后，另一端如手机可以接电话、打电话等操作，此时示例程序BT HF功能工作正常。