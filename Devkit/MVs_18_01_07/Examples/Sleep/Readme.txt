Sleep������Ҫ��ʾ����ν���Sleepģʽ����δ�Sleep���ѡ�

Ӳ������Ҫ��:
    1. AP80ϵ�п�����
    2. ��Ӵ���С�����ֱ��ʹ��AP80ϵ�п������ϵĴ��ڣ�TX/RX/GND ���� B7/B6/GND ����
    

���ʹ��˵��:
    1. �ϵ�ʱ��PC UART�����������Ϣ:
	/==========================================================================\
	|                   Sleep TESTBENCH                                        |
	| Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
	\==========================================================================/

	uart tx:GPIOB7
	uart rx:GPIOB6
	you can send 1 to o18 to sleep the system,the system will wake up when GPIOA10 is low
   

    2. ��ʱ��O18��������1��ϵͳ�����Sleepģʽ����GPIOA10����GPIO�ж�ʱ(�½���)��Sleepģʽ���ѡ�
        