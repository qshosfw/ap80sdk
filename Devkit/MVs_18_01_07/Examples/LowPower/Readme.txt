LowPower����������Сϵͳ�����������ģʽ��������ͨ�����ӵ���������õ�ʵ�ʵĹ���ֵ��
���ڱ�ʾ��ʹ��uart��������ʽ���в��ԣ����Կ�����uart clock���⽫��ʹϵͳ��������400uA���ҵĹ��ġ�

Ӳ������Ҫ��:
    1. AP80ϵ�п����塣
    2. ��Ӵ���С�����ֱ��ʹ��AP80ϵ�п������ϵĴ��ڣ�TX/RX/GND ���� B6/B7/GND ����(�����ʣ�57600,����λ��8,ֹͣλ��1,��żУ�飺��)��

�������﷨��
Test normal run:--------n dpll(rc) clk_division.	e.g. n dpll 8 means free running at dpll clock, divided by 8
Test sleep:-------------s dpll(rc) clk_divison.		e.g. s rc 128 means sleeping at rc clock, divided by 128
Test deep sleep:--------d
Test powerdwon:---------For Powerdown tesging, just use the powerkey!
Help:-------------------h(H/?)

ϵͳ������������DPLL 96MHz��ʱ��Ƶ���ϡ�

1�� ����normal run�Ĳ���
���Է��������磺 n rc 2
��ʾϵͳ��������RCʱ��Դ��2��Ƶ��ʱ��Ƶ����
����Ҫ�˳���ģʽʱ�����Է���exit����

2. ����sleep ��deep sleep
ͨ�����������Ϸ�����Ӧ������磺 s rc 128����ʹ��ϵͳ������RCʱ��Դ��128��Ƶ��ϵͳʱ���£����ڸ�ϵͳʱ���½���sleep��
����sleepģʽ���Ѻ�ϵͳ��Ĭ����DPLL 96MHz��ʱ��Ƶ�������С���sleep���Ѻ�ϵͳ�ڽ���sleep�ĵط������������С�
����deep sleep ���Ѻ�ϵͳ����ͷ��ʼ���С�

3. ����powerdown
����ͨ��ʹ��powerkey��ʹϵͳ����powerdownģʽ��

