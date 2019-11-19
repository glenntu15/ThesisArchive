
RSENames = ["RSE25","RSE26","RSE27","RSE28","RSE29","RSE30"]

def Pass(rsename):

    #RSENames = ["RSE25","RSE26","RSE27","RSE28","RSE29","RSE30"]
    filename = "c:/temp/raw_SSE.csv"

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
        
    outfile.write(line)
    ntotal  = 1
    nwritten = 1

    initialized = False
    buffer = []
    buffcount = 0
    last_epoc = 0

    while True:
        line = infile.readline()
        if not line:
            break
        if (line[0] == '#'):            #skip comments
            continue
    
        parts = line.split(",")
        lenp = len(parts)
        if (lenp < 3):
            break
        
        ntotal = ntotal + 1
        rse_number = parts[21].strip()
       
        rse_number2 = parts[lenp-1].strip()
        
        if (rse_number != rsename) & (rse_number2 != rsename):
            isok = False
            for trial in RSENames:
                if (rse_number2 == trial):
                    isok = True;
            if (isok == False):
                print(" *** invalid name at ",ntotal," name = ",rse_number," line length = ",lenp)
            continue
        
        if (nwritten == 1): # only header writtten out
            text = parts[0].strip()
            epoc = int(text)
        nwritten = nwritten + 1
        outfile.write(line)

    infile.close()
    print(" ntotal = ",ntotal," nwritten = ",nwritten,"first time = ",epoc)
    return epoc
# main program starts here 
#RSENames = ["RSE25","RSE26","RSE27","RSE28","RSE29"]

smallest = 0
for name in RSENames:
    outfilename = outfilename = name+"_TEST_.csv"
    print(" Opening file: ",outfilename)
    try:
        outfile = open(outfilename, 'w')
    except IOError:
        print(" could not open file")
        exit()

    epoc = Pass(name)
    if (smallest == 0):
        smallest = epoc
    else:
        if (epoc < smallest):
            smallest = epoc
    outfile.close()

print(" smallest epoc = ",smallest)

