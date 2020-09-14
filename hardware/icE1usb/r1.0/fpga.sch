EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 2 7
Title "icE1usb - FPGA and related digital"
Date "2020-08-26"
Rev "1.0"
Comp ""
Comment1 "CERN-OHL-S"
Comment2 "(C) 2020 Sylvain Munaut"
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L power:GND #PWR014
U 1 1 5F135107
P 2300 7400
F 0 "#PWR014" H 2300 7150 50  0001 C CNN
F 1 "GND" H 2305 7227 50  0000 C CNN
F 2 "" H 2300 7400 50  0001 C CNN
F 3 "" H 2300 7400 50  0001 C CNN
	1    2300 7400
	1    0    0    -1  
$EndComp
$Comp
L s47-chips:ICE40UP5K-SG48 U1
U 1 1 5F1398C4
P 2300 4600
F 0 "U1" H 1500 1800 50  0000 L CNN
F 1 "ICE40UP5K-SG48" H 1500 1900 50  0000 L CNN
F 2 "Package_DFN_QFN:QFN-48-1EP_7x7mm_P0.5mm_EP5.3x5.3mm" H 2300 1550 50  0001 C CNN
F 3 "" H 2600 6100 50  0001 C CNN
	1    2300 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2300 7400 2300 7300
$Comp
L s47-chips:SPI_FLASH_8P U5
U 1 1 5F13E7E1
P 9800 4500
F 0 "U5" H 9800 4967 50  0000 C CNN
F 1 "W25Q80" H 9800 4876 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 9800 4500 50  0001 C CNN
F 3 "" H 9800 4500 50  0001 C CNN
F 4 "W25Q80DVSNIG" H 9800 4500 50  0001 C CNN "MPN"
	1    9800 4500
	1    0    0    -1  
$EndComp
$Comp
L s47-device:LED_RBAG D?
U 1 1 5F27ACE7
P 3700 6900
AR Path="/5F27ACE7" Ref="D?"  Part="1" 
AR Path="/5F1321E5/5F27ACE7" Ref="D1"  Part="1" 
F 0 "D1" H 3700 6500 50  0000 C CNN
F 1 "STATUS" H 3700 6400 50  0000 C CNN
F 2 "s47-device:LED_RGB_2020" H 3700 6850 50  0001 C CNN
F 3 "~" H 3700 6850 50  0001 C CNN
	1    3700 6900
	1    0    0    -1  
$EndComp
$Comp
L s47-misc:LightPipe LP?
U 1 1 5F27BDD2
P 4400 7100
AR Path="/5F27BDD2" Ref="LP?"  Part="1" 
AR Path="/5F1321E5/5F27BDD2" Ref="LP1"  Part="1" 
F 0 "LP1" H 4628 7146 50  0000 L CNN
F 1 "LightPipe" H 4628 7055 50  0000 L CNN
F 2 "s47-misc:LUMEX_LPF-C011303S" H 4400 7100 50  0001 C CNN
F 3 "" H 4400 7100 50  0001 C CNN
	1    4400 7100
	1    0    0    -1  
$EndComp
$Comp
L Switch:SW_Push SW?
U 1 1 5F27D351
P 6800 5800
AR Path="/5F27D351" Ref="SW?"  Part="1" 
AR Path="/5F1321E5/5F27D351" Ref="SW1"  Part="1" 
F 0 "SW1" H 6800 6085 50  0000 C CNN
F 1 "SW_Push" H 6800 5994 50  0000 C CNN
F 2 "s47-misc:SW_HYP_1TS003B" H 6800 6000 50  0001 C CNN
F 3 "~" H 6800 6000 50  0001 C CNN
	1    6800 5800
	-1   0    0    -1  
$EndComp
$Comp
L s47-device:VCXO_6_TUNE_OE U?
U 1 1 5F27DF04
P 9800 1700
AR Path="/5F27DF04" Ref="U?"  Part="1" 
AR Path="/5F1321E5/5F27DF04" Ref="U4"  Part="1" 
F 0 "U4" H 10000 2200 50  0000 L CNN
F 1 "30M72" H 10000 2100 50  0000 L CNN
F 2 "s47-misc:Oscillator_SMD_VCXO-6Pin_7.0x5.0mm_P2.54mm" H 9800 1700 50  0001 C CNN
F 3 "" H 9800 1700 50  0001 C CNN
	1    9800 1700
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR016
U 1 1 5F2E8E75
P 4000 6800
F 0 "#PWR016" H 4000 6650 50  0001 C CNN
F 1 "+3V3" H 4015 6973 50  0000 C CNN
F 2 "" H 4000 6800 50  0001 C CNN
F 3 "" H 4000 6800 50  0001 C CNN
	1    4000 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 6800 4000 6900
Wire Wire Line
	4000 6900 3900 6900
Wire Wire Line
	3400 6800 3400 6900
Wire Wire Line
	3400 6900 3500 6900
Wire Wire Line
	3200 6900 3300 6900
Wire Wire Line
	3300 6900 3300 7100
Wire Wire Line
	3300 7100 3500 7100
$Comp
L power:+3V3 #PWR09
U 1 1 5F2EE21D
P 1000 900
F 0 "#PWR09" H 1000 750 50  0001 C CNN
F 1 "+3V3" H 1015 1073 50  0000 C CNN
F 2 "" H 1000 900 50  0001 C CNN
F 3 "" H 1000 900 50  0001 C CNN
	1    1000 900 
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 6700 3500 6700
Wire Wire Line
	3200 6800 3400 6800
$Comp
L power:GND #PWR010
U 1 1 5F2F4516
P 1300 2700
F 0 "#PWR010" H 1300 2450 50  0001 C CNN
F 1 "GND" H 1305 2527 50  0000 C CNN
F 2 "" H 1300 2700 50  0001 C CNN
F 3 "" H 1300 2700 50  0001 C CNN
	1    1300 2700
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 2600 1300 2700
$Comp
L Device:C_Small C3
U 1 1 5F2F4A0A
P 1300 3500
F 0 "C3" H 1392 3546 50  0000 L CNN
F 1 "100n" H 1392 3455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1300 3500 50  0001 C CNN
F 3 "~" H 1300 3500 50  0001 C CNN
	1    1300 3500
	-1   0    0    -1  
$EndComp
$Comp
L Device:C_Small C4
U 1 1 5F2F4D71
P 1300 5400
F 0 "C4" H 1392 5446 50  0000 L CNN
F 1 "100n" H 1392 5355 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1300 5400 50  0001 C CNN
F 3 "~" H 1300 5400 50  0001 C CNN
	1    1300 5400
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5F2F503E
P 1300 3700
F 0 "#PWR011" H 1300 3450 50  0001 C CNN
F 1 "GND" H 1305 3527 50  0000 C CNN
F 2 "" H 1300 3700 50  0001 C CNN
F 3 "" H 1300 3700 50  0001 C CNN
	1    1300 3700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR012
U 1 1 5F2F521F
P 1300 5600
F 0 "#PWR012" H 1300 5350 50  0001 C CNN
F 1 "GND" H 1305 5427 50  0000 C CNN
F 2 "" H 1300 5600 50  0001 C CNN
F 3 "" H 1300 5600 50  0001 C CNN
	1    1300 5600
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 5500 1300 5600
Wire Wire Line
	1400 5200 1300 5200
Wire Wire Line
	1300 5200 1300 5300
Wire Wire Line
	1400 3300 1300 3300
Wire Wire Line
	1300 3300 1300 3400
Wire Wire Line
	1300 3600 1300 3700
Wire Wire Line
	1000 5200 1300 5200
Connection ~ 1300 5200
Wire Wire Line
	1000 3300 1300 3300
Connection ~ 1000 3300
Wire Wire Line
	1000 3300 1000 5200
Connection ~ 1300 3300
Wire Wire Line
	1400 2300 1300 2300
Connection ~ 1000 2300
Wire Wire Line
	1000 2300 1000 3300
Wire Wire Line
	1300 2300 1300 2400
Connection ~ 1300 2300
Wire Wire Line
	1300 2300 1000 2300
Wire Wire Line
	2000 1900 2000 1800
Wire Wire Line
	2000 1800 1000 1800
Connection ~ 1000 1800
Wire Wire Line
	1000 1800 1000 2300
$Comp
L power:+1V2 #PWR013
U 1 1 5F2F821B
P 2200 900
F 0 "#PWR013" H 2200 750 50  0001 C CNN
F 1 "+1V2" H 2215 1073 50  0000 C CNN
F 2 "" H 2200 900 50  0001 C CNN
F 3 "" H 2200 900 50  0001 C CNN
	1    2200 900 
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C5
U 1 1 5F2F8B03
P 3200 1600
F 0 "C5" H 3200 1500 50  0000 L CNN
F 1 "1u" V 3300 1650 50  0000 L BNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3200 1600 50  0001 C CNN
F 3 "~" H 3200 1600 50  0001 C CNN
	1    3200 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C6
U 1 1 5F2F9078
P 3400 1600
F 0 "C6" H 3400 1500 50  0000 L CNN
F 1 "100n" V 3500 1650 50  0000 L BNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3400 1600 50  0001 C CNN
F 3 "~" H 3400 1600 50  0001 C CNN
	1    3400 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C7
U 1 1 5F2F926F
P 3600 1600
F 0 "C7" H 3600 1500 50  0000 L CNN
F 1 "100n" V 3700 1650 50  0000 L BNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3600 1600 50  0001 C CNN
F 3 "~" H 3600 1600 50  0001 C CNN
	1    3600 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C8
U 1 1 5F2F95B4
P 3800 1600
F 0 "C8" H 3800 1500 50  0000 L CNN
F 1 "100n" V 3900 1650 50  0000 L BNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3800 1600 50  0001 C CNN
F 3 "~" H 3800 1600 50  0001 C CNN
	1    3800 1600
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R4
U 1 1 5F2F9A9C
P 2200 1200
F 0 "R4" H 2142 1154 50  0000 R CNN
F 1 "120R" H 2142 1245 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 2200 1200 50  0001 C CNN
F 3 "~" H 2200 1200 50  0001 C CNN
	1    2200 1200
	1    0    0    1   
$EndComp
$Comp
L Device:C_Small C2
U 1 1 5F2F0378
P 1300 2500
F 0 "C2" H 1392 2546 50  0000 L CNN
F 1 "100n" H 1392 2455 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 1300 2500 50  0001 C CNN
F 3 "~" H 1300 2500 50  0001 C CNN
	1    1300 2500
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR015
U 1 1 5F30269A
P 3800 1900
F 0 "#PWR015" H 3800 1650 50  0001 C CNN
F 1 "GND" H 3805 1727 50  0000 C CNN
F 2 "" H 3800 1900 50  0001 C CNN
F 3 "" H 3800 1900 50  0001 C CNN
	1    3800 1900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3200 1700 3200 1800
Wire Wire Line
	3200 1800 3400 1800
Wire Wire Line
	3800 1800 3800 1900
Wire Wire Line
	3800 1700 3800 1800
Connection ~ 3800 1800
Wire Wire Line
	3600 1700 3600 1800
Connection ~ 3600 1800
Wire Wire Line
	3600 1800 3800 1800
Wire Wire Line
	3400 1700 3400 1800
Connection ~ 3400 1800
Wire Wire Line
	3400 1800 3600 1800
Wire Wire Line
	3400 1500 3400 1400
Wire Wire Line
	3400 1400 3200 1400
Wire Wire Line
	3200 1400 3200 1500
Wire Wire Line
	2200 1300 2200 1400
Wire Wire Line
	3200 1400 2200 1400
Connection ~ 3200 1400
Connection ~ 2200 1400
Wire Wire Line
	2200 1400 2200 1900
Wire Wire Line
	2200 900  2200 1000
Wire Wire Line
	2200 1000 2400 1000
Wire Wire Line
	2400 1000 2400 1300
Connection ~ 2200 1000
Wire Wire Line
	2200 1000 2200 1100
Wire Wire Line
	2400 1000 2600 1000
Wire Wire Line
	2600 1000 2600 1200
Connection ~ 2400 1000
Wire Wire Line
	3600 1500 3600 1300
Wire Wire Line
	3600 1300 2400 1300
Connection ~ 2400 1300
Wire Wire Line
	2400 1300 2400 1900
Wire Wire Line
	3800 1500 3800 1200
Wire Wire Line
	3800 1200 2600 1200
Connection ~ 2600 1200
Wire Wire Line
	2600 1200 2600 1900
Wire Wire Line
	1000 900  1000 1800
$Comp
L power:GND #PWR031
U 1 1 5F319485
P 9800 2200
F 0 "#PWR031" H 9800 1950 50  0001 C CNN
F 1 "GND" H 9805 2027 50  0000 C CNN
F 2 "" H 9800 2200 50  0001 C CNN
F 3 "" H 9800 2200 50  0001 C CNN
	1    9800 2200
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR030
U 1 1 5F31974E
P 9800 1100
F 0 "#PWR030" H 9800 950 50  0001 C CNN
F 1 "+3V3" H 9815 1273 50  0000 C CNN
F 2 "" H 9800 1100 50  0001 C CNN
F 3 "" H 9800 1100 50  0001 C CNN
	1    9800 1100
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR034
U 1 1 5F31A810
P 10300 3400
F 0 "#PWR034" H 10300 3250 50  0001 C CNN
F 1 "+3V3" H 10315 3573 50  0000 C CNN
F 2 "" H 10300 3400 50  0001 C CNN
F 3 "" H 10300 3400 50  0001 C CNN
	1    10300 3400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR035
U 1 1 5F31A9CA
P 10300 4900
F 0 "#PWR035" H 10300 4650 50  0001 C CNN
F 1 "GND" H 10305 4727 50  0000 C CNN
F 2 "" H 10300 4900 50  0001 C CNN
F 3 "" H 10300 4900 50  0001 C CNN
	1    10300 4900
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C13
U 1 1 5F31BF37
P 10300 4600
F 0 "C13" H 10392 4646 50  0000 L CNN
F 1 "100n" H 10392 4555 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 10300 4600 50  0001 C CNN
F 3 "~" H 10300 4600 50  0001 C CNN
	1    10300 4600
	1    0    0    -1  
$EndComp
Wire Wire Line
	10200 4300 10300 4300
Connection ~ 10300 4300
Wire Wire Line
	10300 4300 10300 4500
Wire Wire Line
	10200 4800 10300 4800
Wire Wire Line
	10300 4800 10300 4700
Wire Wire Line
	10300 4800 10300 4900
Connection ~ 10300 4800
Wire Wire Line
	9400 4300 9100 4300
Wire Wire Line
	9400 4400 9000 4400
Wire Wire Line
	9400 4700 8900 4700
Wire Wire Line
	9400 4800 8800 4800
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J3
U 1 1 5F328580
P 9700 5800
F 0 "J3" H 9750 5375 50  0000 C CNN
F 1 "SPI" H 9750 5466 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical" H 9700 5800 50  0001 C CNN
F 3 "~" H 9700 5800 50  0001 C CNN
	1    9700 5800
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR033
U 1 1 5F3295EA
P 10000 6200
F 0 "#PWR033" H 10000 5950 50  0001 C CNN
F 1 "GND" H 10005 6027 50  0000 C CNN
F 2 "" H 10000 6200 50  0001 C CNN
F 3 "" H 10000 6200 50  0001 C CNN
	1    10000 6200
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR032
U 1 1 5F3298FE
P 10000 5500
F 0 "#PWR032" H 10000 5350 50  0001 C CNN
F 1 "+3V3" H 10015 5673 50  0000 C CNN
F 2 "" H 10000 5500 50  0001 C CNN
F 3 "" H 10000 5500 50  0001 C CNN
	1    10000 5500
	1    0    0    -1  
$EndComp
Wire Wire Line
	10000 5500 10000 5600
Wire Wire Line
	10000 5600 9900 5600
Wire Wire Line
	9900 5900 10000 5900
Wire Wire Line
	10000 5900 10000 6200
$Comp
L s47-chips:IP4220CZ6 U?
U 1 1 5F348D74
P 5600 1600
AR Path="/5F275ED4/5F348D74" Ref="U?"  Part="1" 
AR Path="/5F1321E5/5F348D74" Ref="U2"  Part="1" 
F 0 "U2" H 5830 1646 50  0000 L CNN
F 1 "IP4220CZ6" H 5830 1555 50  0000 L CNN
F 2 "Package_SO:TSOP-6_1.65x3.05mm_P0.95mm" H 5650 1675 50  0001 C CNN
F 3 "https://assets.nexperia.com/documents/data-sheet/IP4220CZ6.pdf" H 5650 1675 50  0001 C CNN
	1    5600 1600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR028
U 1 1 5F369B3A
P 9300 2200
F 0 "#PWR028" H 9300 1950 50  0001 C CNN
F 1 "GND" H 9305 2027 50  0000 C CNN
F 2 "" H 9300 2200 50  0001 C CNN
F 3 "" H 9300 2200 50  0001 C CNN
	1    9300 2200
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C12
U 1 1 5F36A640
P 9300 2000
F 0 "C12" H 9208 2046 50  0000 R CNN
F 1 "100n" H 9208 1955 50  0000 R CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 9300 2000 50  0001 C CNN
F 3 "~" H 9300 2000 50  0001 C CNN
	1    9300 2000
	1    0    0    -1  
$EndComp
Wire Wire Line
	9800 1100 9800 1200
Wire Wire Line
	9800 1200 9300 1200
Wire Wire Line
	9300 1200 9300 1600
Connection ~ 9800 1200
Wire Wire Line
	9800 1200 9800 1300
Wire Wire Line
	9400 1600 9300 1600
Connection ~ 9300 1600
Wire Wire Line
	9300 1600 9300 1900
Wire Wire Line
	9300 2100 9300 2200
Wire Wire Line
	9800 2100 9800 2200
Wire Wire Line
	10900 1700 10500 1700
Text Label 10900 1700 2    50   ~ 0
clk_30m72
$Comp
L Device:C_Small C10
U 1 1 5F37B51C
P 8500 2000
F 0 "C10" H 8408 2046 50  0000 R CNN
F 1 "1u" H 8408 1955 50  0000 R CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8500 2000 50  0001 C CNN
F 3 "~" H 8500 2000 50  0001 C CNN
	1    8500 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C11
U 1 1 5F37BA68
P 8900 2000
F 0 "C11" H 8809 2046 50  0000 R CNN
F 1 "1u" H 8809 1955 50  0000 R CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 8900 2000 50  0001 C CNN
F 3 "~" H 8900 2000 50  0001 C CNN
	1    8900 2000
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R11
U 1 1 5F37C202
P 8700 1800
F 0 "R11" V 8504 1800 50  0000 C CNN
F 1 "1k5" V 8595 1800 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 8700 1800 50  0001 C CNN
F 3 "~" H 8700 1800 50  0001 C CNN
	1    8700 1800
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R9
U 1 1 5F37C857
P 8200 1600
F 0 "R9" V 8004 1600 50  0000 C CNN
F 1 "91k" V 8095 1600 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 8200 1600 50  0001 C CNN
F 3 "~" H 8200 1600 50  0001 C CNN
	1    8200 1600
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R10
U 1 1 5F37C982
P 8200 1800
F 0 "R10" V 8300 1700 50  0000 C CNN
F 1 "1k5" V 8400 1700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 8200 1800 50  0001 C CNN
F 3 "~" H 8200 1800 50  0001 C CNN
	1    8200 1800
	0    1    1    0   
$EndComp
Text Label 7600 1600 0    50   ~ 0
clk_tune_lo
Text Label 7600 1800 0    50   ~ 0
clk_tune_hi
Wire Wire Line
	9400 1800 8900 1800
Wire Wire Line
	8900 1800 8900 1900
Connection ~ 8900 1800
Wire Wire Line
	8900 1800 8800 1800
Wire Wire Line
	8600 1800 8500 1800
Wire Wire Line
	8500 1800 8500 1900
Connection ~ 8500 1800
Wire Wire Line
	8500 1800 8300 1800
$Comp
L power:GND #PWR027
U 1 1 5F385C23
P 8900 2200
F 0 "#PWR027" H 8900 1950 50  0001 C CNN
F 1 "GND" H 8905 2027 50  0000 C CNN
F 2 "" H 8900 2200 50  0001 C CNN
F 3 "" H 8900 2200 50  0001 C CNN
	1    8900 2200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR025
U 1 1 5F385DAD
P 8500 2200
F 0 "#PWR025" H 8500 1950 50  0001 C CNN
F 1 "GND" H 8505 2027 50  0000 C CNN
F 2 "" H 8500 2200 50  0001 C CNN
F 3 "" H 8500 2200 50  0001 C CNN
	1    8500 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	8500 2100 8500 2200
Wire Wire Line
	8900 2100 8900 2200
Wire Wire Line
	8300 1600 8500 1600
Wire Wire Line
	8500 1600 8500 1800
Wire Wire Line
	8100 1600 7600 1600
Wire Wire Line
	7600 1800 8100 1800
Text HLabel 3800 6300 2    50   Output ~ 0
e1A_tx_hi
Text HLabel 3800 6200 2    50   Output ~ 0
e1A_tx_lo
Text HLabel 3800 5900 2    50   Output ~ 0
e1B_tx_hi
Text HLabel 3800 6000 2    50   Output ~ 0
e1B_tx_lo
Text HLabel 3800 6500 2    50   Input ~ 0
e1A_rx_hi_p
Text HLabel 3800 6400 2    50   Input ~ 0
e1A_rx_hi_n
Text HLabel 3800 5800 2    50   Input ~ 0
e1A_rx_lo_p
Text HLabel 3800 5700 2    50   Input ~ 0
e1A_rx_lo_n
Text HLabel 3800 5300 2    50   Input ~ 0
e1B_rx_hi_p
Text HLabel 3800 5200 2    50   Input ~ 0
e1B_rx_hi_n
Text HLabel 3800 5500 2    50   Input ~ 0
e1B_rx_lo_p
Text HLabel 3800 5400 2    50   Input ~ 0
e1B_rx_lo_n
Text HLabel 3800 5600 2    50   Output ~ 0
e1_rx_bias0
Text HLabel 3800 6100 2    50   Output ~ 0
e1_rx_bias1
Wire Wire Line
	3200 4600 3800 4600
Wire Wire Line
	3200 4700 3800 4700
Wire Wire Line
	3200 4800 3800 4800
Wire Wire Line
	3200 4900 3800 4900
Text Label 3800 4600 2    50   ~ 0
flash_mosi
Text Label 3800 4700 2    50   ~ 0
flash_miso
Text Label 3800 4800 2    50   ~ 0
flash_sck
Text Label 3800 4900 2    50   ~ 0
~flash_cs
$Comp
L Device:R_Pack04 RN4
U 1 1 5F3C8D1C
P 8700 3800
F 0 "RN4" H 8888 3846 50  0000 L CNN
F 1 "10k" H 8888 3755 50  0000 L CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" V 8975 3800 50  0001 C CNN
F 3 "~" H 8700 3800 50  0001 C CNN
	1    8700 3800
	1    0    0    -1  
$EndComp
Text Label 3800 2500 2    50   ~ 0
clk_30m72
$Comp
L Device:R_Small R12
U 1 1 5F3CCC5E
P 10400 1700
F 0 "R12" V 10204 1700 50  0000 C CNN
F 1 "27R" V 10295 1700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 10400 1700 50  0001 C CNN
F 3 "~" H 10400 1700 50  0001 C CNN
	1    10400 1700
	0    1    1    0   
$EndComp
Wire Wire Line
	10300 1700 10200 1700
Text Label 7900 4300 0    50   ~ 0
flash_mosi
Text Label 7900 4400 0    50   ~ 0
flash_miso
Text Label 7900 4700 0    50   ~ 0
flash_sck
$Comp
L power:+3V3 #PWR029
U 1 1 5F3F147C
P 9300 3400
F 0 "#PWR029" H 9300 3250 50  0001 C CNN
F 1 "+3V3" H 9315 3573 50  0000 C CNN
F 2 "" H 9300 3400 50  0001 C CNN
F 3 "" H 9300 3400 50  0001 C CNN
	1    9300 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	9300 4600 9400 4600
Wire Wire Line
	9400 4500 9300 4500
Connection ~ 9300 4500
Wire Wire Line
	9300 4500 9300 4600
Text Label 7900 4800 0    50   ~ 0
~flash_cs
Wire Wire Line
	9400 5600 9100 5600
Wire Wire Line
	9100 5600 9100 4300
Wire Wire Line
	9100 4300 7900 4300
Wire Wire Line
	9400 5700 9000 5700
Wire Wire Line
	9000 5700 9000 4400
Connection ~ 9000 4400
Wire Wire Line
	9000 4400 7900 4400
Wire Wire Line
	9400 5800 8900 5800
Wire Wire Line
	8900 5800 8900 4700
Connection ~ 8900 4700
Wire Wire Line
	8900 4700 7900 4700
Wire Wire Line
	9400 5900 8800 5900
Wire Wire Line
	8800 5900 8800 4800
Connection ~ 8800 4800
Wire Wire Line
	8800 4800 7900 4800
Wire Wire Line
	8800 4000 8800 4800
Wire Wire Line
	8500 3600 8500 3500
Wire Wire Line
	8500 3500 8600 3500
Wire Wire Line
	8800 3500 8800 3600
Wire Wire Line
	8700 3500 8700 3600
Connection ~ 8700 3500
Wire Wire Line
	8700 3500 8800 3500
Wire Wire Line
	8600 3500 8600 3600
Connection ~ 8600 3500
Wire Wire Line
	8600 3500 8700 3500
$Comp
L power:+3V3 #PWR026
U 1 1 5F4108CD
P 8800 3400
F 0 "#PWR026" H 8800 3250 50  0001 C CNN
F 1 "+3V3" H 8815 3573 50  0000 C CNN
F 2 "" H 8800 3400 50  0001 C CNN
F 3 "" H 8800 3400 50  0001 C CNN
	1    8800 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	8800 3400 8800 3500
Connection ~ 8800 3500
Wire Wire Line
	10300 3400 10300 4300
Wire Wire Line
	9300 3400 9300 4500
Wire Wire Line
	7900 5100 8600 5100
Wire Wire Line
	8600 5100 8600 6000
Wire Wire Line
	8600 6000 10100 6000
Wire Wire Line
	10100 6000 10100 5800
Wire Wire Line
	10100 5800 9900 5800
Wire Wire Line
	7900 5200 8500 5200
Wire Wire Line
	8500 5200 8500 6100
Wire Wire Line
	8500 6100 10200 6100
Wire Wire Line
	10200 6100 10200 5700
Wire Wire Line
	10200 5700 9900 5700
Text Label 7900 5100 0    50   ~ 0
cdone
Text Label 7900 5200 0    50   ~ 0
~creset
Wire Wire Line
	3200 3300 3800 3300
Wire Wire Line
	3200 3400 3800 3400
Text Label 3800 3300 2    50   ~ 0
cdone
Text Label 3800 3400 2    50   ~ 0
~creset
$Comp
L Device:LED_Small D2
U 1 1 5F442B7A
P 10400 6000
F 0 "D2" V 10446 5932 50  0000 R CNN
F 1 "DONE" V 10355 5932 50  0000 R CNN
F 2 "LED_SMD:LED_0603_1608Metric" V 10400 6000 50  0001 C CNN
F 3 "~" V 10400 6000 50  0001 C CNN
	1    10400 6000
	0    -1   -1   0   
$EndComp
Wire Wire Line
	8500 4000 8500 5200
Connection ~ 8500 5200
Wire Wire Line
	8600 4000 8600 4100
Connection ~ 8600 5100
Connection ~ 9100 4300
Wire Wire Line
	8700 4000 8700 4100
Wire Wire Line
	8700 4100 8600 4100
Connection ~ 8600 4100
Wire Wire Line
	8600 4100 8600 5100
$Comp
L power:GND #PWR036
U 1 1 5F4ADE15
P 10400 6200
F 0 "#PWR036" H 10400 5950 50  0001 C CNN
F 1 "GND" H 10405 6027 50  0000 C CNN
F 2 "" H 10400 6200 50  0001 C CNN
F 3 "" H 10400 6200 50  0001 C CNN
	1    10400 6200
	1    0    0    -1  
$EndComp
Wire Wire Line
	10400 5900 10400 5800
Wire Wire Line
	10400 5800 10100 5800
Connection ~ 10100 5800
Wire Wire Line
	10400 6100 10400 6200
$Comp
L power:GND #PWR024
U 1 1 5F4E85E7
P 7100 5900
F 0 "#PWR024" H 7100 5650 50  0001 C CNN
F 1 "GND" H 7105 5727 50  0000 C CNN
F 2 "" H 7100 5900 50  0001 C CNN
F 3 "" H 7100 5900 50  0001 C CNN
	1    7100 5900
	1    0    0    -1  
$EndComp
Wire Wire Line
	7000 5800 7100 5800
Wire Wire Line
	7100 5800 7100 5900
Text Label 6100 5800 0    50   ~ 0
~boot_sw
$Comp
L power:GND #PWR018
U 1 1 5F516168
P 5600 2100
F 0 "#PWR018" H 5600 1850 50  0001 C CNN
F 1 "GND" H 5605 1927 50  0000 C CNN
F 2 "" H 5600 2100 50  0001 C CNN
F 3 "" H 5600 2100 50  0001 C CNN
	1    5600 2100
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 1800 5600 2100
$Comp
L power:+5V #PWR017
U 1 1 5F51C59B
P 5600 1000
F 0 "#PWR017" H 5600 850 50  0001 C CNN
F 1 "+5V" H 5615 1173 50  0000 C CNN
F 2 "" H 5600 1000 50  0001 C CNN
F 3 "" H 5600 1000 50  0001 C CNN
	1    5600 1000
	1    0    0    -1  
$EndComp
Wire Wire Line
	5600 1000 5600 1400
$Comp
L Device:R_Pack04 RN3
U 1 1 5F522437
P 6600 1600
F 0 "RN3" V 6183 1600 50  0000 C CNN
F 1 "33R" V 6274 1600 50  0000 C CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" V 6875 1600 50  0001 C CNN
F 3 "~" H 6600 1600 50  0001 C CNN
	1    6600 1600
	0    -1   -1   0   
$EndComp
$Comp
L Device:R_Small R5
U 1 1 5F5242F9
P 5100 1000
F 0 "R5" V 4904 1000 50  0000 C CNN
F 1 "1k5" V 4995 1000 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 5100 1000 50  0001 C CNN
F 3 "~" H 5100 1000 50  0001 C CNN
	1    5100 1000
	0    1    1    0   
$EndComp
Text HLabel 7000 1700 2    50   Output ~ 0
dbg_tx
Text HLabel 7000 1800 2    50   Input ~ 0
dbg_rx
Text HLabel 7000 1500 2    50   BiDi ~ 0
usb_dp
Text HLabel 7000 1600 2    50   BiDi ~ 0
usb_dn
Text HLabel 7300 3500 2    50   Output ~ 0
e1_led[0..7]
Text HLabel 3800 2800 2    50   Input ~ 0
gps_pps
Text HLabel 3800 3000 2    50   BiDi ~ 0
i2c_sda
Text HLabel 3800 2900 2    50   BiDi ~ 0
i2c_scl
Text HLabel 3800 2400 2    50   Input ~ 0
gps_rx
Text HLabel 3800 2300 2    50   Output ~ 0
gps_tx
Text HLabel 3800 3600 2    50   3State ~ 0
~gps_reset
Wire Wire Line
	3800 2500 3200 2500
Wire Wire Line
	3200 5300 3800 5300
Wire Wire Line
	3800 5200 3200 5200
Wire Wire Line
	3800 5400 3200 5400
Wire Wire Line
	3800 5500 3200 5500
Wire Wire Line
	3800 5700 3200 5700
Wire Wire Line
	3800 5800 3200 5800
Wire Wire Line
	3800 6400 3200 6400
Wire Wire Line
	3800 6500 3200 6500
Text HLabel 4400 4600 2    50   BiDi ~ 0
gpio[0..2]
Text Label 3800 4200 2    50   ~ 0
gpio0
Text Label 3800 4300 2    50   ~ 0
gpio1
Text Label 3800 4400 2    50   ~ 0
gpio2
Text Label 7000 3700 2    50   ~ 0
e1_led0
Text Label 7000 3800 2    50   ~ 0
e1_led1
Text Label 7000 3900 2    50   ~ 0
e1_led2
Wire Wire Line
	7000 1500 6800 1500
Wire Wire Line
	7000 1600 6800 1600
Wire Wire Line
	7000 1700 6800 1700
Wire Wire Line
	6800 1800 7000 1800
Wire Wire Line
	6200 1300 6200 1600
Wire Wire Line
	6200 1600 6400 1600
Wire Wire Line
	6400 1500 6300 1500
Wire Wire Line
	6300 1500 6300 1200
Wire Wire Line
	6300 1200 5700 1200
Wire Wire Line
	5500 1400 5500 1300
Connection ~ 5500 1300
Wire Wire Line
	5500 1300 6200 1300
Wire Wire Line
	5700 1400 5700 1200
Connection ~ 5700 1200
Wire Wire Line
	6200 1900 6200 1700
Wire Wire Line
	6200 1700 6400 1700
Wire Wire Line
	6400 1800 6300 1800
Wire Wire Line
	6300 1800 6300 2000
Wire Wire Line
	6300 2000 5700 2000
Wire Wire Line
	5500 1800 5500 1900
Connection ~ 5500 1900
Wire Wire Line
	5500 1900 6200 1900
Wire Wire Line
	5700 1800 5700 2000
Connection ~ 5700 2000
Wire Wire Line
	5200 1000 5300 1000
Wire Wire Line
	5000 1000 4600 1000
Wire Wire Line
	4600 1900 5500 1900
Wire Wire Line
	4600 2000 5700 2000
Text Label 4600 1000 0    50   ~ 0
usb_pu_i
Text Label 4600 1200 0    50   ~ 0
usb_dp_i
Text Label 4600 1300 0    50   ~ 0
usb_dn_i
Text Label 4600 1900 0    50   ~ 0
dbg_tx_i
Text Label 4600 2000 0    50   ~ 0
dbg_rx_i
Text Label 3800 4100 2    50   ~ 0
dbg_rx_i
Text Label 3800 4000 2    50   ~ 0
dbg_tx_i
Text Label 3800 3700 2    50   ~ 0
usb_dn_i
Text Label 3800 3800 2    50   ~ 0
usb_dp_i
Text Label 3800 3900 2    50   ~ 0
usb_pu_i
Wire Wire Line
	3200 5600 3800 5600
Wire Wire Line
	3200 5900 3800 5900
Wire Wire Line
	3200 6000 3800 6000
Wire Wire Line
	3200 6100 3800 6100
Wire Wire Line
	3200 6200 3800 6200
Wire Wire Line
	3200 6300 3800 6300
Wire Wire Line
	3200 3600 3800 3600
Wire Wire Line
	3200 3700 3800 3700
Wire Wire Line
	3200 3800 3800 3800
Wire Wire Line
	3200 3900 3800 3900
Wire Wire Line
	3200 4000 3800 4000
Wire Wire Line
	3200 4100 3800 4100
Wire Wire Line
	3200 4500 3800 4500
Wire Wire Line
	3200 3000 3800 3000
Wire Wire Line
	3200 2900 3800 2900
Wire Wire Line
	3200 2800 3800 2800
Wire Wire Line
	3200 2700 3800 2700
Wire Wire Line
	3200 2600 3800 2600
Wire Wire Line
	3200 2400 3800 2400
Wire Wire Line
	3200 2300 3800 2300
$Comp
L 74xx:74HC595 U3
U 1 1 5F7CC4CA
P 6200 4100
F 0 "U3" H 6500 4800 50  0000 C CNN
F 1 "74HC595" H 6500 4700 50  0000 C CNN
F 2 "Package_SO:TSSOP-16_4.4x5mm_P0.65mm" H 6200 4100 50  0001 C CNN
F 3 "http://www.ti.com/lit/ds/symlink/sn74hc595.pdf" H 6200 4100 50  0001 C CNN
	1    6200 4100
	1    0    0    -1  
$EndComp
Wire Wire Line
	6600 3700 7000 3700
Wire Wire Line
	6600 3800 7000 3800
Wire Wire Line
	6600 3900 7000 3900
Wire Wire Line
	6600 4000 7000 4000
Wire Wire Line
	6600 4100 7000 4100
Wire Wire Line
	6600 4200 7000 4200
Wire Wire Line
	6600 4300 7000 4300
Wire Wire Line
	6600 4400 7000 4400
NoConn ~ 6600 4600
Text Label 7000 4000 2    50   ~ 0
e1_led3
Text Label 7000 4100 2    50   ~ 0
e1_led4
Text Label 7000 4200 2    50   ~ 0
e1_led5
Text Label 7000 4300 2    50   ~ 0
e1_led6
Text Label 7000 4400 2    50   ~ 0
e1_led7
Entry Wire Line
	7000 3800 7100 3700
Entry Wire Line
	7000 4000 7100 3900
Entry Wire Line
	7000 4100 7100 4000
Entry Wire Line
	7000 4200 7100 4100
Entry Wire Line
	7000 4300 7100 4200
Entry Wire Line
	7000 4400 7100 4300
Entry Wire Line
	7000 3900 7100 3800
Entry Wire Line
	7000 3700 7100 3600
Wire Bus Line
	7100 3500 7300 3500
$Comp
L power:GND #PWR023
U 1 1 5F89F281
P 6200 4900
F 0 "#PWR023" H 6200 4650 50  0001 C CNN
F 1 "GND" H 6205 4727 50  0000 C CNN
F 2 "" H 6200 4900 50  0001 C CNN
F 3 "" H 6200 4900 50  0001 C CNN
	1    6200 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 4800 6200 4900
$Comp
L power:+3V3 #PWR022
U 1 1 5F8ACC60
P 6200 2900
F 0 "#PWR022" H 6200 2750 50  0001 C CNN
F 1 "+3V3" H 6215 3073 50  0000 C CNN
F 2 "" H 6200 2900 50  0001 C CNN
F 3 "" H 6200 2900 50  0001 C CNN
	1    6200 2900
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C9
U 1 1 5F90011B
P 6000 3200
F 0 "C9" H 5908 3246 50  0000 R CNN
F 1 "100n" H 5908 3155 50  0000 R CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6000 3200 50  0001 C CNN
F 3 "~" H 6000 3200 50  0001 C CNN
	1    6000 3200
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR021
U 1 1 5F901B54
P 6000 3400
F 0 "#PWR021" H 6000 3150 50  0001 C CNN
F 1 "GND" H 5900 3300 50  0000 C CNN
F 2 "" H 6000 3400 50  0001 C CNN
F 3 "" H 6000 3400 50  0001 C CNN
	1    6000 3400
	1    0    0    -1  
$EndComp
Wire Wire Line
	6200 2900 6200 3000
Wire Wire Line
	6000 3300 6000 3400
Wire Wire Line
	6000 3100 6000 3000
Wire Wire Line
	6000 3000 6200 3000
Connection ~ 6200 3000
Wire Wire Line
	6200 3000 6200 3500
Wire Wire Line
	5800 3700 5300 3700
Wire Wire Line
	5800 3900 5300 3900
Wire Wire Line
	5800 4000 5300 4000
Wire Wire Line
	5800 4200 5300 4200
$Comp
L power:GND #PWR019
U 1 1 5F98FC74
P 5700 4900
F 0 "#PWR019" H 5700 4650 50  0001 C CNN
F 1 "GND" H 5705 4727 50  0000 C CNN
F 2 "" H 5700 4900 50  0001 C CNN
F 3 "" H 5700 4900 50  0001 C CNN
	1    5700 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	5700 4900 5700 4300
Wire Wire Line
	5700 4300 5800 4300
Text Label 5300 4000 0    50   ~ 0
cdone
Text Label 5300 3700 0    50   ~ 0
flash_mosi
Text Notes 10600 6300 0    50   ~ 0
Must be Blue !\nNeed High Vf !
Text Label 5300 3900 0    50   ~ 0
flash_sck
Text Label 5300 4200 0    50   ~ 0
e1_led_rclk
Text Label 3800 4500 2    50   ~ 0
e1_led_rclk
Wire Wire Line
	4600 1200 5300 1200
Wire Wire Line
	4600 1300 5500 1300
Wire Wire Line
	5300 1000 5300 1200
Connection ~ 5300 1200
Wire Wire Line
	5300 1200 5700 1200
$Comp
L Device:R_Small R6
U 1 1 5F39DC9A
P 5800 7100
F 0 "R6" H 5742 7054 50  0000 R CNN
F 1 "5k1" H 5742 7145 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 5800 7100 50  0001 C CNN
F 3 "~" H 5800 7100 50  0001 C CNN
	1    5800 7100
	1    0    0    1   
$EndComp
$Comp
L Device:R_Small R8
U 1 1 5F39E0B2
P 5900 7100
F 0 "R8" H 5959 7054 50  0000 L CNN
F 1 "5k1" H 5959 7145 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 5900 7100 50  0001 C CNN
F 3 "~" H 5900 7100 50  0001 C CNN
	1    5900 7100
	1    0    0    1   
$EndComp
$Comp
L power:+3V3 #PWR020
U 1 1 5F39E1C8
P 5800 6800
F 0 "#PWR020" H 5800 6650 50  0001 C CNN
F 1 "+3V3" H 5815 6973 50  0000 C CNN
F 2 "" H 5800 6800 50  0001 C CNN
F 3 "" H 5800 6800 50  0001 C CNN
	1    5800 6800
	1    0    0    -1  
$EndComp
Wire Wire Line
	5800 6800 5800 6900
Wire Wire Line
	5800 6900 5900 6900
Wire Wire Line
	5900 6900 5900 7000
Connection ~ 5800 6900
Wire Wire Line
	5800 6900 5800 7000
Wire Wire Line
	5400 7300 5800 7300
Wire Wire Line
	5800 7300 5800 7200
Wire Wire Line
	5400 7400 5900 7400
Wire Wire Line
	5900 7400 5900 7200
Text Label 5400 7300 0    50   ~ 0
i2c_sda
Text Label 5400 7400 0    50   ~ 0
i2c_scl
Text Label 3800 3000 2    50   ~ 0
i2c_sda
Text Label 3800 2900 2    50   ~ 0
i2c_scl
$Comp
L power:PWR_FLAG #FLG01
U 1 1 5F39C045
P 1900 1300
F 0 "#FLG01" H 1900 1375 50  0001 C CNN
F 1 "PWR_FLAG" H 1900 1473 50  0001 C CNN
F 2 "" H 1900 1300 50  0001 C CNN
F 3 "~" H 1900 1300 50  0001 C CNN
	1    1900 1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	1900 1300 1900 1400
Wire Wire Line
	1900 1400 2200 1400
Text Label 3800 2700 2    50   ~ 0
clk_tune_lo
Text Label 3800 2600 2    50   ~ 0
clk_tune_hi
Text Label 5300 5800 0    50   ~ 0
e1_led_rclk
$Comp
L Device:R_Small R7
U 1 1 5F4EADE8
P 5900 5800
F 0 "R7" V 5704 5800 50  0000 C CNN
F 1 "1k5" V 5795 5800 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 5900 5800 50  0001 C CNN
F 3 "~" H 5900 5800 50  0001 C CNN
	1    5900 5800
	0    -1   1    0   
$EndComp
Wire Wire Line
	6000 5800 6600 5800
Wire Wire Line
	5800 5800 5300 5800
Entry Wire Line
	4100 4300 4200 4400
Entry Wire Line
	4100 4200 4200 4300
Entry Wire Line
	4100 4400 4200 4500
Wire Bus Line
	4200 4600 4400 4600
Wire Wire Line
	3200 4200 4100 4200
Wire Wire Line
	3200 4300 4100 4300
Wire Wire Line
	3200 4400 4100 4400
Text Notes 7800 1000 0    50   ~ 0
f_c_lo = 13.6 Hz\nf_c_hi = 106 Hz\n\n(Dual pole RC with different first resistor)
Wire Bus Line
	4200 4300 4200 4600
Wire Bus Line
	7100 3500 7100 4300
$EndSCHEMATC
