import socket 
from threading import Thread 
from SocketServer import ThreadingMixIn 
import time
import threading
import json, sys
import logging
import random
import csv
import copy

######################Constants######################

NEWCLIENT = 'NewClient'
NEWCOUNTEREX = 'NewCounterExample'
LOCALHOST = '127.0.0.1'
######################################################

class RamseyServer():
    def __init__(self, svrId):
        self.svrId = svrId
        self.bestCounterVal = 0
        self.clients = {}
        self.counterExLock = threading.Lock()
        self.clientLock = threading.Lock()

        self.loadConfig()
        self.setBestCounterVal()
        self.getActiveClients()

        svrInfo= {'svr_name':self.svrId.upper()}
        self.logger = self.logFormatter(svrInfo)
        self.startServer()


    def loadConfig(self):
    	with open('config.json') as config_file:    
    		self.config = json.load(config_file)


    def setBestCounterVal(self):
        '''Read from db and update in best counter example value found so far'''
        pass


    def getActiveClients(self):
    	'''Read from DB the set of active clients'''
    	pass


    def logFormatter(self, svrInfo):
        '''Logging info'''
        logger = logging.getLogger()
        logger.setLevel(logging.DEBUG)
        ch = logging.StreamHandler()
        formatter = logging.Formatter('%(svr_name)s: %(message)s')
        ch.setFormatter(formatter)
        logger.addHandler(ch)
        logger = logging.LoggerAdapter(logger, svrInfo)
        return logger


    def handleNewCounterExample(self, msg):
        self.counterExLock.acquire()
        try:
            currNum = len(msg['matrix'])
            self.bestCounterVal = max(self.bestCounterVal, currNum)
            logMsg = 'Best counter example updated to: %d' %(self.bestCounterVal)
            self.logger.debug(logMsg)
        finally:
            self.counterExLock.release()

        #write to DB here
        self.notifyAllClients()


    def notifyAllClients(self):
        clientIps = self.clients.keys()
        for ip in clientIps:
            port = self.clients[ip]
            msg = str(self.bestCounterVal)
            ip = LOCALHOST
            self.sendTcpMsg(ip, port, msg)


    def handleNewClient(self, msg):
    	clIp, clPort = self.extractClientIpPortInfo(msg)
        self.clientLock.acquire()
        try:
            self.clients[clIp] = clPort
        finally:
            self.clientLock.release()
    	
    	logMsg = 'Clients are updated to: %s' %(repr(self.clients))
        self.logger.debug(logMsg)
    	#write new client to db

    	msg = str(self.bestCounterVal)
    	#sending to localhost for now
    	clIp = LOCALHOST
    	self.sendTcpMsg(clIp, clPort, msg)



    ############################# Misc methods #############################

    def sendTcpMsg(self, ip, port, msg, display=True):
        try:
            tcpClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tcpClient.settimeout(1)
            tcpClient.connect((ip, port))
            tcpClient.send(json.dumps(msg))
            if display:
                logMsg = 'Sent message to: (%s, %d). Message is: %s' %(ip, port, msg)
                self.logger.debug(logMsg)

        except Exception as e:
            '''When a site is down, tcp connect fails and raises exception; catching and 
            if it's a client, removing it from active client list'''
            if ip in self.clients:
             	self.clients.pop(ip)

            #delete from DB


    def getServerIpPort(self, svrId):
        '''Get ip and port on which server is listening from config'''
        return self.config['servers'][svrId][0], self.config['servers'][svrId][1]


    def extractClientIpPortInfo(self, msg):
    	'''Extract IP and port on which client is listening'''
    	return msg['ip'], msg['port']

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
            data = conn.recv(2048)
            while data:
                recvMsg += data
                data = conn.recv(2048)
            
            print 'Received message from: (%s:%d). Message is: %s' %(self.ip, self.port, recvMsg)
            msgType, msg = self.parseRecvMsg(recvMsg)
    
            if msgType == NEWCLIENT:
                self.srvr.handleNewClient(msg)
            elif msgType == NEWCOUNTEREX:
                self.srvr.handleNewCounterExample(msg)

            conn.close() 
            sys.exit()


        def parseRecvMsg(self, recvMsg):
            recvMsg = json.loads(recvMsg)
            msgType, msg = recvMsg.keys()[0], recvMsg.values()[0]
            return msgType, msg


    def startServer(self):
        ip, port = self.getServerIpPort(self.svrId)

        tcpServer = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
        tcpServer.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) 
        tcpServer.bind((ip, port))

        print 'Server ready to listen on (%s:%d)' %(ip, port)
        while True: 
            tcpServer.listen(4) 
            (conn, (cliIP,cliPort)) = tcpServer.accept()
            newthread = self.ConnectionThread(conn, cliIP, cliPort, self) 
            newthread.start()
    
 
svrId = sys.argv[1]
ramseySrvr = RamseyServer(svrId)