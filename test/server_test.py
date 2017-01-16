#!/usr/bin/python

import os, os.path, sys, subprocess, argparse, sched, time
from subprocess import Popen, PIPE, CREATE_NEW_CONSOLE

if subprocess.mswindows:
  executable_name = 'sserver.exe'
else:
  executable_name = 'sserver'
  
parser = argparse.ArgumentParser(description='Client-Server test...')
parser.add_argument('-s', help='run with server', action='store_true', default=False, dest='server_flag')
parser.add_argument('-n', type=int, help='number of clients to run', default=2, dest='number_of_clients')
parser.add_argument('-e', help='executable name', default=executable_name, dest='executable_name')
parser.add_argument('-t', help='test execution time', type=int, default=5, dest='execution_time')
args = parser.parse_args(sys.argv[1:])

print 's =', args.server_flag, 'n = ', args.number_of_clients, 'e =', args.executable_name, 't = ', args.execution_time

t0 = time.time()

def runServer():
  Popen([args.executable_name, '-s'], creationflags=CREATE_NEW_CONSOLE)
  
def runClients():
  for i in xrange(0, args.number_of_clients):
    Popen([args.executable_name], creationflags=CREATE_NEW_CONSOLE)
    
def stopAll():
  Popen([args.executable_name, '-q'], creationflags=CREATE_NEW_CONSOLE)
  print 'test execution stopped'
  print 'execution time = ', time.time() - t0

if args.server_flag:
  runServer()

time.sleep(1)
  
runClients()

s = sched.scheduler(time.time, time.sleep)
s.enter(args.execution_time, 1, stopAll, ())
print 'test execution started...'
s.run()

