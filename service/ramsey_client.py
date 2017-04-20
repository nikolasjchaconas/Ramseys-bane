import socket
import time
import logging
import json, sys
import random
import threading
from threading import Thread
logging.basicConfig(filename='client.log',level=logging.DEBUG)


clientId = sys.argv[1]
BUFFER_SIZE = 2000 
CLIRES= 'ClientResponse'
SHOWRES= 'ShowResponse'
TICKETREQ = 'TicketRequest'
CONFIGCHANGE = 'ConfigChangeRequest'

CONFIGFILE = 'config.json'

MATRIX = [011,001,000]

class RamseyClient():

    def __init__(self, clientId):
        self.clientId = clientId
        self.readAndApplyConfig()
        thread = Thread(target = self.sendNewClientInfo)
        thread.start()
        self.startListening()


    def readAndApplyConfig(self):
        with open(CONFIGFILE) as config_file:    
            self.config = json.load(config_file)


    def getServerIpPort(self, svrId):
        '''Get ip and port on which server is listening from config'''
        return self.config['servers'][svrId][0], self.config['servers'][svrId][1]


    def formNewClientMsg(self):
        msg = { 
        'NewClient': {
            'ip':'10.102.126.'+ str(random.randint(1,100)),
            'port':self.config['clients'][self.clientId][1]
            }
        }
        return msg


    def formNewCounterExampleMsg(self):
        msg = { 
        'NewCounterExample': {
             'matrix':str(MATRIX)
            }
        }
        return msg
    

    def startListening(self):
        '''Start listening for server response'''
        ip, port = self.config["clients"][self.clientId][0], self.config["clients"][self.clientId][1]

        tcpClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
        tcpClient.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1) 
        tcpClient.bind((ip, port))

        while True:
            tcpClient.listen(4) 
            (conn, (cliIP,cliPort)) = tcpClient.accept()
            
            msg = conn.recv(BUFFER_SIZE)
            print msg

            time.sleep(5)
            self.sendNewCounterExToServer()



    def sendNewCounterExToServer(self):
        '''Form the client info message and send a tcp request to randomly chosen server'''
            
        randomIdx =  random.randint(1, len(self.config['servers']))
        svrId = 'svr' + str(randomIdx)
        
        ip, port = self.getServerIpPort(svrId)
        reqMsg = self.formNewCounterExampleMsg()
        reqMsg = json.dumps(reqMsg)
        try:
            tcpClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tcpClient.connect((ip, port))
            tcpClient.send(reqMsg)
            time.sleep(0.5)
            tcpClient.close()
        except Exception as e:
            '''When a site is down, tcp connect fails and raises exception; catching and 
            ignoring it as we don't care about sites that are down'''
            pass


    def sendClientInfoToServer(self):
        '''Form the client info message and send a tcp request to randomly chosen server'''
            
        randomIdx =  random.randint(1, len(self.config['servers']))
        svrId = 'svr'+str(randomIdx)
        
        ip, port = self.getServerIpPort(svrId)
        reqMsg = self.formNewClientMsg()
        reqMsg = json.dumps(reqMsg)
        try:
            tcpClient = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            tcpClient.connect((ip, port))
            tcpClient.send(reqMsg)
            time.sleep(0.5)
            tcpClient.close()
        except Exception as e:
            '''When a site is down, tcp connect fails and raises exception; catching and 
            ignoring it as we don't care about sites that are down'''
            pass
    

    def sendNewClientInfo(self): 
        '''As soon as client comes up, send info to server'''
        self.sendClientInfoToServer()
        
        
client = RamseyClient(clientId)

