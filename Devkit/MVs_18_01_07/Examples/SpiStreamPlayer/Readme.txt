SpiStreamPlayer工程主要演示了MASTER端通过SPI传输音乐流至SLAVER端，SLAVER端收到流后进行解码播放。

硬件环境要求:
    1. AP80系列开发板两块(一块做MASTER，一块做SLAVER)
    2. 外接串口小板或者直接使用AP80系列开发板上的串口，TX/RX/GND 接至 B6/B7/GND 引脚
    3. MASTER AP80系列开发板与SLAVER AP80系列开发板跳跳线连接关系
       MASTER GP_B20 <---> SLAVER GP_B20
       MASTER GP_B21 <---> SLAVER GP_B21
       MASTER GP_B22 <---> SLAVER GP_B22
       MASTER GP_B23 <---> SLAVER GP_B23
       MASTER GP_B5  <---> SLAVER GP_B5

软件使用说明:
    1. MASTER, 下载代码上电运行后，会将SD卡内的歌曲读出并发送至SLAVER端，MASTER可以接收以下PC UART键盘输入控制：
        [Space] ........... 向SLAVER端发送播放或暂停命令
        [m] ............... 向SLAVER端发送静音/非静音命令
        [e] ............... 向SLAVER端发送结束当前歌曲播放命令
        [h] ............... 向SLAVER端发送播放上一首歌曲命令
        [j] ............... 向SLAVER端发送音量减命令
        [k] ............... 向SLAVER端发送音量加命令
        [l] ............... 向SLAVER端发送播放下一首歌曲命令

    2. SLAVER，下载代码上电运行后，即会通过SPI接收流数据，并解码播放，音频输出端能够听到音乐。