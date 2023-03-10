1) Download the files for this lab to your laptop

2) Copy the following files to your Zybo/Cora board
scp device_regeneration.elf root@192.168.1.10:
scp Challenges.db root@192.168.1.10:

ZYBO:
scp ZYBO_LIBS/libsqlite3.so.0.8.6 root@192.168.1.10:/lib

CORA:
scp CORA_LIBS/libsqlite3.so.0.8.6 root@192.168.1.10:/lib

3) After copying the libsqlite3.so.0.8.6, do the following on your Cora or Zybo boards
cd /lib
ln -s libsqlite3.so.0.8.6 libsqlite3.so.0
ln -s libsqlite3.so.0.8.6 libsqlite3.so
cd

4) YOU MUST PROGRAM THE BOARD BEFORE RUNNING THESE COMMANDS
ZYBO: Program with each of these, one-at-a-time, and run authentication
CORA: Program from Vivado/Hardware Manager

echo SR_RFM_V4_TDC_Macro_P1.bit.bin > /sys/class/fpga_manager/fpga0/firmware
echo SR_RFM_V4_TDC_Macro_P2.bit.bin > /sys/class/fpga_manager/fpga0/firmware
echo SR_RFM_V4_TDC_Macro_P3.bit.bin > /sys/class/fpga_manager/fpga0/firmware
echo SR_RFM_V4_TDC_Macro_P4.bit.bin > /sys/class/fpga_manager/fpga0/firmware

5) Run this on your laptop FIRST -- wait for it to get to 'waiting for connections ...', change the IP as needed
verifier_regeneration Master_TDC SR_RFM_V4_TDC SRFSyn1 192.168.1.20 Master1_OptKEK_TVN_0.00_WID_1.75

6) Run this on your Cora/Zybo, change the IP as needed
./device_regeneration.elf Jim 192.168.1.10 192.168.1.20


TESTING
verifier_regeneration Master_TDC SR_RFM_V4_TDC SRFSyn1 192.168.5.20 Master1_OptKEK_TVN_0.00_WID_1.75
./device_regeneration.elf Jim 192.168.1.10 192.168.5.20
