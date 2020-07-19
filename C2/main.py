from api import *
from client import *
from dns import *

if __name__ == "__main__":
    # app run in thread
    thread = threading.Thread(target=start_api)
    thread.daemon = True
    thread.start()

    start_dns()
