SpimDma������Ҫ��ʾ�˸����������·�������á�

Ӳ������Ҫ��:
    1. AP80ϵ�п�����
    2. ��Ӵ���С�����ֱ��ʹ��AP80ϵ�п������ϵĴ��ڣ�TX/RX/GND ���� A1/A0/GND ����
    

���ʹ��˵��:
    1�������̷�Ϊ�жϷ�ʽ�Ͳ�ѯ��ʽ���֣�
    2�������ַ�ʽ�У�����ͨ��оƬ�����master����slave�˽���ͨѶ������ʱ��Ҫ��master��slave����
		     ���н�master�������óɣ�b[5]-->spim_mosi��b[4]-->spim_clk, b[3]-->spim_miso
                         ��slave�������óɣ� b[20]->spis_mosi, b[21]->spis_clk, b[22]->spis_miso
					     a[9]-->slave cs

    3. �ϵ�ʱ��PC UART�����������Ϣ:
       /==========================================================================\
       |                   SPIM DMA EXAMPLE TESTBENCH                                  |
       | Shanghai Mountain View Silicon Technology Co.,Ltd. All Rights Reserved.  |
       \==========================================================================/

         Please send num to sel test item
         1: Spi master dma --> spi slave
         2: Spi slave      --> spi master dma


    4. ������ʾ��Ϣ�����Ӧ����������óɶ�Ӧ���������·����
       ���磺ͨ������1������master�˷���SpimSendBuf���ݣ���slave�˽������ݣ������ս�slave���յ�������
       ͨ�����ڴ�ӡ������

      