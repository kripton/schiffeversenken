import multiprocessing
import SVClient
from QueueManager import QueueManager
#from ProcessQueue import ProcessQueue


class SVChandler:
    def __init__(self):
        self.connections = {}
        self.queueCounter = 0

    def __del__(self):
        for conn in self.connections.keys():
            ##proc = getProcess(conn)
            #if proc.is_alive():
                #self.send(conn, 'bye', 'system shutdown')
            #proc.join()
            #self.getQueueManager(conn).shutdown()
            self.removeClient(conn)
            conn.close()

    def getQueueManager(self, conn):
        return self.connections[conn][0]

    def getQueuePath(self, conn):
        return self.connections[conn][1]

    def getQueue(self, conn):
        return self.connections[conn][2]

    def getProcess(self, conn):
        return self.connections[conn][3]

    def generateQueueNumber(self):
        self.queueCounter += 1
        return self.queueCounter

    def addClient(self, conn):
        queuepath = '/tmp/SVCqueue' + str(self.generateQueueNumber())

        m = QueueManager(address=(queuepath), authkey='gnarf')
        m.start()
        queue = m.get_queue()

        proc = multiprocessing.Process( \
            target=SVClient.processWorker, \
                args=(queuepath,conn,))
        proc.start()
        
        self.connections[conn] = (m, queuepath, queue, proc)

    def removeClient(self, conn):
        proc = self.getProcess(conn)
        if proc.is_alive():
            proc.terminate()
        self.getProcess(conn).join()
        self.getQueueManager(conn).shutdown()
        del self.connections[conn]

    def getSocketList(self):
        return self.connections.keys()

    def send(self, conn, msgtag, text, obj=None):
        self.getQueue(conn).put((msgtag, text, obj))

    def cycle(self, SVS):
        for conn in self.connections.keys():
            proc = self.getProcess(conn)
            queuepath = self.getQueuePath(conn)
            if not proc.is_alive():
                SVS.send('procDead', 'SVClient proc died', queuepath)
                proc.join()
                self.removeClient(conn)
                conn.send('bye\n')
                conn.close()
