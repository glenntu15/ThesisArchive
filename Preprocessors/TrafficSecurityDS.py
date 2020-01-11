import time
import pytz
#import datetime
from datetime import datetime
import math
from operator import itemgetter, attrgetter
from CycleData import CycleData
from synccheck import synccheck
from DistributionStats import DistributionStats
from DistributionStats import roadtype

from MutateData import MutateData

#RSENames = ["RSE25","RSE26","RSE27","RSE28","RSE29","RSE30"]
#RSENames = ["RSE25","RSE26","RSE29"]
#RSENames = ["RSEMUT25"]
RSENames = ["RSE29"]
Start_Time = 1425321066
marktime = 1425419499
pointime = (marktime - Start_Time) / 3600

print(" Point time = ",pointime)
# set to true to generate flags for abnormal data
metadataOn = False
minred  = 2.
maxred = 240.
minyellow = 2.
maxyellow = 6
mingreen = 2.
maxgreen = 240.

def takeSecond(elem):
    return elem[1]
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

def GetTime(epoc):
    tz = pytz.timezone("US/Mountain")
    dt = datetime.utcfromtimestamp(epoc)
    dt = dt.replace(tzinfo=pytz.utc)
    dt = dt.astimezone(tz)
     
    format = '%Y-%m-%d %H:%M %p'
    my_date = datetime.strftime(dt, format)
    date_time = my_date.split()
    date = date_time[0].split('-')
    l = len(date)
    #tstring = date[1] + '-' + date[2] + '-' + date_time[1] + '-' + date_time[2]
    tstring = date_time[1] + '-' + date_time[2]
    return tstring
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# This function just pulls out raw data
def ProcessRSE(name, type):
    # type is integer:
    # 0 Major intersection signal
    # 1 Minor intersecton signal
    # 2 phase 1 ring
    # 3 phase 2 ring

    #default values
    ndx_i = 6   # index of indicator for Major signal
    ndx_t = 7   # indx of time for major 
    maxIndicator = 4
    if (type == 1):  # minor signal indicator
        ndx_i = 9    # index of indicator for Minor signal
        ndx_t = 10   # indx of time for major 
        maxIndicator = 4
    if type == 2:       #phase ring one
        ndx_i = 1
        ndx_t = 2
        maxindicator = 4
    if (type == 3):
        ndx_i = 3
        ndx_t = 4
        maxindicator = 8
    datalist = []  # this will be returned: a list of tuples[epoc,indicator,cumtime)
    #
    # Open input file
    #
    filename = "c:/temp/"+name+"_rawfixed_.csv"

    print(" Opening file: ",filename)
    try:
        infile = open(filename, 'r')
    except IOError:
        print(" could not open file")
        exit()
    # read first (header) line
    line = infile.readline()
    if not line:
        exit();
    #if (line[0] == '#'):            #skip comments
    #    continue
        

    initialized = False
    buffer = []
    buffcount = 0
    last_epoc = 0

    records = 0
    while True:
        line = infile.readline()
        if not line:
            break
        if (line[0] == '#'):            #skip comments
            continue
    
        parts = line.split(",")
        leng = len(parts)
        if (leng < 3):
            break
  
        rse_number = parts[21].strip()
        if rse_number != name:
            print("**** INVALID RSE: ",rse_number)
            continue

        records = records + 1

        text = parts[ndx_i].strip()
        indicator = int(text)  # 1 = red, 3 = green, 4 = yellow

        text = parts[0].strip()
        epoc = int(text)
        #last_epoc = epoc
        #last_indicator = indicator
        text = parts[ndx_t].strip()
        indicator_time = float(text)
        # - - - - - - - - - - - - - - - - -  - - - 
        record = [epoc, indicator, indicator_time]

        datalist.append(record)

        #if (epoc != last_epoc):
        #    if (buffcount == 0):
        #        buffer.append(record)
        #        buffcount = 1
        #    elif (buffcount == 1):
        #        r = buffer.pop()
        #        last_indicator = r[1] ## save this for next list of records with same timestamp
        #        datalist.append(r)
        #        buffer.append(record)
        #        buffcount = 1
        #    else:
        #        if (last_indicator == maxIndicator):  #4 for RGY, 
        #            #buffer.sort(key=takeSecond, reverse=True)
        #            s = sorted(buffer, key=itemgetter(2))
        #            buffer = sorted(s,key=itemgetter(1),reverse=True)
        #        else:
        #            buffer.sort(key=takeSecond)
        #        for r in buffer:
        #            last_indicator = r[1] ## save this for next list of records with same timestamp
        #            datalist.append(r);
                    
        #        buffer.clear()
        #        buffer.append(record)
        #        buffcount = 1
        #    last_epoc = epoc
        #    #buffcount += 1
        #else:
        #    buffer.append(record)
        #    last_epoc = epoc
        #    buffcount += 1
        ##datalist.append(record)

    infile.close()

    ndata = len(datalist)
    print(" records processed: ",records," datapoints: ",ndata)
    return datalist

#
#   Build output file uses the list of tuples
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
def WriteTime_captureMetadata(datalist, outfile):
    initialized = False;
    errormarks = []
    if (metadataOn):
        redlist = []
        greenlist = []
        yellowlist = []

    for record in datalist:
        epoc = record[0]
        indicator = record[1]
        indicator_time = record[2]
        if (initialized == False):
            redtime = 0
            yellowtime = 0
            greentime = 0
            last_epoc = epoc
            last_indicator = indicator
            start_indicator = indicator
            if (indicator == 1):
                redtime = indicator_time
            elif (indicator == 3):
                greentime = indicator_time
            elif (indicator == 4):
                yellowtime = indicator_time
            else:
                print("+++++++++++++++++ERROR === INDICATOR = "+str(indicator))
            initialized = True
            outfile.write("0.,"+str(redtime)+","+str(greentime)+","+str(yellowtime)+"\n")
        else:
            if (epoc - last_epoc) > 180:  # 3 minute gap
                etime = ( (last_epoc + (epoc-last_epoc) /2 ) - Start_Time) / 3600;
                outfile.write(str(etime)+",?,?,?\n")
            if (indicator != last_indicator):
                if (indicator == start_indicator):   # we have been around the cycle
                    
                    #s = time.gmtime(epoc)
                    #l = time.strftime('%Y-%m-%d %H:%M:%S',s)
                    #print(" thistime ",l, "GMT")

                    etime = (epoc - Start_Time) / 3600.
                    #outfile.write(str(etime)+","+str(redtime)+","+str(greentime)+","+str(yellowtime)+","+str(epoc)+"\n")
                    outfile.write(str(etime)+","+str(redtime)+","+str(greentime)+","+str(yellowtime)+"\n")
                    if (metadataOn):
                        if (redtime > maxred) or (redtime < minred):
                            redlist.append((etime,redtime))
                        if (greentime > maxgreen) or (greentime < mingreen):
                            greenlist.append((etime,greentime))
                        if (yellowtime > maxyellow) or (yellowtime < minyellow):
                            yellowlist.append((etime,yellowtime))
                        redtime = 0.
                        greentime = 0.
                        yellowtime = 0.
        last_epoc = epoc
        last_indicator = indicator
        if (indicator == 1):
            redtime = indicator_time
        elif (indicator == 3):
            greentime = indicator_time
        elif (indicator == 4):
            yellowtime = indicator_time
        else:
            print("+++++++++++++++++ERROR === INDICATOR = "+str(indicator))
#
    if (metadataOn):
        errormarks.append(redlist)
        errormarks.append(greenlist)
        errormarks.append(yellowlist)

    return errormarks

#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
# Build a different kind of datalist
#
def BuildMultiIndicatorData(rsename, datalist1, datalist2, datalist3):  # add datalist 2 etc
    line1 = []
    line2 = []
    line3 = []
    #outvalue2 = []
    #outtime2 = []
    NaN = float('nan')

    # easier to process each seperately
    # ds1 and ds2 are signal values 1,3,4
#
#   Process major road signal
#
    first = True
    for record in datalist1:
        indicator = record[1]
        currtime = record[0]
        value = indicator
        if (indicator == 3):
            value = 2
        if (indicator == 4):
            value = 3
        epoc = record[0]
        if (first):
            last_value = value
            last_epoc = epoc
            point = (0,value)
            line1.append(point)
            first = False

            tz = pytz.timezone("US/Mountain")
            utc_dt = time.gmtime(epoc);
            dt = datetime.utcfromtimestamp(epoc)
            dt = dt.replace(tzinfo=pytz.utc)
            dt = dt.astimezone(tz)
      
            print(" file has first time: ",epoc," local time: ", dt.month,"/", dt.day,"/ at:",dt.hour, ":",dt.minute)
        else:
            if (epoc - last_epoc) > 180:  # three minute gap
                 etime = ( (last_epoc + (epoc-last_epoc) / 2 ) - Start_Time) / 3600;
                 point = (etime,NaN)
                 line1.append(point)
                 thistime = (epoc - Start_Time)/3600
                 #point = ((thistime - 0.001), last_value)
                 #line1.append(point) #time right before now
                 point = (thistime,value)
                 line1.append(point)
                 last_value = value
            elif (value != last_value):
                thistime = (epoc - Start_Time)/3600
                #if (epoc == 1425419500):
                    #print(" got to here - toi = ",thistime)
               
                point = ((thistime - 0.0001), last_value)
                line1.append(point) #time right before now
                point = (thistime,value)
                line1.append(point)
                last_value = value
            last_epoc = epoc
#
# ** process minor road signal
#
    first = True
    for record in datalist2:
        indicator = record[1]
        currtime = record[0]
        value = indicator
        if (indicator == 3):
            value = 2
        if (indicator == 4):
            value = 3
        epoc = record[0]
        if (first):
            last_value = value
            start_time = epoc
            last_epoc = epoc
            point = (0,value)
            point = (0,value)
            line2.append(point)
            first = False
        else:
            if (epoc - last_epoc) > 180:  # three minute gap
                etime = ( (last_epoc + (epoc-last_epoc) /2 ) - Start_Time) / 3600;
                point = (etime,NaN)
                line2.append(point)
                #point = ((thistime - 0.001), last_value)
                #line1.append(point) #time right before now
                point = (thistime,value)
                line2.append(point)
                last_value = value
            elif (value != last_value):
                #if (epoc == 1425413690):
                #    print(" got to here")
                thistime = (epoc - Start_Time)/3600
                #if (thistime > 25. and thistime < 25.9):
                #    print(" thistime: ",thistime, "val ",value)

                point = ((thistime - 0.0001), last_value)
                line2.append(point) #time right before now
                point = (thistime,value)
                line2.append(point)
                last_value = value
            
            last_epoc = epoc

    first = True
    for record in datalist3:   # indicator 1,3,4 -> 4 5 6
        indicator = record[1]
        currtime = record[0]
        if (indicator == 1):
            value = 4
        if (indicator == 3):
            value = 5
        if (indicator == 4):
            value = 6
        epoc = record[0]
        
        if (first):
            last_value = value
            start_time = epoc
            point = (0,value)
            line3.append(point)
            first = False
        else:
            if (value != last_value):
                thistime = (epoc - start_time)/3600
                point = ((thistime - 0.001), last_value)
                line3.append(point) #time right before now
                point = (thistime,value)
                line3.append(point)
                last_value = value

    return (line1, line2)
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#  Routine to write out the indicator data from lines -- one RSE singnal trace for major and minor road
#
def WriteSingleRSEIndicatorData(outfile, lines, nlines):
    line1 = []
    line1 = lines[0]
    l1 = len(line1)
    line2 = lines[1]
    l2 = len(line2)
    datalist1 = lines[0]
    #find smallest length
    min1 = l1
    if (l2 < l1):
        min1 = l2
    if (min1 < 100 and l1 > 100):
        for i in range(0,l1):
            #htime = line1[i][0]
            if (math.isnan(line1[i][1])):
                field = "?"
            else:
                field = str(line1[i][1])
            outfile.write(str(line1[i][0])+","+field+","+str(line1[i][0])+",1\n")
    else:
        for i in range(0,min1):
            #htime = line1[i][0]
            if (math.isnan(line1[i][1])):
                field = "?"
            else:
                field = str(line1[i][1])
            outfile.write(str(line1[i][0])+","+field+","+str(line2[i][0])+","+str(line2[i][1])+"\n")

#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
def GenerateHourlySignalData(outfile, datalist1, datalist2):  
    redtime_maj = []
    greentime_maj = []
    redtime_min = []
    greentime_min = []
    hourlabel = []

    red = 0.
    green = 0.
    red_min = 0.
    green_min = 0.
    hour_start = Start_Time
    prevRecordTime =0;
    prev_epoc = Start_Time
    last_indicator = datalist[0][1] # 1,3 or 4
    for record in datalist1:
        epoc = record[0]
        indicator = record[1]
        #if (epoc >= 1425341550 and epoc <= 1425409583):
            #if (red > 5000.) or (green > 5000.):
            #    print("epoc ",epoc," red: ",red," green: ", green, "ind: ",indicator,"time ",record[2] )
        if (epoc - hour_start) >= 3600:
            if (last_indicator == 1):
                red = red +  prevRecordTime
            elif (last_indicator == 3):
                green = green +  prevRecordTime
            redtime_maj.append(red)
            #print("appending red ",red)
            red = 0.
            greentime_maj.append(green)
            green = 0.

            tstring = GetTime(prev_epoc)
            hourlabel.append(tstring)

            hour_start = epoc

        if (indicator != last_indicator):
            if (last_indicator == 1):
                red = red +  prevRecordTime
            elif (last_indicator == 3):
                green = green +  prevRecordTime
            last_indicator = indicator
            #if (red > 5000.) or (green > 5000.):
            #    print(" error here:, red: ",red," green ",green," epoc: ",epoc, "start: ",hour_start)

        prevRecordTime = record[2]
        last_indicator = indicator
        prev_epoc = epoc


    outstring = ""
    for strtime in  hourlabel:
        outstring = outstring + "," + strtime

    outstring = outstring+"\n"
    outfile.write(outstring)

    outstring = "red_maj"
    for val in redtime_maj:
        outstring = outstring +  "," +  str(val)
    
    outstring = outstring + "\n"
    outfile.write(outstring)

    outstring = "green_maj"
    for val in greentime_maj:
        outstring = outstring +  "," +  str(val)

    outstring = outstring + "\n"
    outfile.write(outstring)

    red = 0.
    green = 0.
    red_min = 0.
    green_min = 0.
    hour_start = Start_Time
    prevRecordTime =0;
    prev_epoc = Start_Time
    last_indicator = datalist[0][1] # 1,3 or 4
    for record in datalist2:
        epoc = record[0]
        indicator = record[1]
        if (epoc - hour_start) >= 3600:
            if (last_indicator == 1):
                red = red +  prevRecordTime
            elif (last_indicator == 3):
                green = green +  prevRecordTime
            redtime_min.append(red)
            #print("appending red ",red)
            red = 0.
            greentime_min.append(green)
            green = 0.

            tstring = GetTime(prev_epoc)
            #print(" time: ",tstring," epoc = ",prev_epoc)
            hourlabel.append(tstring)

            hour_start = epoc

        if (indicator != last_indicator):
            if (last_indicator == 1):
                red = red +  prevRecordTime
            elif (last_indicator == 3):
                green = green +  prevRecordTime
            last_indicator = indicator

        prevRecordTime = record[2]
       
        last_indicator = indicator
        prev_epoc = epoc

    outstring = "red_min"
    for val in redtime_min:
        outstring = outstring +  "," +  str(val)
    
    outstring = outstring + "\n"
    outfile.write(outstring)

    outstring = "green_min"
    for val in greentime_min:
        outstring = outstring +  "," +  str(val)

    outstring = outstring + "\n"
    outfile.write(outstring)
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
def GenerateSignalRatioData(datalist, datalist2):
    NaN = float('nan')
    ratiodata = []
    redM = 0.
    green = 0.
    yellow = 0.
    record = datalist[0]
    last_indicator = record[1]
    last_epoc = record[0]
    l1 = len(datalist)
    for indx in range(0,l1):
        record = datalist[indx]
        recmin = datalist2[indx]
        if (recmin[0] != record[0]):
            print("time out of sync")
        if (record[0] - last_epoc) > 280:
            t = last_epoc + (record[0] - last_epoc) / 2
            point = (t,NaN)
            ratiodata.append(point)
            # reset everything
            redM = 0.
            yellow = 0
            green = 0
            last_indicator = record[1]
            last_epoc = record[1]
        if (record[1] == 3 and last_indicator == 1): # transition to green
            if (green + yellow) > .001:
                ratio = redM / (green + yellow)
                point = (record[0],ratio)
            else:
                point = (record[0],NaN)
            ratiodata.append(point)
            redM = 0.
            green = 0.
            yellow = 0.

        if (record[1] == 1):    # red light
            redM = record[2]
            if (recmin[1] == 3):
                green = recmin[2]
            if (recmin[1] ==  4):
                yellow = recmin[2]
        last_epoc = record[0]
        last_indicator = record[1]

    return ratiodata

#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
def WriteMultipleRSEIndicatorData(outfile, lines):
    
    #find smallest length
    minl = 999999999999
    ll = len(lines)
    lengths = [0] * ll
    ix = 0
    for scaledline in lines:
        if (len(scaledline) > 100) and (len(scaledline) < minl):
            minl = len(scaledline)
        lengths[ix] = len(scaledline)
        ix = ix + 1

    for i in range(0,minl):
        string = ""
        n = 0
        for scaledline in lines:
            if (lengths[n] > i):
                htime = scaledline[i][0]
                if (math.isnan(scaledline[i][1])):
                    string = string + "{:.4f}".format(htime) + "," + "??,"
                else:
                    val = scaledline[i][1]
                    string = string + "{:.4f}".format(htime) + "," + "{:.2f}".format(val) + ","
            else:
                string = string + "?,?,"
            n = n + 1
        string = string + "\n"
        #print(string)
        outfile.write(string)

#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#
def WriteMultiCycledata(outfile,lines):

    #find smallest length
    minl = 999999999
    ll = len(lines)
    lengths = [0] * ll
    ix = 0
    for scaledline in lines:
        if (len(scaledline) > 100) and (len(scaledline) < minl):
            minl = len(scaledline)
        lengths[ix] = len(scaledline)
        ix = ix + 1

    for i in range(0,minl):
        string = ""
        n = 0
        for scaledline in lines:
            if (lengths[n] > i):
                htime = scaledline[i][0]
                if (math.isnan(scaledline[i][1])):
                    string = string + "{:.4f}".format(htime) + "," + "?,"
                else:
                    val = scaledline[i][1]
                    string = string + "{:.4f}".format(htime) + "," + "{:.2f}".format(val) + ","
            else:
                string = string + "?,?,"
            n = n + 1
        string = string + "\n"
        #print(string)
        outfile.write(string)
#
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
# set this function up seperate because it might write all RESs
def writeRatiodata(outfile,ratiodata):
    for point in ratiodata:
        t = (point[0] - Start_Time) / 3600
        if math.isnan(point[1]):
            outfile.write(str(t)+",?\n")
        else:
            outfile.write(str(t)+","+str(point[1])+"\n")
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
def WriteMarksData(errormarks, outfile):
    nlines = len(errormarks)
    print("+++++++++++++++++++++++++++++++++>>> nlines = ",nlines)
    redline = errormarks[0]
    le = len(errormarks)
    if (le > 1):
        greenline = errormarks[1]
    else:
        greenline = []
    if (le > 2):
        yellowline = errormarks[2]
    else:
        yellowline = []

    lr = len(redline)
    print("+++++++++++++++++++++++++++++++++>>> length redline = ",lr)

    string1 = "t"
    string2 = "one#1"
    for p in redline:
        string1 = string1 + "," +" {:.4f}".format(p[0])
        string2 = string2 + "," +" {:.4f}".format(p[1])
    outfile.write(string1+"\n")
    outfile.write(string2+"\n")
    if (len(greenline) > 1):

        string1 = "t"
        string2 = "two#1"
        for p in greenline:
            string1 = string1 + "," +" {:.4f}".format(p[0])
            string2 = string2 + "," +" {:.4f}".format(p[1])
        outfile.write(string1+"\n")
        outfile.write(string2+"\n")

    if (len(yellowline) > 1):
        string1 = "t"
        string2 = "three#1"
        for p in yellowline:
            string1 = string1 + "," +" {:.4f}".format(p[0])
            string2 = string2 + "," +" {:.4f}".format(p[1])
        outfile.write(string1+"\n")
        outfile.write(string2+"\n")
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
def write2scatternorms(name, cycledata,cycledata2):
    outfilename = name+"_ScatterNormRushCycle_Data.csv"
    outfilename2 = name+"_ScatterNormNonRushCycle_Data.csv"
    print(" Opening file: ",outfilename)
    try:
        outfile = open(outfilename, 'w')
    except IOError:
        print(" could not open file")
        exit()

    print(" Opening file: ",outfilename2)
    try:
        outfile2 = open(outfilename2, 'w')
    except IOError:
        print(" could not open file")
   
    outfile.write("DSN"+","+name+","+"DST,standardpairs\n")
    outfile.write("t1,Major,t2,Minor\n")

    outfile2.write("DSN"+","+name+","+"DST,standardpairs\n")
    outfile2.write("t1,Major,t2,Minor\n")
    

    #pick the largest number of points and loop over that 
    lmaj = len(cycledata)
    lmin = len(cycledata2)
    lim = lmaj
    if (lmin > lim):
        lim = lmin

    for i in range(0,lim):
        if (i < lmaj):
            point = cycledata[i]
            intensityMaj = point[6]
            total = point[2] + point[3] + point[4]
            r = point[2]
            fr = r / total
            g = point[3]
            fg = g / total
            string = "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + ","
        else:
            string = "??,??,"
            intensityMaj = 0

        if (i < lmin):
            point = cycledata2[i]
            intensityMin = point[6]
            total = point[2] + point[3] + point[4]
            r = point[2]
            fr = r / total
            g = point[3]
            fg = g / total
            string = string + "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "\n"
        else:
            string = string + "??,??\n"

        if (intensityMaj >=200 and intensityMin >= 200):
            outfile.write(string)
        else:
            outfile2.write(string)
    outfile.close()
    outfile2.close()
 # - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
##################################################################################################
# - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#        main program starts here
#
##################################################################################################
NaN = float('nan')

multiRSElines = []  #list of list, each list is a line which is an array of tuples
multicycletimelines = []  #list of list, each list is a line which is an array of tuples
goodRSEList = []
done = False
RSEcount = 1

tz = pytz.timezone("US/Mountain")
dt = datetime.utcfromtimestamp(Start_Time)
dt = dt.replace(tzinfo=pytz.utc)
dt = dt.astimezone(tz)
     
print(" Start time ",Start_Time," local time: ", dt.year,"/",dt.month,"/", dt.day,"/ at:",dt.hour, ":",dt.minute)
toi = (1425413690 - Start_Time) / 3600
print(" Time of interest = ",toi)

format = '%Y-%m-%d %H:%M %p'
my_date = datetime.strftime(dt, format)
print(" my date ",my_date)

MT = MutateData
CD = CycleData
DS = DistributionStats
#
#  Begin loop over RSE files **
#
#SC = synccheck
for name in RSENames:
    if (done):
        break

    print("name = ",name)
   # SC.CheckSync(name)
#
# **************** section:  Time data *************************
#
# ** Process the Raw .csv file
    #datalist1 = []
    datalist = ProcessRSE(name, 0)

    # Mutate
    datalist_m = MT.ShortenRedMaj(datalist)

    #debugoutfilename = name+"_Debug_Data.csv"
    #print(" Opening file: ",debugoutfilename)
    #try:
    #    outfile = open(debugoutfilename, 'w')
    #except IOError:
    #    print(" could not open file")
    #    exit()
    #for record in datalist:
    #    string = " {:d},".format(record[0]) + "{:d},".format(record[1]) + "{:.2f}".format(record[2]) + "\n"
    #    outfile.write(string)
    #outfile.close()

#
#  Write out cumulative time data
#
    outfilename = outfilename = name+"_SignalTimeMajor_Data.csv"
    metadatafilename = name+"_SignalTimeMajor_Marks.csv"
    print(" Opening file: ",outfilename)
    try:
        outfile = open(outfilename, 'w')
    except IOError:
        print(" could not open file")
        exit()

    if (metadataOn):
        outfile.write("DSN"+","+name+","+"DST,standard,MARKS,"+metadatafilename+",\n")
    else:
        outfile.write("DSN"+","+name+","+"DST,standard\n")

    outfile.write("Elapsed time,Red,Green,Yellow\n")

    #WriteTimeOutputFile(datalist, outfile);
    errormarks =  WriteTime_captureMetadata(datalist, outfile);
    outfile.close()
    if (metadataOn):
        try:
            outfile = open(metadatafilename, 'w')
        except IOError:
            print(" could not open file")
            exit()
        WriteMarksData(errormarks, outfile)
        outfile.close()


# Now process RSE file again for minor signal data
    datalist2 = []
    datalist2 = ProcessRSE(name, 1)
    
    l2 = len(datalist2)
    if (l2 > 50):
        outfilename = name+"_SignalTimeMinor_Data.csv"
        metadatafilename = name+"_SignalTimeMinor_Marks.csv"
        print(" Opening file: ",outfilename)
        try:
            outfile = open(outfilename, 'w')
        except IOError:
            print(" could not open file")
            exit()
       
        if (metadataOn):
            outfile.write("DSN"+","+name+","+"DST,standard,MARKS,"+metadatafilename+",\n")
        else:
            outfile.write("DSN"+","+name+","+"DST,standard\n")
        outfile.write("Elapsed time,Red,Green,Yellow\n")

        #WriteTimeOutputFile(datalist2, outfile);
        # errormarks is three arrays of tuples -- WriteMarkData takes a list of list of tuples (multiple standard pairs)
        errormarks.clear()
        errormarks = WriteTime_captureMetadata(datalist2, outfile)
        outfile.close()
        if (metadataOn):
            try:
                outfile = open(metadatafilename, 'w')
            except IOError:
                print(" could not open file")
                exit()
            WriteMarksData(errormarks, outfile)
            outfile.close()
#
# Generate and write hourly data
#

    outfilename = name+"_HourlySignal_Data.csv"
    print(" Opening file: ",outfilename)
    try:
        outfile = open(outfilename, 'w')
    except IOError:
        print(" could not open file")
        exit()

    outfile.write("DSN,"+name+" Hourly Accumulated,DST,datagroupseries\n")
    GenerateHourlySignalData(outfile, datalist, datalist2);

    outfile.close()
#
#  compute signal cycle times -- accumulate for all intersections
#

#
#  compute and write out signal ratios
#
    l1 = len(datalist)
    l2 = len(datalist2)
    print(" l1 = ",l1," l2 = ",l2)
    if (l2 > 100):   # only when major and minor together are there
        
        goodRSEList.append(name)
        
        #outfilename = outfilename = name+"_SignalRatio_Data.csv"
        #print(" Opening file: ",outfilename)
        #try:
        #    outfile = open(outfilename, 'w')
        #except IOError:
        #    print(" could not open file")
        #    exit()

        #outfile.write("DSN"+","+name+","+"DST,standardpairs\n")
        #outfile.write("Time,Ratio\n")
        #ratiodata = []
        #ratiodata = GenerateSignalRatioData(datalist, datalist2)
        #writeRatiodata(outfile,ratiodata)

        #outfile.close()
#
#  compute and save cycletime
#
    alldata = []
    ismajor = True
    alldata = CD.ComputeCycleTime(Start_Time,datalist_m, ismajor) # returns a list of tuples: epochtime,cum,red,green,yellow
    cycledata = alldata[0]

    RT = roadtype;
    DS.GetDistribution(name, cycledata, RT.major)
                      
    errormarks = alldata[1]
    avgRedMaj = alldata[2]
    avgGreenMaj = alldata[3]
    avgYellowMaj = alldata[4]
    cyclelines = []
    for record in cycledata:
        point = (record[0],record[1])
        cyclelines.append(point)

    multicycletimelines.append(cyclelines)

    # *** Now get cycle data for minor street
    print("----- Now processing cycle data for Minor road signal\n")
    ismajor = False
    alldata = CD.ComputeCycleTime(Start_Time,datalist2,ismajor) # returns a list of tuples: epochtime,cum,red,green,yellow,metadata, indicator


    cycledata2 = alldata[0]

    DS.GetDistribution(name, cycledata2, RT.minor)

    errormarks2 = alldata[1]
    avgRedMin = alldata[2]
    avgGreenMin = alldata[3]
    avgYellowMin = alldata[4]
    # write out scatter plot data for cycles
    outfilename = outfilename = name+"_ScatterCycle_Mut4_Data.csv"
    print(" Opening file: ",outfilename)
    try:
        outfile = open(outfilename, 'w')
    except IOError:
        print(" could not open file")
        exit()
##


## ** write it out
    outfile.write("DSN"+","+name+","+"DST,standardpairs\n")
    lmaj = len(cycledata)
    lmin = len(cycledata2)
# # *** if data for major and minor have been written
    if (lmaj > 5 and lmin > 5): 
        outfile.write("Red_Maj,Major,Red_Min,Minor\n")
    else:
        outfile.write("Red_Maj,Major\n")
        
    lim = lmaj;
    if (lmin > 5):
        if (lmin < lim):
            lim = lmin

    #for i in range(0,lim):
    #    point = cycledata[i]
    #    string = "{:.4f}".format(point[2]) + "," + "{:.4f}".format(point[3]) + ","
    #    if (point[3] < 17.):
    #        print("r: ", point[2]," g: ",point[3]," M: ",point[5])
    #    if (lmin > 5):
    #        point = cycledata2[i]
    #        string = string + "{:.4f}".format(point[2]) + "," + "{:.4f}".format(point[3]) + "\n"
    #    else:
    #        string = string + "\n"
    #    outfile.write(string)


#
# THIS CODE FOR NORMALIZED DATA CYCLES
#
        #TotalAvgMaj = avgRedMaj + avgGreenMaj + avgYellowMaj
        #TotalAvgMin = avgRedMin + avgGreenMin + avgYellowMin
        maxfr = 0.
        minfg = 999.
        for i in range(0,lim):
            point = cycledata[i]
            total = point[2] + point[3] + point[4]
            if (math.isnan(total) or math.isnan(point[2]) or math.isnan(point[3])):
                string = "??,??,"
            else:
                r = point[2]
                fr = r / total
                g = point[3]
                
                fg = g / total
                if (r < 4.):
                    print(" this is mutated: ",point[5]," point is: (",fr,",",fg,") metadata: ",point[5])
                string = "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "," 
                metadata = point[5]
                

                #Minor Road data
                if (lmin > 5):
                    
                    point = cycledata2[i]
                    total = point[2] + point[3] + point[4]
                    metadata = point[5]
                    if (math.isnan(total) or math.isnan(point[2]) or math.isnan(point[3])):
                        string = string + "??,??," + metadata + ",??,\n"
                    else:
                        r = point[2]
                        #if (r > 380.):
                        #    print(" point metadata ",metadata)
                        fr = r / total
                        #if (fr > .97):
                        #    print(" other point fr ",fr," metadata ",metadata)
                        
                        g = point[3]
                        fg = g / total
                        #if (fr < .40 and fg > .55):
                        #    print(" ***point: fr, ", fr, " fg, ",fg," metadata",point[5])
                        #    print(" Red ", r, " Green: ", g, "total = ",total)
                        string = string + "{:.4f}".format(fr) + "," + "{:.4f}".format(fg)
                        ##leave out metadata
                        ##string = string + "," + metadata + "," + point[5] + "\n"
                        string = string + "\n"
                        if (fr > maxfr):
                            maxfr = fr
                        if (fg < minfg):
                            minfg = fg
                else:
                    string = string + "\n"
                outfile.write(string)
## *** if data for major and Not for minor have been written
#    if (lmaj > 5 and lmin < 5):
#        outfile.write("Red_Maj,Major\n")
#        for i in range(0,lmaj):
#            point = cycledata[i]
#            total = point[2] + point[3] + point[4]
#            if (math.isnan(total) or math.isnan(point[2]) or math.isnan(point[3])):
#                string = "??,??\n"
#            else:
#                r = point[2]
#                fr = r / total
#                g = point[3]
#                fg = g / total
#                string = "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "\n"

#            outfile.write(string)
    #print(" maxfr = ",maxfr," minfg = ",minfg)
    outfile.close()
##################################################################################################
#
# Now cycle data with intensity of dots 
#
    write2scatternorms(name, cycledata,cycledata2)
    #outfilename = outfilename = name+"_ScatterNormRushCycle_Data.csv"
    #print(" Opening file: ",outfilename)
    #try:
    #    outfile = open(outfilename, 'w')
    #except IOError:
    #    print(" could not open file")
    #    exit()

    ##pick the largest number of points and loop over that 
    #lim = lmaj
    #if (lmin > lim):
    #    lim = lmin
    #outfile.write("DSN"+","+name+","+"DST,augmentedpairs\n")
    #outfile.write("t1,Major,t2,Minor\n")

    #for i in range(0,lim):
    #    if (i < lmaj):
    #        point = cycledata[i]
    #        total = point[2] + point[3] + point[4]
    #        r = point[2]
    #        fr = r / total
    #        g = point[3]
    #        fg = g / total
    #        string = "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "," + str(point[6]) + ","
    #    else:
    #        string = "??,??,??,"

    #    if (i < lmin):
    #        point = cycledata2[i]
    #        total = point[2] + point[3] + point[4]
    #        r = point[2]
    #        fr = r / total
    #        g = point[3]
    #        fg = g / total
    #        string = string + "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "," + str(point[6]) + "\n"
    #    else:
    #        string = string + "??,??,??\n"
    #    outfile.write(string)
    #outfile.close()
##################################################################################################
#
#  Now write 2 files depending on time of day
#
    #outfilename = outfilename = name+"_ScatterNormIntenseCycle_Data.csv"
    #print(" Opening file: ",outfilename)
    #try:
    #    outfile = open(outfilename, 'w')
    #except IOError:
    #    print(" could not open file")
    #    exit()

    ##pick the largest number of points and loop over that 
    #lim = lmaj
    #if (lmin > lim):
    #    lim = lmin
    #outfile.write("DSN"+","+name+","+"DST,augmentedpairs\n")
    #outfile.write("t1,Major,t2,Minor\n")

    #for i in range(0,lim):
    #    if (i < lmaj):
    #        point = cycledata[i]
    #        total = point[2] + point[3] + point[4]
    #        r = point[2]
    #        fr = r / total
    #        g = point[3]
    #        fg = g / total
    #        string = "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "," + str(point[6]) + ","
    #    else:
    #        string = "??,??,??,"

    #    if (i < lmin):
    #        point = cycledata2[i]
    #        total = point[2] + point[3] + point[4]
    #        r = point[2]
    #        fr = r / total
    #        g = point[3]
    #        fg = g / total
    #        string = string + "{:.4f}".format(fr) + "," + "{:.4f}".format(fg) + "," + str(point[6]) + "\n"
    #    else:
    #        string = string + "??,??,??\n"
    #    outfile.write(string)
    #outfile.close()
##################################################################################################
# ** write out marks data -- major street
    #metadatafilename = name+"_cycletime_Marks.csv"
    #allmarks = []
    #allmarks.append(errormarks)
    ##allmarks.append(cycledata2)
    #try:
    #    outfile = open(metadatafilename, 'w')
    #except IOError:
    #    print(" could not open file")
    #    exit()
    #WriteMarksData(allmarks,outfile)
#
#  ****************** Done with time data  **********************
#
    datalist3 = []
    #datalist3 = ProcessRSE(name, 2)

 #   datalist4 = []
 #   datalist4 = ProcessRSE(name, 3)

# generate plot lines from indicators
    lines = BuildMultiIndicatorData(name, datalist_m, datalist2, datalist3)
# 
#   Write out indicator trace lines
#
    #outfilename = outfilename = name+"_SignalIndicatorTrace_Data.csv"
    #print(" Opening file: ",outfilename)
    #try:
    #    outfile = open(outfilename, 'w')
    #except IOError:
    #    print(" could not open file")
    #    exit()

    #outfile.write("DSN"+","+name+","+"DST,standardpairs\n")
    #outfile.write("Elapsed time,Major_Signal,Time2,Minor_Signal\n")

    #nlines = 2
    #WriteSingleRSEIndicatorData(outfile, lines, nlines)
    #outfile.close()
    
# accumulate trace for signals for this intersection -- save traces (in multiRSElines)
   
    for line in lines:
        scaledline = []
        for point in line:
            etime = point[0]
            val = point[1]
            newval = (val * .5) - 1  # sets to -.5,0..5 then add count of RSE
            newpoint = (etime,(newval + RSEcount))
            #if (etime > 25.1 and etime < 25.9):
                #print("iter",RSEcount, " etime: ",etime, "val ",newval)
            scaledline.append(newpoint)

        multiRSElines.append(scaledline)

 #
 # ** Phase ring stuff **
 #

    #outfilename = outfilename = name+"_PhaseRinglIndicatorTrace_Data.csv"
    #print(" Opening file: ",outfilename)
    #try:
    #    outfile = open(outfilename, 'w')
    #except IOError:
    #    print(" could not open file")
    #    exit()

    #outfile.write("DSN"+","+name+","+"DST,standardpairs\n")
    #outfile.write("Elapsed time,Ring_1,Time2,Ring_2\n")

    #nlines = 2
    #WriteSingleRSEIndicatorData(outfile, lines, nlines)
#
# - - - - -  End of Loop over Files
#    

# 
# Write out signal trace aggregated over files
#
outfilename = "MultiRSESignal_Data.csv"
print(" Opening file: ",outfilename)
try:
    outfile = open(outfilename, 'w')
except IOError:
    print(" could not open file")
    exit()

outfile.write("DSN,Multi-Trace,DST,standardpairs\n")
string = ""
index = 1
for rsename in RSENames:
    string = string + "time" + str(index) + ',' + rsename+"_Maj," + "time" + str(index+1) + "," + rsename + "_min,"
    index = index + 1
##outfile.write("time1,Maj_25,Time2,Min_25,Time3,Maj_26,Time4,Min_26,Time5,Maj_27\n")
string = string + "\n"
outfile.write(string)

WriteMultipleRSEIndicatorData(outfile, multiRSElines)
outfile.close()
 
#Write out cycle times aggregated over files

#outfilename = "MultiCycle_Data.csv"
#print(" Opening file: ",outfilename)
#try:
#    outfile = open(outfilename, 'w')
#except IOError:
#    print(" could not open file")
#    exit()

#string = "DSN,CycleTimes,DST,standardpairs\n"
#outfile.write(string)
#string = ""
#for rsename in RSENames:
#    string = string + "T_" + rsename + ',' + rsename+"_Maj,"

#string = string + "\n"
#outfile.write(string)
#WriteMultiCycledata(outfile,multicycletimelines)
#outfile.close()