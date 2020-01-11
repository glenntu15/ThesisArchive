
from enum import Enum
class roadtype(Enum):
    major = 0
    minor = 1
    
class DistributionStats(object):
    """description of class"""

    def GetDistribution(name, cycles, roadtype):
        nbins = 11
        vmax = 150.0
        vmin = 15.0
        rbins = [0] * nbins
        gbins = [0] * nbins
        delta = (vmax - vmin)/(float(nbins-2))
        binvals = [0] * nbins
        val = vmin;
        for i in range(0,nbins):
            binvals[i] = val;
            val = val + delta;

        for point in cycles:
            r = point[2]
            g = point[3]
            
            index = DistributionStats.GetBin(r,binvals)
            rbins[index] = rbins[index] + 1

            index = DistributionStats.GetBin(g,binvals)
            gbins[index] = gbins[index] + 1

        outfilename = ""
        if (roadtype == roadtype.major):
            print("------------Major Road-------------------")
            outfilename = name+"_Major_dist.csv"
            firstline = "DSN,"+name+" Major ,DST,datagroupseries\n"
            print("rbins ",rbins)
            print("gbins ",gbins)
        if (roadtype == roadtype.minor):
            print("------------Minor Road-------------------")
            outfilename = name+"_Minor_dist.csv"
            firstline = "DSN,"+name+" Minor ,DST,datagroupseries\n"
            print("rbins ",rbins)
            print("gbins ",gbins)

        
        #outfile.write("DSN,"+name+" Major Red,DST,datagroupseries\n")
        
       
   
        print(" Opening file: ",outfilename)
        try:
            outfile = open(outfilename, 'w')
        except IOError:
            print(" could not open file")
            exit()

        # write header 
        outfile.write(firstline)
        start = vmin
        print("<",start)
        string = ", <{:.0f}".format(start)
        #string = string + ",gt {:.0f}".format(start)
        
        # Write line for header
        for i in range(1,nbins-1):
            start = start + delta
            print(",",start)
            string = string + ",{:.0f}".format(start)
        print(" DEBUG STRING--->",string)
        string = string + ",> {:.0f}".format(start) + "\n"
        outfile.write(string)
        # Write line for red
        string = "Red"   
        for i in range(0,nbins):
            string = string + ",{:.0f}".format(rbins[i])
        print(" DEBUG STRING--->",string)
        string = string + "\n"
        outfile.write(string)
     # Write line for green
       
        string = "Green"   
        for i in range(0,nbins):
            string = string + ",{:.0f}".format(gbins[i])
        print(" DEBUG STRING--->",string)
        string = string + "\n"
        outfile.write(string)
        outfile.close

#--------------------------------------------------------------------------------------------------
    def GetBin(val, binvals):
        index = 0
        l = len(binvals)
        for curval in binvals:
            if (curval > val):
                return index
            index = index + 1
        if (index == l):
            index = index - 1
        return index

        
            


