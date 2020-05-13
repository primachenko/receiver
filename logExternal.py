#!/usr/bin/env python
from PIL import ImageFont, ImageDraw
from luma.core.interface.serial import i2c
from luma.core.render import canvas
from luma.oled.device import ssd1306, ssd1325, ssd1331, sh1106
import time
import sys
import textwrap

serial = i2c(port=1, address=0x3C)

device = ssd1306(serial, width=128, height=32, rotate=0)
font = ImageFont.truetype("font.ttf", 16)

str1=""
str2=""
str3=""

while 1:
	str3=str2
	str2=str1
	str1 = sys.stdin.readline()
	if (-1 != str1.find("dieLogEx")):
		break
	if (len(str1) > 42):
		fspace = str1.rfind(" ", 0, 21)
		sscape = str1.rfind(" ", fspace, fspace+21)
		str3=str1[sscape+1:]
		str2=str1[fspace+1:sscape+1]
		str1=str1[0:fspace+1]
	if (len(str1) > 21):
		fspace = str1.rfind(" ", 0, 21)
		str3=str2
		str2=str1[fspace+1:]
		str1=str1[0:fspace+1]
	with canvas(device) as draw:
		draw.text((0, 20), text=str(str1), fill="white", font=font)
		draw.text((0, 10), text=str(str2), fill="white", font=font)
		draw.text((0, 0), text=str(str3), fill="white", font=font)
