EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 5 7
Title "icE1usb - E1 interface bias generation"
Date "2020-08-19"
Rev "0.1"
Comp ""
Comment1 "CERN-OHL-S"
Comment2 "(C) 2020 Sylvain Munaut"
Comment3 ""
Comment4 ""
$EndDescr
Text HLabel 2000 3500 0    50   Input ~ 0
bias0
Text HLabel 2000 4500 0    50   Input ~ 0
bias1
Text HLabel 5500 2500 2    50   Output ~ 0
bias_a_p
Text HLabel 5500 3500 2    50   Output ~ 0
bias_a_n
Text HLabel 5500 4500 2    50   Output ~ 0
bias_b_p
Text HLabel 5500 5500 2    50   Output ~ 0
bias_b_n
$Comp
L Device:R_Small R20
U 1 1 5F4D7D0D
P 4700 2500
F 0 "R20" V 4504 2500 50  0000 C CNN
F 1 "0R" V 4595 2500 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 4700 2500 50  0001 C CNN
F 3 "~" H 4700 2500 50  0001 C CNN
	1    4700 2500
	0    1    1    0   
$EndComp
$Comp
L Device:C_Small C27
U 1 1 5F4D81DA
P 3000 3700
F 0 "C27" H 3092 3746 50  0000 L CNN
F 1 "1u" H 3092 3655 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3000 3700 50  0001 C CNN
F 3 "~" H 3000 3700 50  0001 C CNN
	1    3000 3700
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C28
U 1 1 5F4D8409
P 3000 4700
F 0 "C28" H 3092 4746 50  0000 L CNN
F 1 "1u" H 3092 4655 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3000 4700 50  0001 C CNN
F 3 "~" H 3000 4700 50  0001 C CNN
	1    3000 4700
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR057
U 1 1 5F4D8911
P 3000 3900
F 0 "#PWR057" H 3000 3650 50  0001 C CNN
F 1 "GND" H 3005 3727 50  0000 C CNN
F 2 "" H 3000 3900 50  0001 C CNN
F 3 "" H 3000 3900 50  0001 C CNN
	1    3000 3900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR058
U 1 1 5F4D8C18
P 3000 4900
F 0 "#PWR058" H 3000 4650 50  0001 C CNN
F 1 "GND" H 3005 4727 50  0000 C CNN
F 2 "" H 3000 4900 50  0001 C CNN
F 3 "" H 3000 4900 50  0001 C CNN
	1    3000 4900
	1    0    0    -1  
$EndComp
Wire Wire Line
	3000 4800 3000 4900
Wire Wire Line
	2700 4500 3000 4500
Wire Wire Line
	3000 4500 3000 4600
Wire Wire Line
	2500 4500 2000 4500
Wire Wire Line
	2000 3500 2500 3500
Wire Wire Line
	2700 3500 3000 3500
Wire Wire Line
	3000 3500 3000 3600
Wire Wire Line
	3000 3800 3000 3900
$Comp
L Device:C_Small C26
U 1 1 5F4DA17F
P 3000 2700
F 0 "C26" H 3092 2746 50  0000 L CNN
F 1 "1u" H 3092 2655 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 3000 2700 50  0001 C CNN
F 3 "~" H 3000 2700 50  0001 C CNN
	1    3000 2700
	1    0    0    -1  
$EndComp
$Comp
L s47-passive:R_Pack04_Split RN7
U 1 1 5F4DCADF
P 2600 2500
F 0 "RN7" V 2404 2500 50  0000 C CNN
F 1 "10k" V 2495 2500 50  0000 C CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" H 2600 2500 60  0001 C CNN
F 3 "" H 2600 2500 60  0000 C CNN
	1    2600 2500
	0    1    1    0   
$EndComp
$Comp
L s47-passive:R_Pack04_Split RN7
U 2 1 5F4DCF84
P 2800 2700
F 0 "RN7" H 2741 2746 50  0000 R CNN
F 1 "10k" H 2741 2655 50  0000 R CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" H 2800 2700 60  0001 C CNN
F 3 "" H 2800 2700 60  0000 C CNN
	2    2800 2700
	1    0    0    -1  
$EndComp
$Comp
L s47-passive:R_Pack04_Split RN7
U 4 1 5F4DD23C
P 2600 4500
F 0 "RN7" V 2404 4500 50  0000 C CNN
F 1 "10k" V 2495 4500 50  0000 C CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" H 2600 4500 60  0001 C CNN
F 3 "" H 2600 4500 60  0000 C CNN
	4    2600 4500
	0    1    1    0   
$EndComp
$Comp
L s47-passive:R_Pack04_Split RN7
U 3 1 5F4DD9E1
P 2600 3500
F 0 "RN7" V 2404 3500 50  0000 C CNN
F 1 "10k" V 2495 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" H 2600 3500 60  0001 C CNN
F 3 "" H 2600 3500 60  0000 C CNN
	3    2600 3500
	0    1    1    0   
$EndComp
Wire Wire Line
	2700 2500 2800 2500
Wire Wire Line
	3000 2500 3000 2600
Wire Wire Line
	2800 2500 2800 2600
Connection ~ 2800 2500
Wire Wire Line
	2800 2500 3000 2500
$Comp
L power:GND #PWR056
U 1 1 5F4DFC15
P 3000 2900
F 0 "#PWR056" H 3000 2650 50  0001 C CNN
F 1 "GND" H 3005 2727 50  0000 C CNN
F 2 "" H 3000 2900 50  0001 C CNN
F 3 "" H 3000 2900 50  0001 C CNN
	1    3000 2900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR055
U 1 1 5F4DFEE3
P 2800 2900
F 0 "#PWR055" H 2800 2650 50  0001 C CNN
F 1 "GND" H 2805 2727 50  0000 C CNN
F 2 "" H 2800 2900 50  0001 C CNN
F 3 "" H 2800 2900 50  0001 C CNN
	1    2800 2900
	1    0    0    -1  
$EndComp
$Comp
L power:+3V3 #PWR054
U 1 1 5F4E01FD
P 2400 2400
F 0 "#PWR054" H 2400 2250 50  0001 C CNN
F 1 "+3V3" H 2415 2573 50  0000 C CNN
F 2 "" H 2400 2400 50  0001 C CNN
F 3 "" H 2400 2400 50  0001 C CNN
	1    2400 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	2400 2400 2400 2500
Wire Wire Line
	2400 2500 2500 2500
Wire Wire Line
	2800 2800 2800 2900
Wire Wire Line
	3000 2800 3000 2900
$Comp
L Device:R_Small R21
U 1 1 5F4E23E9
P 4700 2700
F 0 "R21" V 4804 2700 50  0000 C CNN
F 1 "0R" V 4895 2700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 4700 2700 50  0001 C CNN
F 3 "~" H 4700 2700 50  0001 C CNN
F 4 "DNP" V 4986 2700 50  0000 C CNN "DNP"
	1    4700 2700
	0    1    1    0   
$EndComp
Text Notes 7600 2700 0    50   ~ 0
A+
Text Notes 8100 2700 0    50   ~ 0
A-
Text Notes 8600 2700 0    50   ~ 0
B+
Text Notes 9100 2700 0    50   ~ 0
B-
Text Notes 6600 3000 0    50   ~ 0
Single Sensitivity\nDual Rail bias adjust
Text Notes 6600 3400 0    50   ~ 0
Dual Sensitivity\nSingle Rail bias adjust
Text Notes 7600 3300 0    50   ~ 0
Fixed
Text Notes 8600 3300 0    50   ~ 0
Fixed
Text Notes 8100 3300 0    50   ~ 0
Bias 0
Text Notes 9100 3300 0    50   ~ 0
Bias 1
Text Notes 9100 2900 0    50   ~ 0
Bias 1
Text Notes 8600 2900 0    50   ~ 0
Bias 0
Text Notes 7600 2900 0    50   ~ 0
Bias 0
Text Notes 8100 2900 0    50   ~ 0
Bias 1
Wire Notes Line
	6500 2800 9500 2800
Wire Notes Line
	7500 2600 7500 3500
Wire Wire Line
	3000 4500 3500 4500
Connection ~ 3000 4500
Wire Wire Line
	3000 3500 3500 3500
Connection ~ 3000 3500
Wire Wire Line
	3000 2500 3500 2500
Connection ~ 3000 2500
Text Label 3500 2500 2    50   ~ 0
bias_fixed
Text Label 3500 3500 2    50   ~ 0
bias_0
Wire Wire Line
	5500 5500 4000 5500
Text Label 4000 5500 0    50   ~ 0
bias_1
Wire Wire Line
	5300 4500 5500 4500
Wire Wire Line
	4800 2700 5000 2700
Wire Wire Line
	5000 2700 5000 2500
Connection ~ 5000 2500
Wire Wire Line
	5000 2500 4800 2500
Wire Wire Line
	4600 2500 4000 2500
Wire Wire Line
	4600 2700 4000 2700
Wire Wire Line
	5000 2500 5300 2500
Wire Wire Line
	5300 2500 5300 4500
Connection ~ 5300 2500
Wire Wire Line
	5300 2500 5500 2500
$Comp
L Device:R_Small R22
U 1 1 5F50BC9B
P 4700 3500
F 0 "R22" V 4504 3500 50  0000 C CNN
F 1 "0R" V 4595 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 4700 3500 50  0001 C CNN
F 3 "~" H 4700 3500 50  0001 C CNN
	1    4700 3500
	0    1    1    0   
$EndComp
Wire Wire Line
	4600 3500 4000 3500
Wire Wire Line
	4600 3700 4000 3700
Wire Wire Line
	4800 3500 5000 3500
Wire Wire Line
	4800 3700 5000 3700
Wire Wire Line
	5000 3700 5000 3500
Connection ~ 5000 3500
Wire Wire Line
	5000 3500 5500 3500
Text Label 4000 2500 0    50   ~ 0
bias_fixed
Text Label 4000 2700 0    50   ~ 0
bias_0
Text Label 4000 3500 0    50   ~ 0
bias_0
Text Label 3500 4500 2    50   ~ 0
bias_1
Text Label 4000 3700 0    50   ~ 0
bias_1
$Comp
L Device:R_Small R23
U 1 1 5F50BF56
P 4700 3700
F 0 "R23" V 4804 3700 50  0000 C CNN
F 1 "0R" V 4895 3700 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 4700 3700 50  0001 C CNN
F 3 "~" H 4700 3700 50  0001 C CNN
F 4 "DNP" V 4986 3700 50  0000 C CNN "DNP"
	1    4700 3700
	0    1    1    0   
$EndComp
Text Notes 2100 5400 0    50   ~ 0
f_c = 16 Hz for single pole RC
$EndSCHEMATC
