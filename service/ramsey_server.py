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
from db_store import DBStore
from subprocess import call
import os

######################Constants######################

LOCALHOST = '127.0.0.1'
DB_NAME = 'ramseys_bane'
DB_USER = 'ramsey'
COUNTER_EX_TABLE = 'counter_examples'
BUFFER_SIZE = 1024*1024
COUNTER_EX_DIR = 'counter_examples'

######################################################

class RamseyServer():
    def __init__(self, svrId):
        self.svrId = svrId
        self.bestCounterVal = 0
        self.bestMatrix = ''
        self.counterExLock = threading.Lock()

        svrInfo= {'svr_name':self.svrId.upper()}
        self.logger = self.logFormatter(svrInfo)

        self.loadConfig()
        self.initDbConnection()
        self.setBestCounterVal()

        self.startServer()


    def loadConfig(self):
    	with open('config.json') as config_file:    
    		self.config = json.load(config_file)


    def initDbConnection(self):

        db_node = random.choice(self.config['db_nodes'])
        #db_node, _ = self.getServerIpPort(self.svrId)
        self.db = DBStore(DB_NAME, DB_USER, db_node)

        create_table = 'CREATE TABLE IF NOT EXISTS '+ COUNTER_EX_TABLE +' (number INT PRIMARY KEY, matrix VARCHAR)'
        self.db.create(create_table)


    def setBestCounterVal(self):
        '''Read from db and update in best counter example value found so far'''
        bestVal, bestMatrix = 0, ''
        get_statement = 'SELECT number, matrix FROM ' + COUNTER_EX_TABLE
        rows = self.db.get(get_statement)

        for num, matrix in rows:
            if num > bestVal:
                bestVal = num
                bestMatrix = matrix
                
        self.bestCounterVal, self.bestMatrix = bestVal, bestMatrix
        self.logger.debug('Loaded the best counter example number from db: %d' %self.bestCounterVal)


    def logFormatter(self, svrInfo):
        '''Logging info'''
        logger = logging.getLogger()
        logger.setLevel(logging.DEBUG)
        logging.basicConfig(filename='server.log',level=logging.DEBUG)
        ch = logging.StreamHandler()
        formatter = logging.Formatter('%(svr_name)s: %(message)s')
        ch.setFormatter(formatter)
        logger.addHandler(ch)
        logger = logging.LoggerAdapter(logger, svrInfo)
        return logger


    def handleNewCounterExample(self, conn, msg):
        writeToDB = False

        '''The first part of input string is the current counter example computed by the client'''
        parts = msg.split(':')
        try:
            currNum = int(parts[0])
            if len(parts) > 1:
                matrix = parts[1]
        except Exception as e:
            '''In case client didn't in right format; ignore that int conversion and set currNum to 0'''
            currNum = 0
        
        '''Acquire lock to update best counter example'''
        self.counterExLock.acquire()
        try:
            '''Even if something fails in this part of code, lock is always released'''
            if currNum > self.bestCounterVal:
                self.bestCounterVal = currNum
                self.bestMatrix = matrix
                writeToDB = True
                logMsg = 'Best counter example updated to: %d' %(self.bestCounterVal)
                self.logger.debug(logMsg)
        finally:
            self.counterExLock.release()

        '''write to DB if currNum is greater any previous value'''
        if writeToDB:
            insert_statement = 'INSERT INTO '+ COUNTER_EX_TABLE + ' (number, matrix) VALUES (%d, \'%s\')' % (self.bestCounterVal, matrix)
            self.db.insert(insert_statement)
            self.postOnSlack()

        self.replyToClient(conn)
        


    def replyToClient(self, conn):
        '''Making sure that always the latest counter example value is sent to the client'''
        self.counterExLock.acquire()
        try:
            reply = str(self.bestCounterVal) + ':' + self.bestMatrix
        finally:
            self.counterExLock.release()
        
        conn.send(reply)
        time.sleep(1)
        conn.close()


    def getServerIpPort(self, svrId):
        '''Get ip and port on which server is listening from config'''
        return self.config['servers'][svrId][0], self.config['servers'][svrId][1]



    def createExampleFile(self):
        try:
            if not os.path.exists(COUNTER_EX_DIR):
                os.makedirs(COUNTER_EX_DIR)
            f= open(COUNTER_EX_DIR + '/' + str(self.bestCounterVal) + '.txt','w+')
            f.write(self.bestMatrix)
            f.close()
        except Exception as e:
            self.logger.debug(e)
            self.logger.debug('Some error while writing to file! Ignoring it!')

        
    def postOnSlack(self):
        self.createExampleFile()
        try:
            cmd = 'curl -F file=@' + COUNTER_EX_DIR + '/' + str(self.bestCounterVal) + '.txt'
            cmd += ' -F channels=#counter_examples -F token=xoxp-168043439156-168044120404-175871744339-d470018d55873ffc1f59b0e8b1d42f17'
            cmd += ' https://slack.com/api/files.upload'
            # cmd = "curl -X POST -H 'Content-type: application/json' --data '{\"text\":\""
            # cmd += str(self.bestCounterVal) + "-" + self.bestMatrix 
            # cmd += "\"}' https://hooks.slack.com/services/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"
            
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

            currNum = data.split(':')[0]
            dataSize = int(currNum)*int(currNum) + len(currNum) + 1 
            
            while len(data) < dataSize:
                data += conn.recv(BUFFER_SIZE)

            self.srvr.logger.debug('Received message from: (%s:%d). Counter example number received is %s' %(self.ip, self.port, currNum))
            
            self.srvr.handleNewCounterExample(conn, data)
 
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
ramseySrvr = RamseyServer(svrId)