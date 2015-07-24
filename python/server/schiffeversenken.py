import sys
import socket
import select
import SVShandler
import SVChandler
from SVStaticData import SVStaticData


if __name__ == '__main__':
    SVS = SVShandler.SVShandler()
    SVC = SVChandler.SVChandler()

    SVS.send('init', 'Welcome to hell', None)
    
    port = SVStaticData.portNumber
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    keeptrying = True
    while keeptrying:
        try:
            keeptrying = False
            server.bind(("", port))
        except: #assuming port wasn't free
            port +=1
            if port < 24000:
                keeptrying = True
    if port < 24000:
        server.listen(1)
    else:
        print 'Error: could not bind port. Shutdown.'
        sys.exit(1)
    print('Server listening on port ' + str(port))

    try:
        while True:
            readConns, writeConns, failConns = select.select( \
                [server] + SVC.getSocketList(), [], [], 1)

            for conn in readConns:
                if conn is server:
                    newconn, addr = server.accept()
                    SVC.addClient(newconn)
                    SVS.addClient(SVC.getQueuePath(newconn))

                else:
                    data = conn.recv(1024)

                    if not data:
                        SVS.send('peerQuit', 'Peer disconnected', \
                            SVC.getQueuePath(conn))
                        SVC.removeClient(conn)
                        conn.close()

                    else:
                        SVC.send(conn, 'sock', data, None)

            SVC.cycle(SVS)
            if not SVS.cycle(): break

    except KeyboardInterrupt: pass

    finally:
        print 'This is the end... cleaning up'
        del SVS
        del SVC
        server.close()
