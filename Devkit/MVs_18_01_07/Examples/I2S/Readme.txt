I2sExample������Ҫ��ʾ�˸����������·�������á�

Ӳ������Ҫ��:
    1. AP80ϵ�п�����
    2. ��Ӵ���С�����ֱ��ʹ��AP80ϵ�п������ϵĴ��ڣ�TX/RX/GND ���� A1/A0/GND ����
    3. �������I2S�豸
    

���ʹ��˵��:

    1. AP80ϵ�п�����I2s��Ӧ��������Ϊ�� i2s_lrck(io) - b[3], i2s_bclk(io)- b[4], i2s_din(i)- b[6], i2s_do(o) - b[5]
    2. �ϵ�����֮��Ĭ������»��ӡ������Ϣ��
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

    3. ͨ��UART�����Ӧ���֣�ѡ��I2Sһ�����÷�ʽ���������� 1����AP80ϵ�б����ó�Line in���뵽I2S out���������I2S�����ó�       
       Master,Mclk=12M, SampleRate=48000, I2S Justified format.

ע�����

    1�����AP80ϵ�б�ѡ���ѡ���I2S in ��I2S out·����I2s in��ӵ�оƬ֧��9�ֲ����ʣ�����I2S out��ӵ�оƬȴֻ֧�����е�ĳ���֣�
       ����AP80ϵ�е�I2S���ܹ�ͬʱ���2�ֲ�����ʱ�ӣ����Դ�����²�֧�֡�
       ���磺I2S in��ӵ�оƬ֧��9�ֲ����ʣ���I2S out���STϵ��CLASSD����оƬֻ�ܹ�֧��32Khz�����ϲ����ʣ����Դ��������I2S in����
       �Ĳ�����ֻ�ܹ�֧��32Khz�����ϲ����ʡ�

    2�����APϵ��Ϊ�������뵽I2s out�����ڶ���I2S��˵ͬһʱ��ֻ��һ�ֲ����ʣ��������I2S OUT��ӵĹ���оƬ����֧�����в�����ʱ��
       AP80ϵ�пɽ����ز���ת���ɹ���оƬ֧�ֵĲ����ʡ�