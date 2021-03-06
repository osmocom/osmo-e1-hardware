EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 7
Title "icE1usb - Top level overview"
Date "2020-08-26"
Rev "1.0"
Comp ""
Comment1 "CERN-OHL-S"
Comment2 "(C) 2020 Sylvain Munaut"
Comment3 ""
Comment4 ""
$EndDescr
$Sheet
S 3000 3000 1500 3000
U 5F1321E5
F0 "fpga" 50
F1 "fpga.sch" 50
F2 "e1A_tx_hi" O R 4500 4000 50 
F3 "e1A_tx_lo" O R 4500 4100 50 
F4 "e1B_tx_hi" O R 4500 5000 50 
F5 "e1B_tx_lo" O R 4500 5100 50 
F6 "e1A_rx_hi_p" I R 4500 4200 50 
F7 "e1A_rx_hi_n" I R 4500 4300 50 
F8 "e1A_rx_lo_p" I R 4500 4400 50 
F9 "e1A_rx_lo_n" I R 4500 4500 50 
F10 "e1B_rx_hi_p" I R 4500 5200 50 
F11 "e1B_rx_hi_n" I R 4500 5300 50 
F12 "e1B_rx_lo_p" I R 4500 5400 50 
F13 "e1B_rx_lo_n" I R 4500 5500 50 
F14 "e1_rx_bias0" O R 4500 4700 50 
F15 "e1_rx_bias1" O R 4500 4800 50 
F16 "dbg_tx" O L 3000 3500 50 
F17 "dbg_rx" I L 3000 3400 50 
F18 "usb_dp" B L 3000 3900 50 
F19 "usb_dn" B L 3000 3800 50 
F20 "gps_pps" I R 4500 3400 50 
F21 "i2c_sda" B R 4500 3700 50 
F22 "i2c_scl" B R 4500 3800 50 
F23 "gps_rx" I R 4500 3200 50 
F24 "gps_tx" O R 4500 3300 50 
F25 "~gps_reset" T R 4500 3500 50 
F26 "gpio[0..2]" B R 4500 5800 50 
F27 "e1_led[0..7]" O R 4500 5700 50 
$EndSheet
$Comp
L Connector:USB_C_Receptacle_USB2.0 X4
U 1 1 5F14D4F7
P 1300 3800
F 0 "X4" H 1407 4667 50  0000 C CNN
F 1 "USB" H 1407 4576 50  0000 C CNN
F 2 "s47-conn:USB_C_Receptacle_HRO_TYPE-C-31-M-12" H 1450 3800 50  0001 C CNN
F 3 "https://www.usb.org/sites/default/files/documents/usb_type-c.zip" H 1450 3800 50  0001 C CNN
F 4 "HRO_TYPE-C-31-M-12" H 1300 3800 50  0001 C CNN "MPN"
	1    1300 3800
	1    0    0    -1  
$EndComp
$Comp
L s47-conn:AMPHENOL_RJHSE508102 X5
U 1 1 5F14F0A2
P 9300 1800
F 0 "X5" H 9300 2467 50  0000 C CNN
F 1 "E1" H 9300 2376 50  0000 C CNN
F 2 "Connector_RJ:RJ45_Amphenol_RJHSE538X-02" H 9400 1800 50  0001 C CNN
F 3 "https://cdn.amphenol-icc.com/media/wysiwyg/files/drawing/rjhsex08x02.pdf" H 9400 1800 50  0001 C CNN
F 4 "AMPHENOL RJHSE508102" H 9300 1800 50  0001 C CNN "MPN"
	1    9300 1800
	-1   0    0    1   
$EndComp
$Sheet
S 6500 1000 1000 1500
U 5F275ED4
F0 "e1A" 50
F1 "e1if.sch" 50
F2 "rj[1..8]" U R 7500 1200 50 
F3 "tx_hi" I L 6500 1200 50 
F4 "tx_lo" I L 6500 1300 50 
F5 "rx_lo_p" O L 6500 1900 50 
F6 "rx_lo_n" O L 6500 2000 50 
F7 "rx_hi_p" O L 6500 1600 50 
F8 "rx_hi_n" O L 6500 1700 50 
F9 "rx_bias_p" I L 6500 2200 50 
F10 "rx_bias_n" I L 6500 2300 50 
$EndSheet
$Sheet
S 3000 1000 1500 1500
U 5F27E0F4
F0 "gps" 50
F1 "gps.sch" 50
F2 "RF" I L 3000 1200 50 
F3 "i2c_sda" B R 4500 1800 50 
F4 "i2c_scl" B R 4500 1900 50 
F5 "rx" I R 4500 1400 50 
F6 "tx" O R 4500 1500 50 
F7 "~reset" I R 4500 1200 50 
F8 "pps" O R 4500 1300 50 
$EndSheet
$Comp
L s47-conn:AMPHENOL_RJHSE508102 X5
U 2 1 5F2825AC
P 9300 5300
F 0 "X5" H 9300 5967 50  0000 C CNN
F 1 "E1" H 9300 5876 50  0000 C CNN
F 2 "Connector_RJ:RJ45_Amphenol_RJHSE538X-02" H 9400 5300 50  0001 C CNN
F 3 "https://cdn.amphenol-icc.com/media/wysiwyg/files/drawing/rjhsex08x02.pdf" H 9400 5300 50  0001 C CNN
F 4 "AMPHENOL RJHSE508102" H 9300 5300 50  0001 C CNN "MPN"
	2    9300 5300
	-1   0    0    1   
$EndComp
Entry Wire Line
	10700 1700 10800 1800
Entry Wire Line
	10700 2000 10800 2100
Entry Wire Line
	10700 5300 10800 5400
Entry Wire Line
	10700 5400 10800 5500
Wire Bus Line
	8200 4700 7500 4700
Wire Bus Line
	8200 1200 7500 1200
Entry Wire Line
	8200 2100 8300 2200
Entry Wire Line
	8200 2000 8300 2100
Entry Wire Line
	8200 1900 8300 2000
Entry Wire Line
	8200 1800 8300 1900
Entry Wire Line
	8200 1700 8300 1800
Entry Wire Line
	8200 1600 8300 1700
Entry Wire Line
	8200 1500 8300 1600
Entry Wire Line
	8200 1400 8300 1500
Entry Wire Line
	8200 5600 8300 5700
Entry Wire Line
	8200 5500 8300 5600
Entry Wire Line
	8200 5400 8300 5500
Entry Wire Line
	8200 5300 8300 5400
Entry Wire Line
	8200 5200 8300 5300
Entry Wire Line
	8200 5100 8300 5200
Entry Wire Line
	8200 5000 8300 5100
Entry Wire Line
	8200 4900 8300 5000
Wire Wire Line
	8300 5000 8800 5000
Wire Wire Line
	8300 5100 8800 5100
Wire Wire Line
	8300 5200 8800 5200
Wire Wire Line
	8300 5300 8800 5300
Wire Wire Line
	8300 5400 8800 5400
Wire Wire Line
	8300 5500 8800 5500
Wire Wire Line
	8300 5600 8800 5600
Wire Wire Line
	8300 5700 8800 5700
Wire Wire Line
	8300 2200 8800 2200
Wire Wire Line
	8300 2100 8800 2100
Wire Wire Line
	8300 2000 8800 2000
Wire Wire Line
	8300 1900 8800 1900
Wire Wire Line
	8300 1800 8800 1800
Wire Wire Line
	8300 1700 8800 1700
Wire Wire Line
	8300 1600 8800 1600
Wire Wire Line
	8300 1500 8800 1500
Text Label 10400 5200 0    50   ~ 0
e1_led0
Text Label 10400 5500 0    50   ~ 0
e1_led1
Text Label 10400 1800 0    50   ~ 0
e1_led6
Text Label 8400 1500 0    50   ~ 0
e1A_rj1
Text Label 8400 1600 0    50   ~ 0
e1A_rj2
Text Label 8400 1700 0    50   ~ 0
e1A_rj3
Text Label 8400 1800 0    50   ~ 0
e1A_rj4
Text Label 8400 1900 0    50   ~ 0
e1A_rj5
Text Label 8400 2000 0    50   ~ 0
e1A_rj6
Text Label 8400 2100 0    50   ~ 0
e1A_rj7
Text Label 8400 2200 0    50   ~ 0
e1A_rj8
Text Label 7600 4700 0    50   ~ 0
e1B_rj[1..8]
Text Label 8400 5000 0    50   ~ 0
e1B_rj1
Text Label 8400 5100 0    50   ~ 0
e1B_rj2
Text Label 8400 5200 0    50   ~ 0
e1B_rj3
Text Label 8400 5300 0    50   ~ 0
e1B_rj4
Text Label 8400 5400 0    50   ~ 0
e1B_rj5
Text Label 8400 5500 0    50   ~ 0
e1B_rj6
Text Label 8400 5600 0    50   ~ 0
e1B_rj7
Text Label 8400 5700 0    50   ~ 0
e1B_rj8
Text Label 7600 1200 0    50   ~ 0
e1A_rj[1..8]
Wire Wire Line
	1900 3700 2000 3700
Wire Wire Line
	2000 3700 2000 3800
Wire Wire Line
	2000 3800 1900 3800
Wire Wire Line
	1900 3900 2000 3900
Wire Wire Line
	2000 3900 2000 4000
Wire Wire Line
	2000 4000 1900 4000
$Comp
L power:+5V #PWR04
U 1 1 5F2BD2FD
P 2000 3100
F 0 "#PWR04" H 2000 2950 50  0001 C CNN
F 1 "+5V" H 2015 3273 50  0000 C CNN
F 2 "" H 2000 3100 50  0001 C CNN
F 3 "" H 2000 3100 50  0001 C CNN
	1    2000 3100
	1    0    0    -1  
$EndComp
Wire Wire Line
	2000 3100 2000 3200
Wire Wire Line
	2000 3200 1900 3200
$Comp
L power:GND #PWR02
U 1 1 5F2BE95B
P 1300 5300
F 0 "#PWR02" H 1300 5050 50  0001 C CNN
F 1 "GND" H 1305 5127 50  0000 C CNN
F 2 "" H 1300 5300 50  0001 C CNN
F 3 "" H 1300 5300 50  0001 C CNN
	1    1300 5300
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 5F2C0111
P 1000 5000
F 0 "R1" H 1059 5046 50  0000 L CNN
F 1 "91k" H 1059 4955 50  0000 L CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 1000 5000 50  0001 C CNN
F 3 "~" H 1000 5000 50  0001 C CNN
	1    1000 5000
	1    0    0    -1  
$EndComp
$Comp
L Device:C_Small C1
U 1 1 5F2C06B2
P 800 5000
F 0 "C1" H 708 4954 50  0000 R CNN
F 1 "100n" H 708 5045 50  0000 R CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 800 5000 50  0001 C CNN
F 3 "~" H 800 5000 50  0001 C CNN
	1    800  5000
	1    0    0    1   
$EndComp
Wire Wire Line
	1300 4700 1300 5200
Wire Wire Line
	1000 4700 1000 4800
Wire Wire Line
	800  4900 800  4800
Wire Wire Line
	800  4800 1000 4800
Connection ~ 1000 4800
Wire Wire Line
	1000 4800 1000 4900
Wire Wire Line
	800  5100 800  5200
Wire Wire Line
	800  5200 1000 5200
Wire Wire Line
	1000 5200 1000 5100
Wire Wire Line
	1000 5200 1300 5200
Connection ~ 1000 5200
Connection ~ 1300 5200
Wire Wire Line
	1300 5200 1300 5300
$Comp
L power:GND #PWR05
U 1 1 5F2CAD6A
P 2300 3600
F 0 "#PWR05" H 2300 3350 50  0001 C CNN
F 1 "GND" H 2400 3600 50  0000 C CNN
F 2 "" H 2300 3600 50  0001 C CNN
F 3 "" H 2300 3600 50  0001 C CNN
	1    2300 3600
	1    0    0    -1  
$EndComp
Wire Wire Line
	2200 3500 2300 3500
Wire Wire Line
	2300 3500 2300 3600
Wire Wire Line
	2200 3400 2300 3400
Wire Wire Line
	2300 3400 2300 3500
Connection ~ 2300 3500
Wire Wire Line
	2000 3400 1900 3400
Wire Wire Line
	2000 3500 1900 3500
$Comp
L Device:R_Small R2
U 1 1 5F2CB2E3
P 2100 3400
F 0 "R2" V 2000 3300 50  0000 C CNN
F 1 "5k1" V 2000 3500 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 2100 3400 50  0001 C CNN
F 3 "~" H 2100 3400 50  0001 C CNN
	1    2100 3400
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R3
U 1 1 5F2CB885
P 2100 3500
F 0 "R3" V 2200 3400 50  0000 C CNN
F 1 "5k1" V 2200 3600 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" H 2100 3500 50  0001 C CNN
F 3 "~" H 2100 3500 50  0001 C CNN
	1    2100 3500
	0    1    1    0   
$EndComp
Connection ~ 2000 3800
Connection ~ 2000 3900
$Comp
L Connector:Conn_Coaxial X1
U 1 1 5F2D90EA
P 1000 1200
AR Path="/5F2D90EA" Ref="X1"  Part="1" 
AR Path="/5F27E0F4/5F2D90EA" Ref="X?"  Part="1" 
F 0 "X1" H 1100 1175 50  0000 L CNN
F 1 "GPS" H 1100 1084 50  0000 L CNN
F 2 "s47-conn:SMA_TEConnectivity_619540-1_Horizontal" H 1000 1200 50  0001 C CNN
F 3 " ~" H 1000 1200 50  0001 C CNN
F 4 "TEConnectivity 619540-1" H 1000 1200 50  0001 C CNN "MPN"
	1    1000 1200
	-1   0    0    -1  
$EndComp
$Comp
L power:GND #PWR01
U 1 1 5F2E3710
P 1000 1500
F 0 "#PWR01" H 1000 1250 50  0001 C CNN
F 1 "GND" H 1005 1327 50  0000 C CNN
F 2 "" H 1000 1500 50  0001 C CNN
F 3 "" H 1000 1500 50  0001 C CNN
	1    1000 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1000 1400 1000 1500
$Comp
L Connector:AudioJack3 X2
U 1 1 5F33C437
P 1100 2300
AR Path="/5F33C437" Ref="X2"  Part="1" 
AR Path="/5F1321E5/5F33C437" Ref="X?"  Part="1" 
F 0 "X2" H 1082 2625 50  0000 C CNN
F 1 "Serial" H 1082 2534 50  0000 C CNN
F 2 "s47-conn:Jack_2.5mm_CUI_SJ-2523-SMT_Horizontal" H 1100 2300 50  0001 C CNN
F 3 "~" H 1100 2300 50  0001 C CNN
F 4 "CUI SJ-2523-SMT" H 1100 2300 50  0001 C CNN "MPN"
	1    1100 2300
	1    0    0    1   
$EndComp
$Comp
L power:GND #PWR03
U 1 1 5F35C170
P 1400 2500
F 0 "#PWR03" H 1400 2250 50  0001 C CNN
F 1 "GND" H 1405 2327 50  0000 C CNN
F 2 "" H 1400 2500 50  0001 C CNN
F 3 "" H 1400 2500 50  0001 C CNN
	1    1400 2500
	1    0    0    -1  
$EndComp
Wire Wire Line
	1300 2400 1400 2400
Wire Wire Line
	1400 2400 1400 2500
Text Label 1400 2300 0    50   ~ 0
dbg_tx
Text Label 1400 2200 0    50   ~ 0
dbg_rx
Text Label 2100 3900 0    50   ~ 0
usb_dp
Text Label 2100 3800 0    50   ~ 0
usb_dn
Entry Wire Line
	10700 1800 10800 1900
Entry Wire Line
	10700 5200 10800 5300
Text Label 10400 2000 0    50   ~ 0
e1_led4
Text Label 10400 1900 0    50   ~ 0
e1_led5
Text Label 10400 5400 0    50   ~ 0
e1_led2
Wire Wire Line
	10700 1700 10300 1700
Wire Wire Line
	9900 1500 9800 1500
Wire Wire Line
	9900 5000 9800 5000
Wire Wire Line
	10300 5200 10700 5200
Entry Wire Line
	10700 1900 10800 2000
Entry Wire Line
	10700 5500 10800 5600
Wire Wire Line
	10700 1900 10300 1900
Wire Wire Line
	9800 1900 9800 2100
Text Label 10400 5300 0    50   ~ 0
e1_led3
Text Label 10400 1700 0    50   ~ 0
e1_led7
$Comp
L Device:R_Pack04 RN1
U 1 1 5F4BB9CE
P 10100 1900
F 0 "RN1" V 9683 1900 50  0000 C CNN
F 1 "33R" V 9774 1900 50  0000 C CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" V 10375 1900 50  0001 C CNN
F 3 "~" H 10100 1900 50  0001 C CNN
	1    10100 1900
	0    1    1    0   
$EndComp
$Comp
L Device:R_Pack04 RN2
U 1 1 5F4BC470
P 10100 5400
F 0 "RN2" V 9683 5400 50  0000 C CNN
F 1 "33R" V 9774 5400 50  0000 C CNN
F 2 "Resistor_SMD:R_Array_Convex_4x0603" V 10375 5400 50  0001 C CNN
F 3 "~" H 10100 5400 50  0001 C CNN
	1    10100 5400
	0    1    1    0   
$EndComp
Wire Wire Line
	9800 1600 9800 1800
Wire Wire Line
	9800 1800 9900 1800
Wire Wire Line
	9900 1500 9900 1700
Wire Wire Line
	9800 1900 9900 1900
Wire Wire Line
	9800 2200 9900 2200
Wire Wire Line
	9900 2200 9900 2000
Wire Wire Line
	10700 1800 10300 1800
Wire Wire Line
	10300 2000 10700 2000
Wire Wire Line
	10700 5300 10300 5300
Wire Wire Line
	10700 5400 10300 5400
Wire Wire Line
	10700 5500 10300 5500
Wire Wire Line
	9800 5300 9900 5300
Wire Wire Line
	9800 5100 9800 5300
Wire Wire Line
	9800 5400 9900 5400
Wire Wire Line
	9800 5400 9800 5600
Wire Wire Line
	9800 5700 9900 5700
Wire Wire Line
	9900 5700 9900 5500
Wire Wire Line
	9900 5000 9900 5200
$Sheet
S 6500 3000 1000 1000
U 5F6ACE9B
F0 "e1bias" 50
F1 "e1bias.sch" 50
F2 "bias0" I L 6500 3400 50 
F3 "bias1" I L 6500 3500 50 
F4 "bias_a_p" O L 6500 3200 50 
F5 "bias_a_n" O L 6500 3100 50 
F6 "bias_b_p" O L 6500 3800 50 
F7 "bias_b_n" O L 6500 3700 50 
$EndSheet
$Comp
L Connector:RJ45 X3
U 1 1 5F26EF88
P 1200 7000
F 0 "X3" H 1257 7667 50  0000 C CNN
F 1 "GPIO" H 1257 7576 50  0000 C CNN
F 2 "Connector_RJ:RJ45_Amphenol_RJHSE5380" V 1200 7025 50  0001 C CNN
F 3 "~" V 1200 7025 50  0001 C CNN
F 4 "Amphenol RJHSE5080" H 1200 7000 50  0001 C CNN "MPN"
	1    1200 7000
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR08
U 1 1 5F6DF7A7
P 4400 7200
F 0 "#PWR08" H 4400 6950 50  0001 C CNN
F 1 "GND" H 4405 7027 50  0000 C CNN
F 2 "" H 4400 7200 50  0001 C CNN
F 3 "" H 4400 7200 50  0001 C CNN
	1    4400 7200
	1    0    0    -1  
$EndComp
Wire Wire Line
	4300 6900 4600 6900
Wire Wire Line
	4300 7000 4700 7000
$Comp
L power:+3V3 #PWR07
U 1 1 5F6FA346
P 4400 6700
F 0 "#PWR07" H 4400 6550 50  0001 C CNN
F 1 "+3V3" H 4415 6873 50  0000 C CNN
F 2 "" H 4400 6700 50  0001 C CNN
F 3 "" H 4400 6700 50  0001 C CNN
	1    4400 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	4400 6700 4400 6800
Wire Wire Line
	4400 6800 4300 6800
Wire Wire Line
	4300 7100 4400 7100
Wire Wire Line
	4400 7100 4400 7200
Wire Wire Line
	4500 3700 4600 3700
Wire Wire Line
	4600 3700 4600 1800
Wire Wire Line
	4600 1800 4500 1800
Wire Wire Line
	4600 3700 4600 6900
Connection ~ 4600 3700
Wire Wire Line
	4700 7000 4700 3800
Wire Wire Line
	4700 1900 4500 1900
Wire Wire Line
	4500 3800 4700 3800
Connection ~ 4700 3800
Wire Wire Line
	4700 3800 4700 1900
Wire Wire Line
	4500 3200 4800 3200
Wire Wire Line
	4800 3200 4800 1500
Wire Wire Line
	4800 1500 4500 1500
Wire Wire Line
	4500 3300 4900 3300
Wire Wire Line
	4900 3300 4900 1400
Wire Wire Line
	4900 1400 4500 1400
Wire Wire Line
	4500 1300 5000 1300
Wire Wire Line
	5000 1300 5000 3400
Wire Wire Line
	5000 3400 4500 3400
Wire Wire Line
	4500 3500 5100 3500
Wire Wire Line
	5100 3500 5100 1200
Wire Wire Line
	5100 1200 4500 1200
Wire Wire Line
	4500 4000 5300 4000
Wire Wire Line
	5300 4000 5300 1200
Wire Wire Line
	5300 1200 6500 1200
Wire Wire Line
	6500 1300 5400 1300
Wire Wire Line
	5400 1300 5400 4100
Wire Wire Line
	5400 4100 4500 4100
Wire Wire Line
	4500 4200 5500 4200
Wire Wire Line
	5500 4200 5500 1600
Wire Wire Line
	5500 1600 6500 1600
Wire Wire Line
	4500 4300 5600 4300
Wire Wire Line
	5600 4300 5600 1700
Wire Wire Line
	5600 1700 6500 1700
Wire Wire Line
	6500 1900 5700 1900
Wire Wire Line
	5700 1900 5700 4400
Wire Wire Line
	5700 4400 4500 4400
Wire Wire Line
	6500 2000 5800 2000
Wire Wire Line
	5800 2000 5800 4500
Wire Wire Line
	5800 4500 4500 4500
Wire Wire Line
	6500 2300 6400 2300
Wire Wire Line
	6400 2300 6400 3100
Wire Wire Line
	6400 3100 6500 3100
Wire Wire Line
	6500 3200 6300 3200
Wire Wire Line
	6300 3200 6300 2200
Wire Wire Line
	6300 2200 6500 2200
Text Label 3400 6800 0    50   ~ 0
gpio0
Text Label 3400 6900 0    50   ~ 0
gpio1
Text Label 3400 7000 0    50   ~ 0
gpio2
Wire Bus Line
	4500 5800 4800 5800
Wire Bus Line
	4800 5800 4800 6300
Wire Bus Line
	4800 6300 3300 6300
Entry Wire Line
	3300 6700 3400 6800
Entry Wire Line
	3300 6800 3400 6900
Entry Wire Line
	3300 6900 3400 7000
Wire Bus Line
	4500 5700 4900 5700
Wire Bus Line
	4900 5700 4900 6300
Wire Bus Line
	4900 6300 10800 6300
Wire Wire Line
	4500 4700 5900 4700
Wire Wire Line
	5900 4700 5900 3400
Wire Wire Line
	5900 3400 6500 3400
Wire Wire Line
	4500 4800 6000 4800
Wire Wire Line
	6000 4800 6000 3500
Wire Wire Line
	6000 3500 6500 3500
$Sheet
S 5500 6500 1000 1000
U 5F2D7F29
F0 "psu" 50
F1 "psu.sch" 50
$EndSheet
$Sheet
S 6500 4500 1000 1500
U 5F276580
F0 "e1B" 50
F1 "e1if.sch" 50
F2 "rj[1..8]" U R 7500 4700 50 
F3 "tx_hi" I L 6500 4700 50 
F4 "tx_lo" I L 6500 4800 50 
F5 "rx_lo_p" O L 6500 5700 50 
F6 "rx_lo_n" O L 6500 5800 50 
F7 "rx_hi_p" O L 6500 5400 50 
F8 "rx_hi_n" O L 6500 5500 50 
F9 "rx_bias_p" I L 6500 5100 50 
F10 "rx_bias_n" I L 6500 5200 50 
$EndSheet
Wire Wire Line
	6400 3800 6500 3800
Wire Wire Line
	6300 3700 6500 3700
Wire Wire Line
	6500 4700 6100 4700
Wire Wire Line
	6100 4700 6100 5000
Wire Wire Line
	6100 5000 4500 5000
Wire Wire Line
	6500 4800 6200 4800
Wire Wire Line
	6200 4800 6200 5100
Wire Wire Line
	6200 5100 4500 5100
Wire Wire Line
	6400 3800 6400 5100
Wire Wire Line
	6400 5100 6500 5100
Wire Wire Line
	6500 5200 6300 5200
Wire Wire Line
	6300 5200 6300 3700
Wire Wire Line
	4500 5200 6200 5200
Wire Wire Line
	6200 5200 6200 5400
Wire Wire Line
	6200 5400 6500 5400
Wire Wire Line
	6500 5500 6100 5500
Wire Wire Line
	6100 5500 6100 5300
Wire Wire Line
	6100 5300 4500 5300
Wire Wire Line
	4500 5400 6000 5400
Wire Wire Line
	6000 5400 6000 5700
Wire Wire Line
	6000 5700 6500 5700
Wire Wire Line
	6500 5800 5900 5800
Wire Wire Line
	5900 5800 5900 5500
Wire Wire Line
	5900 5500 4500 5500
Wire Wire Line
	2000 3800 3000 3800
Wire Wire Line
	2000 3900 3000 3900
NoConn ~ 1900 4300
NoConn ~ 1900 4400
Wire Wire Line
	2700 2200 2700 3400
Wire Wire Line
	2700 3400 3000 3400
Wire Wire Line
	1300 2200 2700 2200
Wire Wire Line
	3000 3500 2600 3500
Wire Wire Line
	2600 3500 2600 2300
Wire Wire Line
	1300 2300 2600 2300
Wire Wire Line
	1200 1200 3000 1200
Wire Wire Line
	3800 6800 3400 6800
Wire Wire Line
	3800 6900 3400 6900
Wire Wire Line
	3800 7000 3400 7000
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J2
U 1 1 5F54C3EB
P 4100 7000
F 0 "J2" H 4150 6575 50  0000 C CNN
F 1 "EXT_IN" H 4150 6666 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical" H 4100 7000 50  0001 C CNN
F 3 "~" H 4100 7000 50  0001 C CNN
	1    4100 7000
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic:Conn_02x04_Odd_Even J1
U 1 1 5F363772
P 2400 7000
F 0 "J1" H 2450 6550 50  0000 C CNN
F 1 "EXT_OUT" H 2450 6650 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical" H 2400 7000 50  0001 C CNN
F 3 "~" H 2400 7000 50  0001 C CNN
	1    2400 7000
	-1   0    0    1   
$EndComp
Wire Wire Line
	1600 7200 2000 7200
Wire Wire Line
	2000 7200 2000 7100
Wire Wire Line
	2000 7100 2100 7100
Wire Wire Line
	1600 7000 2100 7000
Wire Wire Line
	1600 6800 1900 6800
Wire Wire Line
	1900 6800 1900 6900
Wire Wire Line
	1900 6900 2100 6900
Wire Wire Line
	1600 6600 2000 6600
Wire Wire Line
	2000 6600 2000 6800
Wire Wire Line
	2000 6800 2100 6800
Wire Wire Line
	1600 7300 2700 7300
Wire Wire Line
	2700 7300 2700 7100
Wire Wire Line
	2700 7100 2600 7100
Wire Wire Line
	1600 7100 1900 7100
Wire Wire Line
	1900 7100 1900 7400
Wire Wire Line
	1900 7400 2800 7400
Wire Wire Line
	2800 7400 2800 7000
Wire Wire Line
	2800 7000 2600 7000
Wire Wire Line
	1600 6900 1800 6900
Wire Wire Line
	1800 6900 1800 7500
Wire Wire Line
	1800 7500 2900 7500
Wire Wire Line
	2900 7500 2900 6900
Wire Wire Line
	2900 6900 2600 6900
Wire Wire Line
	1600 6700 1700 6700
Wire Wire Line
	1700 6700 1700 7600
Wire Wire Line
	1700 7600 3000 7600
Wire Wire Line
	3000 7600 3000 6800
Wire Wire Line
	3000 6800 2600 6800
Text Label 5000 6300 0    50   ~ 0
e1_led[0..7]
Text Label 3900 6300 0    50   ~ 0
gpio[0..2]
$Comp
L power:+5V #PWR06
U 1 1 5F4C5366
P 3700 6700
F 0 "#PWR06" H 3700 6550 50  0001 C CNN
F 1 "+5V" H 3715 6873 50  0000 C CNN
F 2 "" H 3700 6700 50  0001 C CNN
F 3 "" H 3700 6700 50  0001 C CNN
	1    3700 6700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3800 7100 3700 7100
Wire Wire Line
	3700 7100 3700 6700
Wire Bus Line
	3300 6300 3300 6900
Wire Bus Line
	10800 1800 10800 6300
Wire Bus Line
	8200 4700 8200 5600
Wire Bus Line
	8200 1200 8200 2100
$EndSCHEMATC
