




#recepy = [["Einmaischen",20,0],["Einmaischen2",30,30]]
#print(recepy)
#print(len(recepy))

#recepy.append(["Einmaischen3",40,40])
#print(recepy)
#print(len(recepy))

#print("write")
#with open('recepy.txt', 'w') as f:
#    for item in recepy:
#        f.write("%s," % item[0])
#        f.write("%s," % item[1])
#        f.write("%s\n" % item[1])

#recepy = []

#print("read")
#with open('recepy.txt', 'r') as f:
#    for line in f:
#        value = line.split(",")
#        recepy.append([value[0],int(value[1]),int(value[2])])


#print(recepy)
#print(recepy[0])
#print(recepy[1][1])

    #f.readline().rstrip(",")
    #myNames = [line.strip() for line in f]


#print(myNames)
#print(myNames[0])


#with open('recepy.json', 'w') as output:
#    output.write(str(recepy))

#with open('recepy.json') as f:
#  myNames = [line.strip() for line in f]
#  print(myNames)

#print("load from file")
#print(myNames)
#print(len(myNames))
#print(myNames[0],[0],[0])




#recepy.pop(2)
#print(recepy)
#print(len(recepy))

#with open('recepy.json', 'w') as jsonfile:
#    json.dump(data, jsonfile)
#with open('recepy.json') as f:
#  data = json.load(f)
#print(data)

#print(b)


import os
from threading import Thread
import threading
import time
import random


class Recepy:
    def __init__(self):
        self.filename = os.path.dirname(__file__)+ "/recepy.txt"
        self.recepy = []
        self.loadRecepy()
        self.printRecepy()
        self.numOfItems = len(self.recepy)

    def loadRecepy(self):
        with open(self.filename, 'r') as f:
            for line in f:
                value = line.split(",")
                self.recepy.append([value[0],int(value[1]),int(value[2])])

    def storeRecepy(self):
        with open(self.filename, 'w') as f:
            for item in self.recepy:
                f.write("%s," % item[0])
                f.write("%s," % item[1])
                f.write("%s\n" % item[1])
   
    def addItem(self, topic, temp, time):
        self.recepy.append([topic, temp, time])

    def getItem(self, index):
        return self.recepy[index]

    def setItem(self, index, topic, temp, time):
        self.recepy[index] = ([topic, temp, time])

    def deleteLastItem(self):
        self.recepy.pop(self.numOfItems-1)

    def deleteItems(self):
        self.recepy.clear()

    def printRecepy(self):
        print(self.recepy)


class Brewing:
    def __init__(self, recepy:Recepy):
        self.recepy = recepy
        self.pause = False
       

    def start(self):
        for i in range(0,self.recepy.numOfItems):
            item = self.recepy.getItem(i)
            print(item)
            temp1 = 0
            print("Heating until target temp is reached")
            while temp1 < item[1]:
                temp1 = temp1 + 1 
                print(temp1)

            print("\n\nStart Timer")
            print(item[2])


class TempSensors(threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)
        self.temp1 = 0
        self.temp2 = 0
        self.temp3 = 0
        self.status = True
        print("Start Thread")
        
    def run(self):
        while self.status:
            self.temp1 = random.randint(20, 90)
            self.temp2 = random.randint(20, 90)
            self.temp3 = random.randint(20, 90)
            time.sleep(2)
                 
        

if __name__ == "__main__":

    R = Recepy()
    R.printRecepy()
    print(R.numOfItems)

    B = Brewing(R)
    B.start()

    T = TempSensors()
    T.start()
    
    print(T.temp1)
    print(T.temp2)
    print(T.temp3)
    time.sleep(3)
    print(T.temp1)
    print(T.temp2)
    print(T.temp3)

# R.addItem("Markus", 50,50)
# R.printRecepy()
# print(R.numOfItems)

#    print(R.getItem(1))
#    print(R.getItem(1)[0])

#    R.setItem(1, "Markus", 50, 59)
#    R.printRecepy()
#    print(R.numOfItems)

#    R.deleteLastItem()
#    R.printRecepy()
#    print(R.numOfItems)


