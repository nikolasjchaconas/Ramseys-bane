import socket 
from threading import Thread 
import time
import threading
import json, sys
import logging
import random
from db_store import DBStore
from subprocess import call
import os
import copy

######################Constants######################

LOCALHOST = '127.0.0.1'

BUFFER_SIZE = 1024*1024
STATS_DIR = 'statistics'
STATS_FILE = 'stats.log'


######################################################

class StatsServer():
    def __init__(self, svrId):
        self.svrId = svrId
        self.stats = 0
        self.lock = threading.Lock()
        svrInfo= {'svr_name':self.svrId.upper()}
        self.logger = self.logFormatter(svrInfo)

        self.loadConfig()
        self.startServer()


    def loadConfig(self):
        with open('config.json') as config_file:    
            self.config = json.load(config_file)


    def logFormatter(self, svrInfo):
        '''Logging info'''
        logger = logging.getLogger()
        logger.setLevel(logging.DEBUG)
        logging.basicConfig(filename='stats.log',level=logging.DEBUG)
        ch = logging.StreamHandler()
        formatter = logging.Formatter('%(svr_name)s: %(message)s')
        ch.setFormatter(formatter)
        logger.addHandler(ch)
        logger = logging.LoggerAdapter(logger, svrInfo)
        return logger


    def handleNewStatsInfo(self, conn, msg):

        newStats = int(msg.split(':')[1])
        self.lock.acquire()
        try:
            self.stats += newStats
            self.appendStatsToFile()
        finally:
            self.lock.release()


    def getServerIpPort(self, svrId):
        '''Get ip and port on which server is listening from config'''
        return self.config['stats_servers'][svrId][0], self.config['stats_servers'][svrId][1]


    def appendStatsToFile(self):
        try:
            if not os.path.exists(STATS_DIR):
                os.makedirs(STATS_DIR)
            f= open(STATS_DIR + '/' + STATS_FILE, 'a+')
            f.write("Total cycles burned is: " + str(self.stats))
            f.close()
        except Exception as e:
            self.logger.debug(e)
            self.logger.debug('Some error while writing to file! Ignoring it!')


    def postOnSlack(self):
        
        try:
            cmd = 'curl -F file=@' + STATS_DIR + '/' + STATS_FILE
            cmd += ' -F channels=#counter_examples -F token=***REMOVED***'
            cmd += ' https://slack.com/api/files.upload'
            # cmd = "curl -X POST -H 'Content-type: application/json' --data '{\"text\":\""
            # cmd += str(self.bestCliqueCount) + "-" + self.bestGraph 
            # cmd += "\"}' ***REMOVED***"
            
            os.system(cmd)
        except Exception as e:
            self.logger.debug(e)
            self.logger.debug('Some error while posting on Slack! Ignoring it!')
        

####################################################################################### 

    # Multithreaded Python server : TCP Server Socket Thread Pool
    class ConnectionThread(Thread): 
     
        def __init__(self, conn, ip, port, srvr): 
            Thread.__init__(self) 
            self.ip = ip
            self.port = port
            self.conn = conn
            self.srvr = srvr


        def run(self): 
            
            conn, recvMsg = self.conn, ''
            data = conn.recv(BUFFER_SIZE)
            # datasize, _ = int(data.split(':')[0])
            # datasize += len(data.split(':')[0] + 1)

            # while len(data) < dataSize:
            #     data += conn.recv(BUFFER_SIZE)

            #self.srvr.logger.debug('Received message: %s, %s, %s' %(counterNum, cliqueCnt, index))

            self.srvr.handleNewStatsInfo(conn, data)

            time.sleep(2)
            conn.close()
            '''Kill the thread after use'''
            sys.exit()


    def startServer(self):
        ip, port = self.getServerIpPort(self.svrId)

        tcpServer = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
        tcpServer.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) 
        tcpServer.bind((ip, port))

        self.logger.debug('Server ready to listen on (%s:%d)' %(ip, port))
        while True: 
            tcpServer.listen(4) 
            (conn, (cliIP,cliPort)) = tcpServer.accept()
            newthread = self.ConnectionThread(conn, cliIP, cliPort, self) 
            newthread.start()

svrId = sys.argv[1]
ramseySrvr = StatsServer(svrId)
