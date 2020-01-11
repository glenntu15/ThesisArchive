import os

class MutateData(object):
    """description of class"""

    def ShortenRedMaj(datalist):
        outdatalist = []
        startepoc = datalist[0][0]
        cyclecount = 0
        wasred = 0
        recordcount = 0
        nmutated = 0
        inmutation = False
        deltatime = 0.0;
        for record in datalist:
            newrecord = record.copy()
            if (record[1] == 1):
                wasred = wasred + 1  #wasred is to flag cycles for counting
                if ((cyclecount % 3) == 1):
                # every third cycle...after red time > x, start setting red to green
                    redtime= record[2]
                    if (redtime > 4.):
                        if (not inmutation):  # start of mutation
                            inmutation = True
                            nmutated = nmutated + 1
                            startredtime = redtime;
                        inmutation = True
                        newrecord[1] = 3  # mutate on this red, make it green
                        deltatime = redtime - startredtime; #redtime is last red time, add to green time
                        newrecord[2] = deltatime;
                        recordcount = recordcount + 1
            elif (record[1] == 3):  # green 
                
                if (wasred > 0):
                    cyclecount = cyclecount+1; # cycles start on Red, when off read increment cycle count
                    wasred = 0;   
                    if (inmutation):
                        newrecord[2] = record[2] + deltatime;
                if (inmutation):
                    newrecord[2] = record[2] + deltatime;
            else:
                inmutation = False;

            outdatalist.append(newrecord)
        
        print("++++++++++>>> records changed ",recordcount," mutations: ",nmutated," cycles : ",cyclecount)
        return outdatalist

        
