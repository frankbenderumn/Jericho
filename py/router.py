from threading import Thread
from ctypes import *
sofile = "./build/lib/prizm.so"
Server = CDLL(sofile)

# class Encrypt:
#     def hash(toAuth = ""):
#         print("Hashing")

# class Session:
#     id = Encrypt.hash()
#     def __init__(self, toAuth):
#         print("Starting session")
#         Encrypt.hash()


# class RoutingEngine:
#     'Common base class for all employees'
#     empCount = 0

#     def __init__(self, session):
#         self.session = Session.authenticate(session)
#         RoutingEngine.empCount += 1

#     def parseUrl(self, url):
#         print("Total Employee %d", RoutingEngine.empCount)

#     def printUrl(self, url):
#         print("Url: %s", url)

print("Dammit");


# Server.pySend(c_char_p("Single threaded?\n"))
def call():
    print("Demo")

def run():
    Server.main.argtypes = c_int,POINTER(c_char_p)
    args = (c_char_p * 3)(b'5',b'8081',b'./frontend')
    Server.main(len(args),args)

run()

# t1 = Thread(target=run, daemon=True)
# t2 = Thread(target=call, daemon=True)

# t1.start()

# t2.start()
# # t1.join()
# t2.join()

# print("OH BOY NOW WE NEED EVEN MORE THREADS");

# # Server.pySend(c_char_p("Single threaded?\n"))
# # Prizm.wonder()
# # Prizm.main()
# t1.join()
