EESchema Schematic File Version 4
LIBS:spherometer-display-cache
EELAYER 29 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L device:Q_NPN_BEC Q2
U 1 1 5C7BA6FA
P 2750 4050
F 0 "Q2" H 2941 4096 50  0000 L CNN
F 1 "NPN" H 2941 4005 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 2950 4150 50  0001 C CNN
F 3 "" H 2750 4050 50  0001 C CNN
	1    2750 4050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5C7BB0D1
P 2850 3600
F 0 "#PWR0101" H 2850 3350 50  0001 C CNN
F 1 "GND" H 2855 3427 50  0000 C CNN
F 2 "" H 2850 3600 50  0001 C CNN
F 3 "" H 2850 3600 50  0001 C CNN
	1    2850 3600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0102
U 1 1 5C7BC1B6
P 2850 4250
F 0 "#PWR0102" H 2850 4000 50  0001 C CNN
F 1 "GND" H 2855 4077 50  0000 C CNN
F 2 "" H 2850 4250 50  0001 C CNN
F 3 "" H 2850 4250 50  0001 C CNN
	1    2850 4250
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0103
U 1 1 5C7BC608
P 6100 2900
F 0 "#PWR0103" H 6100 2750 50  0001 C CNN
F 1 "VCC" H 6117 3073 50  0000 C CNN
F 2 "" H 6100 2900 50  0001 C CNN
F 3 "" H 6100 2900 50  0001 C CNN
	1    6100 2900
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5C7BCED2
P 6100 3400
F 0 "#PWR0104" H 6100 3150 50  0001 C CNN
F 1 "GND" H 6105 3227 50  0000 C CNN
F 2 "" H 6100 3400 50  0001 C CNN
F 3 "" H 6100 3400 50  0001 C CNN
	1    6100 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04 J3
U 1 1 5C7BD895
P 2750 2200
F 0 "J3" V 2714 1912 50  0000 R CNN
F 1 "Display" V 2623 1912 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x04_Pitch2.54mm" H 2750 2200 50  0001 C CNN
F 3 "~" H 2750 2200 50  0001 C CNN
	1    2750 2200
	0    -1   -1   0   
$EndComp
$Comp
L power:VCC #PWR0105
U 1 1 5C7BEAB5
P 2950 2400
F 0 "#PWR0105" H 2950 2250 50  0001 C CNN
F 1 "VCC" V 2967 2528 50  0000 L CNN
F 2 "" H 2950 2400 50  0001 C CNN
F 3 "" H 2950 2400 50  0001 C CNN
	1    2950 2400
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5C7BF5EC
P 2850 2400
F 0 "#PWR0106" H 2850 2150 50  0001 C CNN
F 1 "GND" H 2855 2227 50  0000 C CNN
F 2 "" H 2850 2400 50  0001 C CNN
F 3 "" H 2850 2400 50  0001 C CNN
	1    2850 2400
	1    0    0    -1  
$EndComp
$Comp
L atmel:ATTINY85-20SU U1
U 1 1 5C7C0CD1
P 4750 3150
F 0 "U1" H 4750 3667 50  0000 C CNN
F 1 "ATTINY85V-10SU" H 4750 3576 50  0000 C CNN
F 2 "Housings_SOIC:SOIC-8_3.9x4.9mm_Pitch1.27mm" H 5700 3150 50  0001 C CIN
F 3 "http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf" H 4750 3150 50  0001 C CNN
	1    4750 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	2650 2400 2650 2900
Wire Wire Line
	2650 2900 3400 2900
Wire Wire Line
	2750 2400 2750 3100
Wire Wire Line
	2750 3100 3400 3100
Text Label 2900 2900 0    50   ~ 0
SDA
Text Label 2900 3100 0    50   ~ 0
SCL
$Comp
L Switch:SW_Push SW2
U 1 1 5C7C2518
P 3750 2400
F 0 "SW2" H 4250 2500 50  0000 C CNN
F 1 "LargePush" H 4250 2400 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_TL3342" H 3750 2600 50  0001 C CNN
F 3 "" H 3750 2600 50  0001 C CNN
	1    3750 2400
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5C7C3C44
P 3950 2400
F 0 "#PWR0107" H 3950 2150 50  0001 C CNN
F 1 "GND" H 3955 2227 50  0000 C CNN
F 2 "" H 3950 2400 50  0001 C CNN
F 3 "" H 3950 2400 50  0001 C CNN
	1    3950 2400
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 2400 3300 2400
Wire Wire Line
	3300 2400 3300 3000
Wire Wire Line
	3300 3000 3400 3000
Text Label 3300 2600 0    50   ~ 0
Button
Wire Wire Line
	2850 3200 3400 3200
Wire Wire Line
	2850 3850 3200 3850
Wire Wire Line
	3200 3850 3200 3300
Wire Wire Line
	3200 3300 3400 3300
$Comp
L device:Q_NPN_BEC Q1
U 1 1 5C7C80D0
P 2750 3400
F 0 "Q1" H 2941 3446 50  0000 L CNN
F 1 "NPN" H 2941 3355 50  0000 L CNN
F 2 "TO_SOT_Packages_SMD:SOT-23" H 2950 3500 50  0001 C CNN
F 3 "" H 2750 3400 50  0001 C CNN
	1    2750 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector:Conn_01x04 J2
U 1 1 5C7C8F49
P 2050 2200
F 0 "J2" V 2014 1912 50  0000 R CNN
F 1 "Indicator" V 1950 2700 50  0000 R CNN
F 2 "Connectors_JST:JST_PH_S4B-PH-K_04x2.00mm_Angled" H 2050 2200 50  0001 C CNN
F 3 "~" H 2050 2200 50  0001 C CNN
	1    2050 2200
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0108
U 1 1 5C7C9DE0
P 2250 2400
F 0 "#PWR0108" H 2250 2150 50  0001 C CNN
F 1 "GND" H 2255 2227 50  0000 C CNN
F 2 "" H 2250 2400 50  0001 C CNN
F 3 "" H 2250 2400 50  0001 C CNN
	1    2250 2400
	1    0    0    -1  
$EndComp
$Comp
L device:R R1
U 1 1 5C7CA7C5
P 2400 3400
F 0 "R1" V 2193 3400 50  0000 C CNN
F 1 "33k" V 2284 3400 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2330 3400 50  0001 C CNN
F 3 "" H 2400 3400 50  0001 C CNN
	1    2400 3400
	0    1    1    0   
$EndComp
$Comp
L device:R R2
U 1 1 5C7CB12C
P 2400 4050
F 0 "R2" V 2193 4050 50  0000 C CNN
F 1 "33k" V 2284 4050 50  0000 C CNN
F 2 "Resistors_SMD:R_0603" V 2330 4050 50  0001 C CNN
F 3 "" H 2400 4050 50  0001 C CNN
	1    2400 4050
	0    1    1    0   
$EndComp
Wire Wire Line
	2150 2400 2150 3400
Wire Wire Line
	2150 3400 2250 3400
Wire Wire Line
	2050 2400 2050 4050
Wire Wire Line
	2050 4050 2250 4050
$Comp
L Connector:Conn_01x03 J1
U 1 1 5C7CC029
P 1300 3050
F 0 "J1" H 1300 2850 50  0000 C CNN
F 1 "Battery" H 1500 3050 50  0000 C CNN
F 2 "Connectors_JST:JST_PH_S3B-PH-K_03x2.00mm_Angled" H 1300 3050 50  0001 C CNN
F 3 "~" H 1300 3050 50  0001 C CNN
	1    1300 3050
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5C7CDE1C
P 1500 3150
F 0 "#PWR0109" H 1500 2900 50  0001 C CNN
F 1 "GND" H 1505 2977 50  0000 C CNN
F 2 "" H 1500 3150 50  0001 C CNN
F 3 "" H 1500 3150 50  0001 C CNN
	1    1500 3150
	1    0    0    -1  
$EndComp
$Comp
L power:+1V5 #PWR0110
U 1 1 5C7CE6F4
P 1500 3050
F 0 "#PWR0110" H 1500 2900 50  0001 C CNN
F 1 "+1V5" V 1515 3178 50  0000 L CNN
F 2 "" H 1500 3050 50  0001 C CNN
F 3 "" H 1500 3050 50  0001 C CNN
	1    1500 3050
	0    1    1    0   
$EndComp
$Comp
L power:VCC #PWR0111
U 1 1 5C7CF276
P 1500 2950
F 0 "#PWR0111" H 1500 2800 50  0001 C CNN
F 1 "VCC" H 1517 3123 50  0000 C CNN
F 2 "" H 1500 2950 50  0001 C CNN
F 3 "" H 1500 2950 50  0001 C CNN
	1    1500 2950
	1    0    0    -1  
$EndComp
$Comp
L power:+1V5 #PWR0112
U 1 1 5C7CF95A
P 1950 2400
F 0 "#PWR0112" H 1950 2250 50  0001 C CNN
F 1 "+1V5" H 1965 2573 50  0000 C CNN
F 2 "" H 1950 2400 50  0001 C CNN
F 3 "" H 1950 2400 50  0001 C CNN
	1    1950 2400
	-1   0    0    1   
$EndComp
$Comp
L Switch:SW_Push SW1
U 1 1 5C7D1A93
P 3750 2200
F 0 "SW1" H 4200 2300 50  0000 C CNN
F 1 "SmallPush" H 4250 2200 50  0000 C CNN
F 2 "Buttons_Switches_SMD:SW_SPST_PTS645" H 3750 2400 50  0001 C CNN
F 3 "" H 3750 2400 50  0001 C CNN
	1    3750 2200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3550 2200 3550 2400
Connection ~ 3550 2400
Wire Wire Line
	3950 2200 3950 2400
Connection ~ 3950 2400
Text Notes 4500 2350 0    50   ~ 0
Alternatives
$Comp
L device:R R3
U 1 1 5C7D3C7F
P 3400 3550
F 0 "R3" H 3330 3504 50  0000 R CNN
F 1 "33k" H 3330 3595 50  0000 R CNN
F 2 "Resistors_SMD:R_0603" V 3330 3550 50  0001 C CNN
F 3 "" H 3400 3550 50  0001 C CNN
	1    3400 3550
	-1   0    0    1   
$EndComp
$Comp
L power:VCC #PWR01
U 1 1 5C7D4897
P 3400 3700
F 0 "#PWR01" H 3400 3550 50  0001 C CNN
F 1 "VCC" V 3417 3828 50  0000 L CNN
F 2 "" H 3400 3700 50  0001 C CNN
F 3 "" H 3400 3700 50  0001 C CNN
	1    3400 3700
	0    1    1    0   
$EndComp
$Comp
L device:C C1
U 1 1 5C7B9330
P 6100 3150
F 0 "C1" H 6215 3196 50  0000 L CNN
F 1 "100n" H 6215 3105 50  0000 L CNN
F 2 "Capacitors_SMD:C_0603" H 6138 3000 50  0001 C CNN
F 3 "" H 6100 3150 50  0001 C CNN
	1    6100 3150
	1    0    0    -1  
$EndComp
Wire Wire Line
	6100 2900 6100 3000
Connection ~ 6100 2900
Wire Wire Line
	6100 3300 6100 3400
Connection ~ 6100 3400
$EndSCHEMATC