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

class CycleDataThesis(object):
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
            if(tod > 5.) and (tod < 9):
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
        CycleDataThesis.hadState0 = True  #Always start with state one
        CycleDataThesis.hadState1 = False;
        CycleDataThesis.hadState2 = False;
        CycleDataThesis.first_indicator = current_indicator
        CycleDataThesis.last_indicator = current_indicator
        if (current_indicator == 1):
            CycleDataThesis.stateOrder = [1,3,4]
        if (current_indicator == 3):
            CycleDataThesis.stateOrder = [3,4,1]
        if (current_indicator == 4):
            CycleDataThesis.stateOrder = [4,1,3]
        CycleDataThesis.redtime = 0.
        CycleDataThesis.greentime = 0.
        CycleDataThesis.yellowtime = 0.

    # IsCompleteCycle updates state variables
    def IsCompleteCycle(current_indicator):
        if( CycleDataThesis.hadState0 and CycleDataThesis.hadState1 and CycleDataThesis.hadState2):  # YES! all the way around
            if (current_indicator != CycleData.stateOrder[0]):
                print(" ERROR===> start state skipped-- new record generated")
                status = 2  # true with error
            else:
                status = 1  # just fine
            CycleDataThesis.hadState0 = True
            CycleDataThesis.hadState1 = False
            CycleDataThesis.hadState2 = False
            CycleDataThesis.last_indicator = current_indicator
            return status
        else:  # where are we
            if (current_indicator == CycleData.stateOrder[1]):
                if (CycleDataThesis.hadState1):
                    print(" ERROR== repeat of state 1")
                else:
                    CycleDataThesis.hadState1 = True
            if (current_indicator == CycleData.stateOrder[2]):
                if (CycleDataThesis.hadState2):
                    print(" ERROR== repeat of state 2")
                else:
                    CycleDataThesis.hadState2 = True
            return 0
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    def ComputeCycleTime(Start_Time, datalist):
        NaN = float('nan')
        cycletime = []
        markpoints = []
        record = datalist[0]
        #state variables for this function
        CycleDataThesis.InitGlobalStateVars(record[1])
        #first_indicator = record[1]
        #last_indicator = first_indicator
        last_epoc = record[0]
        prevRecordTime = record[2]
        #end state variables
        l1 = len(datalist)
        cumtime = 0.0
        for record in datalist:   # t = last_epoc + (record[0] - last_epoc) / 2
            if ((record[0] - last_epoc) > 60):  # output interupted record reset everything
                t = last_epoc + (record[0] - last_epoc) / 2
                etime = (t - Start_Time) / 3600.

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
            if (epoch == 1425413690):
                print(" time reached")
            indicator = record[1]
            if (record[2] < 0.):
                print(" ERROR --- Negative time, indicator:",record[1])
            if (indicator != CycleDataThesis.last_indicator):
                #if (record[1] == CycleData.first_indicator): # been all the way around
                    #print(" new cycle record0",record[0])
                result = CycleDataThesis.IsCompleteCycle(record[1])
                if (result > 0 ):
                    #cumtime = cumtime + prevRecordTime
                    etime = (last_epoc - Start_Time) / 3600.
                    if (result > 1):  # new cycle but error, write error point
                        errorpoint = (CycleData.redtime,CycleData.greentime)
                        markpoints.append(errorpoint)

                    cumtime = CycleDataThesis.redtime + CycleData.greentime + CycleData.yellowtime
                    metadata = str(last_epoc) + ":r:" +  "{:.2f}".format(CycleData.redtime)
                    metadata = metadata + ":g:" + "{:.2f}".format(CycleData.greentime) + ":y:"+ "{:.2f}".format(CycleData.yellowtime)
                    rvals = CycleDataThesis.GetIntensity(last_epoc)
                    intensity = rvals[0]
                    #metadata = rvals[1]
                    point = (etime,cumtime, CycleData.redtime, CycleData.greentime, CycleData.yellowtime, metadata,intensity)
                    
                    cycletime.append(point)
                    if (CycleData.redtime < 0.0 or CycleData.greentime < 0. or CycleData.yellowtime < 0):
                        print(">>> BIG ERROR -- saving negative time r:",CycleData.redtime, ",G: ",CycleData.greentime, ", Y: ",CycleData.yellowtime, ",indicator, ", indicator, "at: ", record[0])
                    last_epoc = record[0]
                    prevRecordTime = record[2]
                    cumtime = 0.0
                    CycleDataThesis.redtime = 0.
                    CycleDataThesis.greentime = 0.
                    CycleDataThesis.yellowtime = 0.
                #else:
                    #cumtime = cumtime + prevRecordTime
                CycleData.last_indicator = record[1]
            prevRecordTime = record[2]
            last_epoc = record[0]
            
            if (indicator == 1):
                CycleDataThesis.redtime = record[2]
            elif (indicator == 3):
                CycleDataThesis.greentime = record[2]
            elif (indicator == 4):
                CycleDataThesis.yellowtime = record[2]
        return (cycletime,markpoints)

        


