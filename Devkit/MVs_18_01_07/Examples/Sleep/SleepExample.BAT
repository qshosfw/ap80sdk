SET PATH=C:\Keil\UV4\ARM\ARMCC\bin\;C:\Strawberry\perl\bin;C:\Actel\Libero_v9.1\Designer\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;c:\altera\80\quartus\bin;C:\Python27;C:\Program Files\doxygen\bin;C:\Program Files\MATLAB\R2010b\runtime\win32;C:\Program Files\MATLAB\R2010b\bin;C:\Program Files\TwoLAME;C:\Program Files\TortoiseSVN\bin;D:\Program Files\Altium Designer Summer 09\System;C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;C:\Strawberry\perl\bin;C:\Actel\Libero_v9.1\Designer\bin;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem;c:\altera\80\quartus\bin;C:\Python27;C:\Program Files\doxygen\bin;C:\Program Files\MATLAB\R2010b\runtime\win32;C:\Program Files\MATLAB\R2010b\bin;C:\Program Files\TwoLAME;C:\Program Files\TortoiseSVN\bin;D:\Program Files\Altium Designer Summer 09\System;C:\Strawberry\c\bin;C:\Strawberry\perl\site\bin;D:\Program Files\WPS Office\9.1.0.5025\office6;D:\Program Files\MyBin;C:\JL\mc\bin;C:\JL\dv10\bin;C:\JL\dv12\bin;C:\JL\bin
SET CPU_TYPE=Cortex-M3
SET CPU_VENDOR=ARM
SET UV2_TARGET=SleepExample
SET CPU_CLOCK=0x00B71B00
"C:\Keil\UV4\ARM\ARMCC\bin\ArmCC" --Via ".\output\startup.__i"
"C:\Keil\UV4\ARM\ARMCC\bin\ArmCC" --Via ".\output\sleepexample.__i"
"C:\Keil\UV4\ARM\ARMCC\bin\ArmCC" --Via ".\output\retarget.__i"
"C:\Keil\UV4\ARM\ARMCC\bin\ArmLink" --Via ".\output\SleepExample.lnp"
fromelf.exe --bin -o "D:\work\new_work\O18\SDK\MVs18_new\MVs18_SDK\Examples\Sleep\output\SleepExample.axf.bin" "D:\work\new_work\O18\SDK\MVs18_new\MVs18_SDK\Examples\Sleep\output\SleepExample.axf"
