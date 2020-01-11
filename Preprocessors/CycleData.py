from enum import Enum
import time
import pytz
#import datetime
from datetime import datetime
import math

class State(Enum):
    RED = 1
    GREEN = 3
    YELLOW = 4

class CycleData(object):
    """description of class"""
    # variable with class scope
    hadState0 = False
    hadState1 = False;
    hadState2 = False;
    first_indicator = 0
    last_indicator = 0
    stateOrder = []
    redtime = 0.
    greentime = 0.
    yellowtime = 0.

    def GetIntensity(epoc):
        tz = pytz.timezone("US/Mountain")
        dt = datetime.utcfromtimestamp(epoc)
        dt = dt.replace(tzinfo=pytz.utc)
        dt = dt.astimezone(tz)
        
        intensity = 100

        format = '%Y-%m-%d %H:%M %p'
        my_date = datetime.strftime(dt, format)
        date_time = my_date.split()
        hm = date_time[1].split(":")
        h = float(hm[0])
        m = float(hm[1])
        h = h + m/60
        tod = h
        if (date_time[2] == "AM"):
            if(tod > 6.) and (tod < 9):
                intensity = 250
        else:
            tod = tod - 12
            if(tod >= 3.5) and (tod <= 6):
                intensity = 250
        #date = date_time[0].split('-')
        #l = len(date)
        #tstring = date[1] + '-' + date[2] + '-' + date_time[1] + '-' + date_time[2]
        return (intensity,my_date)

    def InitGlobalStateVars(current_indicator):
        CycleData.hadState0 = True  #Always start with state one
        CycleData.hadState1 = False;
        CycleData.hadState2 = False;
        CycleData.first_indicator = current_indicator
        CycleData.last_indicator = current_indicator
        if (current_indicator == 1):
            CycleData.stateOrder = [1,3,4]
        if (current_indicator == 3):
            CycleData.stateOrder = [3,4,1]
        if (current_indicator == 4):
            CycleData.stateOrder = [4,1,3]
        CycleData.redtime = 0.
        CycleData.greentime = 0.
        CycleData.yellowtime = 0.

    # IsCompleteCycle updates state variables
    def IsCompleteCycle(current_indicator):
        if( CycleData.hadState0 and CycleData.hadState1 and CycleData.hadState2):  # YES! all the way around
            if (current_indicator != CycleData.stateOrder[0]):
                print(" ERROR===> start state skipped-- new record generated")
                status = 2  # true with error
            else:
                status = 1  # just fine
            CycleData.hadState0 = True
            CycleData.hadState1 = False
            CycleData.hadState2 = False
            CycleData.last_indicator = current_indicator
            return status
        else:  # where are we
            if (current_indicator == CycleData.stateOrder[1]):
                if (CycleData.hadState1):
                    print(" ERROR== repeat of state 1")
                else:
                    CycleData.hadState1 = True
            if (current_indicator == CycleData.stateOrder[2]):
                if (CycleData.hadState2):
                    print(" ERROR== repeat of state 2")
                else:
                    CycleData.hadState2 = True
            return 0
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def ComputeCycleTime(Start_Time, datalist, ismajor):
        NaN = float('nan')
        if (ismajor):
            firststate = 1 # red
        else:
            firststate = 3 # green
        isincycle = False
        cycletime = []
        markpoints = []
        record = datalist[0]
        #state variables for this function
        CycleData.InitGlobalStateVars(record[1])
        #first_indicator = record[1]
        #last_indicator = first_indicator
        last_epoc = record[0]
        prevRecordTime = record[2]
        #end state variables
        l1 = len(datalist)
        cumtime = 0.0
        totalred = 0.
        totalgreen = 0.
        totalyellow = 0.
        numcycles = 0
        maxred = 0.
        minred = 9999.
        maxgreen = 0.
        mingreen = 9999.
        idebug = 0
        for record in datalist:   # t = last_epoc + (record[0] - last_epoc) / 2
            idebug = idebug + 1
            if ((record[0] - last_epoc) > 60):  # output interupted record reset everything
                t = last_epoc + (record[0] - last_epoc) / 2
                etime = (t - Start_Time) / 3600.
                isincycle = False
                #point = (etime,NaN,NaN,NaN,NaN, last_epoc)
                #cycletime.append(point)

                CycleData.InitGlobalStateVars(record[1])
                #CycleData.first_indicator = record[1]
                #CycleData.last_indicator = CycleData.first_indicator
                #last_epoc = record[0]
                prevRecordTime = record[2]
            #print(" record: ",record[1], " ", record[2])
            #
            # do another sanity check here
            if (last_epoc - record[0] > 60):
                print(" ERROR out of order record: ",record[0]," last: ",last_epoc)
            epoch = record[0]
            
            indicator = record[1]
            if (not isincycle):
                if (indicator == firststate):
                    isincycle = True
                    CycleData.last_indicator = indicator
                else:
                    continue

            #if (epoch == 1425413690):
            #    print(" time reached")
            
            if (record[2] < 0.):
                print(" ERROR --- Negative time, indicator:",record[1])
            if (indicator != CycleData.last_indicator):
                #if (record[1] == CycleData.first_indicator): # been all the way around
                    #print(" new cycle record0",record[0])
                result = CycleData.IsCompleteCycle(record[1])
                if (result > 0 ):
                    isincycle = 0
                    #cumtime = cumtime + prevRecordTime
                    etime = (last_epoc - Start_Time) / 3600.
                    if (result > 1):  # new cycle but error, write error point
                        errorpoint = (CycleData.redtime,CycleData.greentime)
                        markpoints.append(errorpoint)

                    cumtime = CycleData.redtime + CycleData.greentime + CycleData.yellowtime
                    metadata = str(last_epoc) + " (" + "{:.4f}".format(etime) + ") "
                    metadata = metadata + ":r:" +  "{:.2f}".format(CycleData.redtime)
                    metadata = metadata + ":g:" + "{:.2f}".format(CycleData.greentime) + ":y:"+ "{:.2f}".format(CycleData.yellowtime)
                    rvals = CycleData.GetIntensity(last_epoc)
                    intensity = rvals[0]
                    #metadata = rvals[1]
                    point = (etime,cumtime, CycleData.redtime, CycleData.greentime, CycleData.yellowtime, metadata,intensity)

                    # accumulate statistics
                    if (CycleData.redtime > maxred):
                        maxred = CycleData.redtime;
                    if (CycleData.redtime < minred):
                        minred = CycleData.redtime;
                        if (minred < .01):
                            print(" WHAT?")
                    if (CycleData.greentime > maxgreen):
                        maxgreen = CycleData.greentime;
                    if (CycleData.greentime < mingreen):
                        mingreen = CycleData.greentime;
                    totalred = totalred + CycleData.redtime
                    totalgreen = totalgreen + CycleData.greentime
                    totalyellow = totalyellow + CycleData.yellowtime;
                    numcycles = numcycles + 1

                    cycletime.append(point)

                    if (CycleData.redtime < 0.0 or CycleData.greentime < 0. or CycleData.yellowtime < 0):
                        print(">>> BIG ERROR -- saving negative time r:",CycleData.redtime, ",G: ",CycleData.greentime, ", Y: ",CycleData.yellowtime, ",indicator, ", indicator, "at: ", record[0])
                    last_epoc = record[0]
                    prevRecordTime = record[2]
                    cumtime = 0.0
                    CycleData.redtime = 0.
                    CycleData.greentime = 0.
                    CycleData.yellowtime = 0.
                #else:
                    #cumtime = cumtime + prevRecordTime
                CycleData.last_indicator = record[1]
            prevRecordTime = record[2]
            last_epoc = record[0]
            
            if (indicator == 1):
                CycleData.redtime = record[2]
            elif (indicator == 3):
                CycleData.greentime = record[2]
            elif (indicator == 4):
                CycleData.yellowtime = record[2]
        if (numcycles > 1):
            avgred = totalred / numcycles;
            avggreen = totalgreen / numcycles
            avgyellow = totalyellow / numcycles;
            print("----> num cycles ",numcycles," avg red ",avgred," avg green ",avggreen," avg yellow ",avgyellow);
            print("----> Max red ",maxred," min red ",minred," max green ",maxgreen, " min green ",mingreen)

            CycleData.Getstddev(cycletime, numcycles, avgred, avggreen, avgyellow)
        return (cycletime,markpoints,avgred, avggreen, avgyellow)

    def Getstddev(cycletime, numcycles, avgred, avggreen, avgyellow):
        devred = 0.
        devgreen = 0.
        devyellow = 0.
        N = 0
        ## point = (etime,cumtime, CycleData.redtime, CycleData.greentime, CycleData.yellowtime, metadata,intensity)
        for point in cycletime:
            z = point[2] - avgred
            devred = devred + z * z;
            z = point[3] - avggreen
            devgreen = devgreen + z * z;
            z = point[4] - avgyellow
            devyellow = devyellow + z * z;
            N = N + 1
        devred = math.sqrt(devred / N)
        devgreen = math.sqrt(devgreen / N)
        devyellow = math.sqrt(devyellow / N)
        print(" std dev red = ",devred, " std dev green = ",devgreen," std dev yellow = ",devyellow)
        rmin = avgred - devred
        rmax = avgred + devred
        gmin = avggreen - devgreen
        gmax = avggreen + devgreen
        ymin = avgyellow - devyellow
        ymax = avgyellow + devyellow
        nrmax = 0
        nrmin = 0
        ngmax = 0
        ngmin = 0
        nymin = 0
        nymax = 0
        for point in cycletime:
            if (point[2] > rmax):
                nrmax = nrmax + 1
            if (point[2] < rmin):
                nrmin = nrmin + 1
            if (point[3] > gmax):
                ngmax = ngmax + 1
            if (point[3] < gmin):
                ngmin = ngmin + 1
            if (point[4] > ymax):
                nymax = nymax + 1
            if (point[4] < ymin):
                nymin = nymin + 1
        print (" Number Red > st dev: ",nrmax," Number red < st dev",nrmin)
        print (" Number green > st dev: ",ngmax," Number red < st dev",ngmin)



        


