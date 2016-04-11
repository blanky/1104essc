# Code from Chapter 10 of Machine Learning: An Algorithmic Perspective
# by Stephen Marsland (http://seat.massey.ac.nz/personal/s.r.marsland/MLBook.html)

# You are free to use, change, or redistribute the code in any way you wish for
# non-commercial purposes, but please maintain the name of the original author.
# This code comes with no warranty of any kind.

# Adapted by Burak Bayramli, 2010


import pylab
import numpy
import math

AX = 0
AY = 0
BX = 0
BY = 12
CX = 10
CY = 0
DX = 10
DY = 12

Xcoord = 6
Ycoord = 10

AR = math.floor(math.sqrt(Xcoord ** 2 + Ycoord ** 2))
BR = math.floor(math.sqrt(Xcoord ** 2 + (12 - Ycoord) ** 2))
CR = math.floor(math.sqrt((10 - Xcoord) ** 2 + Ycoord **2))
DR = math.floor(math.sqrt((10 - Xcoord) ** 2 + (12 - Ycoord) ** 2))

print "AR:" + repr(AR) +" BR:" + repr(BR)  +" CR:" + repr(CR) 

matA = numpy.matrix([[2*(DX - AX), 2*(DY - AY)],[2*(DX - BX), 2*(DY - BY)], [2*(DX - CX), 2*(DY - CY)]])

class Kalman:
    def __init__(self, x_init, cov_init, meas_err, proc_err):
        self.ndim = len(x_init)
        self.A = numpy.array([(1, 0, dt, 0), (0, 1, 0, dt), (0, 0, 1, 0), (0, 0, 0, 1)]);
        self.H = numpy.array([(1, 0, 0, 0), (0, 1, 0, 0)])
        self.x_hat =  x_init
        self.cov = cov_init
        self.Q_k = numpy.eye(ndim)*proc_err
        self.R = numpy.eye(len(self.H))*meas_err

    def update(self, obs):

        # Make prediction
        self.x_hat_est = numpy.dot(self.A,self.x_hat)
        self.cov_est = numpy.dot(self.A,numpy.dot(self.cov,numpy.transpose(self.A))) + self.Q_k

        # Update estimate
        self.error_x = obs - numpy.dot(self.H,self.x_hat_est)
        self.error_cov = numpy.dot(self.H,numpy.dot(self.cov_est,numpy.transpose(self.H))) + self.R
        self.K = numpy.dot(numpy.dot(self.cov_est,numpy.transpose(self.H)),numpy.linalg.inv(self.error_cov))
        self.x_hat = self.x_hat_est + numpy.dot(self.K,self.error_x)
        if ndim>1:
            self.cov = numpy.dot((numpy.eye(self.ndim) - numpy.dot(self.K,self.H)),self.cov_est)
        else:
            self.cov = (1-self.K)*self.cov_est 
            
if __name__ == "__main__":		
    ndim = 4
    ndim_obs = 2
    nsteps = 50
    xcoord = 5.0
    ycoord = 2.0
    vx = 0.5 #m.s
    vy = 1.0 #m/s
    dt = 1.0 #sec
    meas_error = 10.0 #m

    print matA
    print matA.T

    matB = numpy.matrix([[AR ** 2 - DR ** 2 - AX ** 2 - AY ** 2 + DX ** 2 +DY ** 2], [BR ** 2 - DR ** 2 - BX ** 2 - BY ** 2 + DX ** 2 + DY ** 2], [CR ** 2 - DR ** 2 - CX **2 - CY **2 + DX ** 2 +DY ** 2]])

    print matB

    matC = matA.T
    print matC
    matC = matC.dot(matA)
    print matC
    matC = matC.I
    print matC
    matC = matC.dot(matA.T)
    print matC
    matC = matC.dot(matB)
    print matC.item(0)
    print matC.item(1)
    #x = (numpy.linalg.inv((matA.T.dot(matA))) * matA.T) * matB



    pylab.plot(matC.item(0), matC.item(1), 'o')
    pylab.xlim(0, 10)
    pylab.ylim(0, 12)
    pylab.xticks(numpy.arange(0.5, 11.5, 1.0))
    pylab.yticks(numpy.arange(0.5, 13.5, 1.0))
    pylab.grid(b=True, which='major', color='k', linestyle='-')
    #pylab.show()


    #generate ground truth
    x_true = numpy.array( [(xcoord+i*vx,ycoord+i*vy,vx,vy) for i in range(nsteps)] )
    obs_err = numpy.random.normal(0,numpy.ones(ndim_obs)*meas_error,(nsteps,ndim_obs)) # observations 
    obs = x_true[:,0:2]+obs_err

    #init filter
    proc_error = 0.01;
    init_error = 150.0;
    x_init = numpy.array( [xcoord+init_error, ycoord+init_error, vx, vy] ) #introduced initial xcoord error of 2m 
    cov_init=init_error*numpy.eye(ndim)

    #filtering
    x_hat = numpy.zeros((ndim,nsteps))
    k = Kalman(x_init, cov_init, meas_error, proc_error)    
    for t in range(nsteps):
        k.update(obs[t])
        x_hat[:,t]=k.x_hat

    #plot the results
    pylab.figure()
    pylab.scatter(x_true[:,0],x_true[:,1],s=1, c='g',marker='o', label="true location")
    pylab.scatter(obs[:,0],   obs[:,1],   s=10,c='k',marker='+', label="measured loc")
    pylab.scatter(x_hat[0], x_hat[1], s=10,c='red',marker='s', edgecolors='red', label="estimated loc")
    pylab.xlabel('x [m]')
    pylab.ylabel('y [m]')

    pylab.show()


