import os

class MutateData(object):
    """description of class"""

    def ShortenRedMaj(datalist):
        outdatalist = []
        startepoc = datalist[0][0]
        cyclecount = 0
        wasred = 0
        recordcount = 0
        for record in datalist:
            newrecord = record  
            if (record[1] == 1):
                wasred = wasred + 1
                if ((cyclecount % 101) == 1):
                    if (wasred > 6): # let two records be red 
                        newrecord[1] = 3  # mutate on this red, make it green
                    recordcount = recordcount + 1
            else:
                if (wasred > 0):
                    cyclecount = cyclecount+1;
                    wasred = 0;

            outdatalist.append(newrecord)
        
        print("++++++++++>>> records changed ",recordcount)
        return outdatalist

        
