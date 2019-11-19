

class synccheck(object):
    """description of class"""
    def CheckSync(name):
        ndx_i = 6   # index of indicator for Major signal
        ndx_it = 7   # indx of time for major 
        ndx_j = 9   # index of indicator for Major signal
        ndx_jt = 10   # indx of time for major 

        filename = "c:/temp/"+name+"_rawfixed_.csv"

        print(" SyncCheck Opening file: ",filename)
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
                print("****INVALID RSE: ",rse_number)
                continue

            records = records + 1
            text = parts[ndx_i].strip()
            indicator_MAJ = int(text)  # 1 = red, 3 = green, 4 = yellow
            text = parts[ndx_j].strip()
            indicator_MIN = int(text)  # 1 = red, 3 = green, 4 = yellow
            if (indicator_MAJ == 3):
                if (indicator_MIN) == 3:
                    print(" +++++++++++++ TWO GREEN LIGHTS ##############")
                    print(" Epoch ",line[0])
            if (indicator_MAJ == 4) and (indicator_MIN == 4):
                 print(" +++++++++++++ TWO YELLOW LIGHTS ##############")
                 print(" Epoch ",line[0])