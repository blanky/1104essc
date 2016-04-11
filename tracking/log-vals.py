#!/usr/bin/python

import math
import numpy as np
import pylab
import serial
import threading
import time
import sys
import re

Aaddr= "07b51d03"

Baddr= "0799a884"

Caddr= "07b54c81"

Daddr= "0798f385"

AX = 0
AY = 0

BX = 0
BY = 12

CX = 10
CY = 0

DX = 10
DY = 12

AU = 0
AR = 0
BU = 0
BR = 0
CU = 0
CR = 0
DU = 0
DR = 0

matA = np.matrix([[2*(DX - AX), 2*(DY - AY)],[2*(DX - BX), 2*(DY - BY)], [2*(DX - CX), 2*(DY - CY)]])

# weighting value between 0 and 2
# more than one goes towards least squares
WEIGHTING = 1.3

current_rssi = -30

# measured inputs for least squares
x = np.array([10.0, 20.0, 30.0, 40.0, 50.0, 60.0])
y = np.array([-27.0, -31.0, -33.0, -36.0, -40.0, -41.0])
A = np.vstack([x, np.ones(len(x))]).T
mA, cA = np.linalg.lstsq(A, y)[0]

# measured inputs for least squares
x = np.array([10.0, 20.0, 30.0, 40.0, 50.0, 60.0])
y = np.array([-24.0, -27.0, -33.0, -35.0, -39.0, -39.0])
A = np.vstack([x, np.ones(len(x))]).T
mB, cB = np.linalg.lstsq(A, y)[0]

# measured inputs for least squares
x = np.array([10.0, 20.0, 30.0, 40.0, 50.0, 60.0])
y = np.array([-33.0, -38.0, -41.0, -43.0, -50.0, -50.0])
A = np.vstack([x, np.ones(len(x))]).T
mC, cC = np.linalg.lstsq(A, y)[0]

# measured inputs for least squares
x = np.array([10.0, 20.0, 30.0, 40.0, 50.0, 60.0])
y = np.array([-22.0, -27.0, -31.0, -39.0, -44.0, -51.0])
A = np.vstack([x, np.ones(len(x))]).T
mD, cD = np.linalg.lstsq(A, y)[0]



def leastsquares(rssi, c, m):
	return (rssi - c)/m
	
def freespaceeq(rssi):
	#
	#      sqrt(FSPL)*lambda
	# d = -------------------
	#           4*pi
	diffdb = 5.0 - rssi;
	power = round(math.pow(10.0, diffdb/10.0))
	return (math.sqrt(power)/(4*math.pi))

# weighing calibration between least squares and inverse square
def get_distance(rssi):
	val1 = leastsquares(rssi)
	val2 = freespaceeq(rssi)
	return (WEIGHTING*val1 + (2-WEIGHTING)*val2)/2

# not yet implemented, but will change m, c
def calibration():
	global m, c
	temp = []
	for i in x:
		print "Please place sensortags " + str(i) + "cm apart"
		raw_input("Press Enter to continue...")
		temp.append(current_rssi)
		print temp
	# todo, change m, c
	z = np.array(temp)
	m, c = np.linalg.lstsq(A, z)[0]


# serial read loop
def read_loop():
	global current_rssi
	global AR
	global BR
	global CR
	global DR
	output = ''
	while read_data:
		try:
			data = s.read()
			if(len(data) > 0):
				output += data
				if(output[-1] == '\n'):
					#output = output.replace('\n', '')
					print output
					#p = re.compile(':\s')
					var = output.split(':')
					print "var" + str(var)
					#p = re.compile(',\s')
					var2 = var[1].split(',')
					print "var2" + str(var2)

					if var[0] == Aaddr:
						#AU = int(var2[0])
						AR = leastsquares(int(var2[1]), cA, mA)/(sqrt(2)*7)
					elif var[0] == Baddr:
						#BU = int(var2[0])
						BR = leastsquares(int(var2[1]), cB, mB)/(sqrt(2)*7)
					elif var[0] == Caddr:
						#CU = int(var2[0])
						CR = leastsquares(int(var2[1]), cC, mC)/(sqrt(2)*7)
					elif var[0] == Daddr:
						#DU = int(var2[0])
						DR =leastsquares(int(var2[1]), cD, mD)/(sqrt(2)*7)
					output = ''
		except Exception, e:
			print "Exception", e
	print "closing serial port"
	s.close()


# init serial port
s = serial.Serial(port="/dev/ttyACM0", baudrate=115200)

# start reading thread
read_data = True
t1 = threading.Thread(target=read_loop, args=())
t1.start()

#calibration
if(len(sys.argv) == 2 and sys.argv[1] == '-c'):
	print "Starting calibration sequence."
	print "Please follow the on-screen prompts."
	calibration()
else:
	print "Using default calibration."
	print "Accuracy not gauranteed."

while True:
	try:
		matB = np.matrix([[AR ** 2 - DR ** 2 - AX ** 2 - AY ** 2 + DX ** 2 +DY ** 2], [BR ** 2 - DR ** 2 - BX ** 2 - BY ** 2 + DX ** 2 + DY ** 2], [CR ** 2 - DR ** 2 - CX **2 - CY **2 + DX ** 2 +DY ** 2]])
		
		matC = matA.T
		matC = matC.dot(matA)
		matC = matC.I
		matC = matC.dot(matA.T)
		matC = matC.dot(matB)

		print str(matC.item(0))
		print str(matC.item(1))
		# pylab.plot(matC.item(0), matC.item(1), 'o')
  #   	pylab.xlim(0, 10)
  #   	pylab.ylim(0, 12)
  #   	pylab.xticks(numpy.arange(0.5, 11.5, 1.0))
  #   	pylab.yticks(numpy.arange(0.5, 13.5, 1.0))
  #   	pylab.grid(b=True, which='major', color='k', linestyle='-')
  #   	pylab.show()
		time.sleep(5)

	except KeyboardInterrupt:
		print "Shutdown"
		read_data = False
		break

