from time import sleep
from subprocess import call

while True:
    try:
        call(['git', 'pull'])
        call(['python', 'schiffeversenken.py'])
        sleep(5)

    except KeyboardInterrupt: break
