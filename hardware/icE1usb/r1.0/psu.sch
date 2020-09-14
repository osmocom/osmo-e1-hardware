EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 6 7
Title "icE1usb - Power Supply"
Date "2020-08-26"
Rev "1.0"
Comp ""
Comment1 "CERN-OHL-S"
Comment2 "(C) 2020 Sylvain Munaut"
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L Regulator_Linear:MIC5504-1.2YM5 U9
U 1 1 5F364B2A
P 4400 3500
F 0 "U9" H 4400 3867 50  0000 C CNN
F 1 "MIC5504-1.2YM5" H 4400 3776 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 4400 3100 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/MIC550X.pdf" H 4150 3750 50  0001 C CNN
	1    4400 3500
	1    0    0    -1  
$EndComp
$Comp
L Regulator_Linear:MIC5504-3.3YM5 U10
U 1 1 5F36521A
P 6600 3500
F 0 "U10" H 6600 3867 50  0000 C CNN
F 1 "MIC5504-3.3YM5" H 6600 3776 50  0000 C CNN
F 2 "Package_TO_SOT_SMD:SOT-23-5" H 6600 3100 50  0001 C CNN
F 3 "http://ww1.microchip.com/downloads/en/DeviceDoc/MIC550X.pdf" H 6350 3750 50  0001 C CNN
	1    6600 3500
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR062
U 1 1 5F36617B
P 4400 4000
F 0 "#PWR062" H 4400 3750 50  0001 C CNN
F 1 "GND" H 4405 3827 50  0000 C CNN
F 2 "" H 4400 4000 50  0001 C CNN
F 3 "" H 4400 4000 50  0001 C CNN
	1    4400 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR067
U 1 1 5F3664EB
P 6600 4000
F 0 "#PWR067" H 6600 3750 50  0001 C CNN
F 1 "GND" H 6605 3827 50  0000 C CNN
F 2 "" H 6600 4000 50  0001 C CNN
F 3 "" H 6600 4000 50  0001 C CNN
	1    6600 4000
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR068
U 1 1 5F366894
P 7200 3200
F 0 "#PWR068" H 7200 3050 50  0001 C CNN
F 1 "+3V3" H 7215 3373 50  0000 C CNN
F 2 "" H 7200 3200 50  0001 C CNN
F 3 "" H 7200 3200 50  0001 C CNN
	1    7200 3200
	1    0    0    -1  
$EndComp
$Comp
L power:+1V2 #PWR063
U 1 1 5F366DAE
P 5000 3200
F 0 "#PWR063" H 5000 3050 50  0001 C CNN
F 1 "+1V2" H 5015 3373 50  0000 C CNN
F 2 "" H 5000 3200 50  0001 C CNN
F 3 "" H 5000 3200 50  0001 C CNN
	1    5000 3200
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR060
U 1 1 5F367087
P 3800 3200
F 0 "#PWR060" H 3800 3050 50  0001 C CNN
F 1 "+5V" H 3815 3373 50  0000 C CNN
F 2 "" H 3800 3200 50  0001 C CNN
F 3 "" H 3800 3200 50  0001 C CNN
	1    3800 3200
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C29
U 1 1 5F3676A9
P 3800 3800
F 0 "C29" H 3892 3846 50  0000 L CNN
F 1 "1u" H 3892 3755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3800 3800 50  0001 C CNN
F 3 "~" H 3800 3800 50  0001 C CNN
	1    3800 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C30
U 1 1 5F367AFB
P 5000 3800
F 0 "C30" H 5092 3846 50  0000 L CNN
F 1 "1u" H 5092 3755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 5000 3800 50  0001 C CNN
F 3 "~" H 5000 3800 50  0001 C CNN
	1    5000 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C31
U 1 1 5F367D86
P 6000 3800
F 0 "C31" H 6092 3846 50  0000 L CNN
F 1 "1u" H 6092 3755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 6000 3800 50  0001 C CNN
F 3 "~" H 6000 3800 50  0001 C CNN
	1    6000 3800
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C32
U 1 1 5F368064
P 7200 3800
F 0 "C32" H 7292 3846 50  0000 L CNN
F 1 "1u" H 7292 3755 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 7200 3800 50  0001 C CNN
F 3 "~" H 7200 3800 50  0001 C CNN
	1    7200 3800
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR061
U 1 1 5F395396
P 3800 4000
F 0 "#PWR061" H 3800 3750 50  0001 C CNN
F 1 "GND" H 3805 3827 50  0000 C CNN
F 2 "" H 3800 4000 50  0001 C CNN
F 3 "" H 3800 4000 50  0001 C CNN
	1    3800 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR064
U 1 1 5F3954C5
P 5000 4000
F 0 "#PWR064" H 5000 3750 50  0001 C CNN
F 1 "GND" H 5005 3827 50  0000 C CNN
F 2 "" H 5000 4000 50  0001 C CNN
F 3 "" H 5000 4000 50  0001 C CNN
	1    5000 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	4000 3400 3800 3400
Wire Wire Line
	3800 3400 3800 3300
Wire Wire Line
	4000 3600 3800 3600
Wire Wire Line
	3800 3600 3800 3400
Connection ~ 3800 3400
Wire Wire Line
	3800 3600 3800 3700
Connection ~ 3800 3600
Wire Wire Line
	3800 3900 3800 4000
Wire Wire Line
	4400 3800 4400 4000
Wire Wire Line
	4800 3400 5000 3400
Wire Wire Line
	5000 3400 5000 3700
Connection ~ 5000 3400
Wire Wire Line
	5000 3900 5000 4000
$Comp
L power:+5V #PWR065
U 1 1 5F397752
P 6000 3200
F 0 "#PWR065" H 6000 3050 50  0001 C CNN
F 1 "+5V" H 6015 3373 50  0000 C CNN
F 2 "" H 6000 3200 50  0001 C CNN
F 3 "" H 6000 3200 50  0001 C CNN
	1    6000 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	6000 3200 6000 3400
Wire Wire Line
	6200 3400 6000 3400
Connection ~ 6000 3400
Wire Wire Line
	6000 3400 6000 3700
Wire Wire Line
	6600 3800 6600 4000
Wire Wire Line
	7000 3400 7200 3400
Connection ~ 7200 3400
Wire Wire Line
	7200 3400 7200 3700
$Comp
L power:GND #PWR066
U 1 1 5F398205
P 6000 4000
F 0 "#PWR066" H 6000 3750 50  0001 C CNN
F 1 "GND" H 6005 3827 50  0000 C CNN
F 2 "" H 6000 4000 50  0001 C CNN
F 3 "" H 6000 4000 50  0001 C CNN
	1    6000 4000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR069
U 1 1 5F398486
P 7200 4000
F 0 "#PWR069" H 7200 3750 50  0001 C CNN
F 1 "GND" H 7205 3827 50  0000 C CNN
F 2 "" H 7200 4000 50  0001 C CNN
F 3 "" H 7200 4000 50  0001 C CNN
	1    7200 4000
	1    0    0    -1  
$EndComp
Wire Wire Line
	7200 3900 7200 4000
Wire Wire Line
	6000 3900 6000 4000
Wire Wire Line
	6200 3600 5500 3600
Wire Wire Line
	5500 3600 5500 3400
Wire Wire Line
	5500 3400 5000 3400
$Comp
L power:PWR_FLAG #FLG04
U 1 1 5F39708F
P 3600 3200
F 0 "#FLG04" H 3600 3275 50  0001 C CNN
F 1 "PWR_FLAG" H 3600 3373 50  0001 C CNN
F 2 "" H 3600 3200 50  0001 C CNN
F 3 "~" H 3600 3200 50  0001 C CNN
	1    3600 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 3200 3600 3300
Wire Wire Line
	3600 3300 3800 3300
Connection ~ 3800 3300
Wire Wire Line
	3800 3300 3800 3200
Wire Wire Line
	5000 3200 5000 3400
Wire Wire Line
	7200 3200 7200 3400
$Comp
L power:GND #PWR059
U 1 1 5F6D7A5F
P 3600 4000
F 0 "#PWR059" H 3600 3750 50  0001 C CNN
F 1 "GND" H 3605 3827 50  0000 C CNN
F 2 "" H 3600 4000 50  0001 C CNN
F 3 "" H 3600 4000 50  0001 C CNN
	1    3600 4000
	1    0    0    -1  
$EndComp
$Comp
L power:PWR_FLAG #FLG05
U 1 1 5F6D82E5
P 3600 3900
F 0 "#FLG05" H 3600 3975 50  0001 C CNN
F 1 "PWR_FLAG" H 3600 4073 50  0001 C CNN
F 2 "" H 3600 3900 50  0001 C CNN
F 3 "~" H 3600 3900 50  0001 C CNN
	1    3600 3900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3600 3900 3600 4000
$EndSCHEMATC
