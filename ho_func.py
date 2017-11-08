import pandas as pd

def rsrp_value(rsrp):
    rsrp_linear=-140
    rsrp_value=0
    while((rsrp_linear<rsrp)&(rsrp_value<97)):
        rsrp_linear+=1
        rsrp_value+=1
    return rsrp_value

def rsrq_value(rsrq):
    rsrq_linear=-19.5
    rsrq_value=0
    while((rsrq_linear<rsrq)&(rsrq_value<34)):
        rsrq_linear+=0.5
        rsrq_value+=1
    return rsrq_value

def compute_results(filename):
    import pandas as pd
    file_obj = open(filename, 'r')
    df = []
    for line in file_obj.read().split('\n'):
        line = line.split(' ')
        if (line[0]=='Download'):
            df.append([line[15][5:], float(line[8]), line[14][3:], 'COMPLETED', float(line[5])])       
        elif (line[0]=='End'):
            df.append([line[23][5:], float(line[11]), line[22][3:], 'UNCOMPLETED', 100.0])
    df = pd.DataFrame(df, columns=['nRun', 'angle', 'targetCellId', 'downloadCondition', 'downloadTime'])
    return df
