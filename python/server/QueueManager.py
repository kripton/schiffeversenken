import Queue
from multiprocessing.managers import BaseManager

class QueueManager(BaseManager): pass
queue = Queue.Queue()

QueueManager.register('get_queue', callable=lambda:queue)
