import multiprocessing
import SVService
from QueueManager import QueueManager


class SVShandler:
    def __init__(self):
        self.qm = QueueManager(address=('/tmp/SVSqueue'), authkey='gnarf')
        self.qm.start()
        self.queue = self.qm.get_queue()

        self.proc = multiprocessing.Process( \
            target=SVService.processWorker, \
            args=[])
        self.proc.start()

    def __del__(self):
        if self.proc.is_alive():
            self.send('bye', 'System shutdown', None)
        self.proc.join()
        self.qm.shutdown()

    def addClient(self, queuepath):
        self.send('addClient', 'new Connection', queuepath)

    def removeClient(self, queue):
        self.send('removeClient', 'Peer lost', queue)

    def send(self, msgtag, text, obj=None):
        self.queue.put((msgtag, text, obj))

    def getQueue(self):
        return self.queue

    def cycle(self):
        if self.proc.is_alive(): return True
        else:
            self.proc.join()
            self.qm.shutdown()
            return False
