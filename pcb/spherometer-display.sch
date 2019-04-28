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
L Device:Q_NPN_BEC Q2
U 1 1 5C7BA6FA
P 2750 4050
F 0 "Q2" H 2941 4096 50  0000 L CNN
F 1 "NPN" H 2941 4005 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2950 4150 50  0001 C CNN
F 3 "" H 2750 4050 50  0001 C CNN
	1    2750 4050
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5C7BB0D1
P 2850 3600
F 0 "#PWR0101" H 2850 3350 50  0001 C CNN
F 1 "GND" H 2950 3500 50  0000 C CNN
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
F 1 "GND" H 2950 4150 50  0000 C CNN
F 2 "" H 2850 4250 50  0001 C CNN
F 3 "" H 2850 4250 50  0001 C CNN
	1    2850 4250
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0103
U 1 1 5C7BC608
P 4950 2600
F 0 "#PWR0103" H 4950 2450 50  0001 C CNN
F 1 "VCC" H 4967 2773 50  0000 C CNN
F 2 "" H 4950 2600 50  0001 C CNN
F 3 "" H 4950 2600 50  0001 C CNN
	1    4950 2600
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0104
U 1 1 5C7BCED2
P 4950 3800
F 0 "#PWR0104" H 4950 3550 50  0001 C CNN
F 1 "GND" H 4955 3627 50  0000 C CNN
F 2 "" H 4950 3800 50  0001 C CNN
F 3 "" H 4950 3800 50  0001 C CNN
	1    4950 3800
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0105
U 1 1 5C7BEAB5
P 2800 2400
F 0 "#PWR0105" H 2800 2250 50  0001 C CNN
F 1 "VCC" V 2817 2528 50  0000 L CNN
F 2 "" H 2800 2400 50  0001 C CNN
F 3 "" H 2800 2400 50  0001 C CNN
	1    2800 2400
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0106
U 1 1 5C7BF5EC
P 2900 2400
F 0 "#PWR0106" H 2900 2150 50  0001 C CNN
F 1 "GND" H 2905 2227 50  0000 C CNN
F 2 "" H 2900 2400 50  0001 C CNN
F 3 "" H 2900 2400 50  0001 C CNN
	1    2900 2400
	1    0    0    -1  
$EndComp
$Comp
L MCU_Microchip_ATtiny:ATtiny85V-10SU U1
U 1 1 5C7C0CD1
P 4050 3200
F 0 "U1" H 3650 3450 50  0000 C CNN
F 1 "ATTINY85V-10SU" H 3900 3350 50  0000 C CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 5000 3200 50  0001 C CIN
F 3 "http://www.atmel.com/images/atmel-2586-avr-8-bit-microcontroller-attiny25-attiny45-attiny85_datasheet.pdf" H 4050 3200 50  0001 C CNN
	1    4050 3200
	-1   0    0    -1  
$EndComp
Text Label 2800 2900 0    50   ~ 0
SDA_MOSI
Text Label 2800 3100 0    50   ~ 0
SCL_SCK
$Comp
L Switch:SW_Push SW1
U 1 1 5C7C2518
P 3650 2450
F 0 "SW1" H 4150 2550 50  0000 C CNN
F 1 "UI-Button" H 4150 2450 50  0000 C CNN
F 2 "digi-spherometer:EVPAV-Switch" H 3650 2650 50  0001 C CNN
F 3 "" H 3650 2650 50  0001 C CNN
	1    3650 2450
	1    0    0    -1  
$EndComp
$Comp
L power:GND #PWR0107
U 1 1 5C7C3C44
P 3850 2450
F 0 "#PWR0107" H 3850 2200 50  0001 C CNN
F 1 "GND" H 3855 2277 50  0000 C CNN
F 2 "" H 3850 2450 50  0001 C CNN
F 3 "" H 3850 2450 50  0001 C CNN
	1    3850 2450
	1    0    0    -1  
$EndComp
Wire Wire Line
	2850 3200 3450 3200
Wire Wire Line
	2850 3850 3200 3850
Wire Wire Line
	3200 3850 3200 3300
Wire Wire Line
	3200 3300 3450 3300
$Comp
L Device:Q_NPN_BEC Q1
U 1 1 5C7C80D0
P 2750 3400
F 0 "Q1" H 2941 3446 50  0000 L CNN
F 1 "NPN" H 2941 3355 50  0000 L CNN
F 2 "Package_TO_SOT_SMD:SOT-23" H 2950 3500 50  0001 C CNN
F 3 "" H 2750 3400 50  0001 C CNN
	1    2750 3400
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J2
U 1 1 5C7C8F49
P 2050 2200
F 0 "J2" V 2050 2500 50  0000 R CNN
F 1 "Indicator" V 1950 2700 50  0000 R CNN
F 2 "Connector_JST:JST_PH_S4B-PH-K_1x04_P2.00mm_Horizontal" H 2050 2200 50  0001 C CNN
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
L Device:R R1
U 1 1 5C7CA7C5
P 2400 3400
F 0 "R1" V 2400 3400 50  0000 C CNN
F 1 "47k" V 2500 3400 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 2330 3400 50  0001 C CNN
F 3 "" H 2400 3400 50  0001 C CNN
	1    2400 3400
	0    1    1    0   
$EndComp
$Comp
L Device:R R2
U 1 1 5C7CB12C
P 2400 4050
F 0 "R2" V 2400 4050 50  0000 C CNN
F 1 "47k" V 2500 4050 50  0000 C CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 2330 4050 50  0001 C CNN
F 3 "" H 2400 4050 50  0001 C CNN
	1    2400 4050
	0    1    1    0   
$EndComp
Wire Wire Line
	2150 2400 2150 3200
Wire Wire Line
	2150 3400 2250 3400
Wire Wire Line
	2050 2400 2050 3850
Wire Wire Line
	2050 4050 2250 4050
$Comp
L Connector_Generic:Conn_01x03 J1
U 1 1 5C7CC029
P 1300 2750
F 0 "J1" H 1300 2550 50  0000 C CNN
F 1 "Battery" H 1500 2650 50  0000 C CNN
F 2 "Connector_JST:JST_PH_S3B-PH-K_1x03_P2.00mm_Horizontal" H 1300 2750 50  0001 C CNN
F 3 "~" H 1300 2750 50  0001 C CNN
	1    1300 2750
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0109
U 1 1 5C7CDE1C
P 1500 2850
F 0 "#PWR0109" H 1500 2600 50  0001 C CNN
F 1 "GND" H 1505 2677 50  0000 C CNN
F 2 "" H 1500 2850 50  0001 C CNN
F 3 "" H 1500 2850 50  0001 C CNN
	1    1500 2850
	1    0    0    -1  
$EndComp
$Comp
L power:VCC #PWR0111
U 1 1 5C7CF276
P 1500 2650
F 0 "#PWR0111" H 1500 2500 50  0001 C CNN
F 1 "VCC" H 1517 2823 50  0000 C CNN
F 2 "" H 1500 2650 50  0001 C CNN
F 3 "" H 1500 2650 50  0001 C CNN
	1    1500 2650
	1    0    0    -1  
$EndComp
$Comp
L Device:R R3
U 1 1 5C7D3C7F
P 3300 3550
F 0 "R3" V 3300 3600 50  0000 R CNN
F 1 "47k" H 3300 3700 50  0000 R CNN
F 2 "Resistor_SMD:R_0603_1608Metric" V 3230 3550 50  0001 C CNN
F 3 "" H 3300 3550 50  0001 C CNN
	1    3300 3550
	-1   0    0    1   
$EndComp
$Comp
L power:VCC #PWR01
U 1 1 5C7D4897
P 3300 3700
F 0 "#PWR01" H 3300 3550 50  0001 C CNN
F 1 "VCC" V 3317 3828 50  0000 L CNN
F 2 "" H 3300 3700 50  0001 C CNN
F 3 "" H 3300 3700 50  0001 C CNN
	1    3300 3700
	-1   0    0    1   
$EndComp
$Comp
L Device:C C1
U 1 1 5C7B9330
P 4950 3200
F 0 "C1" H 5065 3246 50  0000 L CNN
F 1 "100n" H 5065 3155 50  0000 L CNN
F 2 "Capacitor_SMD:C_0603_1608Metric" H 4988 3050 50  0001 C CNN
F 3 "" H 4950 3200 50  0001 C CNN
	1    4950 3200
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic:Conn_01x04 J3
U 1 1 5CBCFE92
P 2700 2200
F 0 "J3" V 2700 1900 50  0000 R CNN
F 1 "Disp" V 2600 1950 50  0000 R CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical" H 2700 2200 50  0001 C CNN
F 3 "~" H 2700 2200 50  0001 C CNN
	1    2700 2200
	0    -1   -1   0   
$EndComp
Wire Wire Line
	3450 3400 3300 3400
Wire Wire Line
	4050 3800 4950 3800
Wire Wire Line
	4950 3350 4950 3800
Connection ~ 4950 3800
Wire Wire Line
	4050 2600 4950 2600
Wire Wire Line
	4950 2600 4950 3050
Connection ~ 4950 2600
$Comp
L Jumper:SolderJumper_2_Open JP1
U 1 1 5CC4FCB3
P 2450 3200
F 0 "JP1" H 2250 3400 50  0000 C CNN
F 1 "OC Direct" H 2350 3300 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_RoundedPad1.0x1.5mm" H 2450 3200 50  0001 C CNN
F 3 "~" H 2450 3200 50  0001 C CNN
	1    2450 3200
	1    0    0    -1  
$EndComp
Wire Wire Line
	2150 3200 2300 3200
Connection ~ 2150 3200
Wire Wire Line
	2150 3200 2150 3400
Wire Wire Line
	2600 3200 2850 3200
Connection ~ 2850 3200
$Comp
L Jumper:SolderJumper_2_Open JP2
U 1 1 5CC51175
P 2450 3850
F 0 "JP2" H 2250 4050 50  0000 C CNN
F 1 "OC Direct" H 2350 3950 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_RoundedPad1.0x1.5mm" H 2450 3850 50  0001 C CNN
F 3 "~" H 2450 3850 50  0001 C CNN
	1    2450 3850
	1    0    0    -1  
$EndComp
Wire Wire Line
	2050 3850 2300 3850
Connection ~ 2050 3850
Wire Wire Line
	2050 3850 2050 4050
Wire Wire Line
	2600 3850 2850 3850
Connection ~ 2850 3850
$Comp
L Connector_Generic:Conn_01x05 J4
U 1 1 5CC5587F
P 1300 3650
F 0 "J4" H 1300 3350 50  0000 C CNN
F 1 "Programming" V 1450 3650 50  0000 C CNN
F 2 "Connector_PinHeader_2.54mm:PinHeader_1x05_P2.54mm_Vertical" H 1300 3650 50  0001 C CNN
F 3 "~" H 1300 3650 50  0001 C CNN
	1    1300 3650
	-1   0    0    1   
$EndComp
$Comp
L power:GND #PWR0113
U 1 1 5CC59146
P 1500 3850
F 0 "#PWR0113" H 1500 3600 50  0001 C CNN
F 1 "GND" H 1505 3677 50  0000 C CNN
F 2 "" H 1500 3850 50  0001 C CNN
F 3 "" H 1500 3850 50  0001 C CNN
	1    1500 3850
	1    0    0    -1  
$EndComp
Text Label 3450 3400 3    50   ~ 0
Reset
Text Label 1600 3450 0    50   ~ 0
Reset
Wire Wire Line
	1500 3450 1600 3450
Text Label 1600 3750 0    50   ~ 0
SCL_SCK
Text Label 1600 3650 0    50   ~ 0
MISO
Text Label 1600 3550 0    50   ~ 0
SDA_MOSI
Wire Wire Line
	1500 3550 1600 3550
Wire Wire Line
	1500 3650 1600 3650
Wire Wire Line
	1500 3750 1600 3750
Text Label 3450 3000 2    50   ~ 0
MISO
Wire Wire Line
	3450 2450 3200 2450
Wire Wire Line
	3200 2450 3200 3000
Wire Wire Line
	3200 3000 3450 3000
Text Notes 3200 4300 0    50   ~ 0
Use OC Direct jumpers for\nindicators that already have OpenCollector\noutput, e.g. Mitutoyo
$Comp
L power:PWR_FLAG #FLG0101
U 1 1 5CC6966C
P 1500 2650
F 0 "#FLG0101" H 1500 2725 50  0001 C CNN
F 1 "PWR_FLAG" V 1500 2778 50  0001 L CNN
F 2 "" H 1500 2650 50  0001 C CNN
F 3 "~" H 1500 2650 50  0001 C CNN
	1    1500 2650
	0    1    1    0   
$EndComp
Connection ~ 1500 2650
$Comp
L power:PWR_FLAG #FLG0102
U 1 1 5CC6AE51
P 1500 2850
F 0 "#FLG0102" H 1500 2925 50  0001 C CNN
F 1 "PWR_FLAG" V 1500 2978 50  0001 L CNN
F 2 "" H 1500 2850 50  0001 C CNN
F 3 "~" H 1500 2850 50  0001 C CNN
	1    1500 2850
	0    1    1    0   
$EndComp
Connection ~ 1500 2850
Wire Wire Line
	1500 2750 1750 2750
Wire Wire Line
	1950 2750 1950 2400
$Comp
L power:+1V5 #PWR0110
U 1 1 5CC6CBF4
P 1750 2750
F 0 "#PWR0110" H 1750 2600 50  0001 C CNN
F 1 "+1V5" H 1765 2923 50  0000 C CNN
F 2 "" H 1750 2750 50  0001 C CNN
F 3 "" H 1750 2750 50  0001 C CNN
	1    1750 2750
	1    0    0    -1  
$EndComp
Connection ~ 1750 2750
Wire Wire Line
	1750 2750 1950 2750
$Comp
L power:PWR_FLAG #FLG0103
U 1 1 5CC6D3C9
P 1750 2750
F 0 "#FLG0103" H 1750 2825 50  0001 C CNN
F 1 "PWR_FLAG" V 1750 2878 50  0001 L CNN
F 2 "" H 1750 2750 50  0001 C CNN
F 3 "~" H 1750 2750 50  0001 C CNN
	1    1750 2750
	-1   0    0    1   
$EndComp
Wire Wire Line
	2600 2400 2600 2900
Wire Wire Line
	2600 2900 3450 2900
Wire Wire Line
	2700 2400 2700 3100
Wire Wire Line
	2700 3100 3450 3100
Text Label 2150 2900 0    50   ~ 0
Data
Text Label 2050 2900 2    50   ~ 0
Clock
$EndSCHEMATC
