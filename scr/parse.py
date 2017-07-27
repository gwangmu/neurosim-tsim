import sys

def check (line):
    strs = ['NBC', 'SRAM', 'NB', 'Sink', 'TSMgr', 'AMQ', 'AMT', 'Acc', \
            'AMR', 'AS', 'Src', 'AT', \
            'DRAM', 'AEC', 'SDQ', 'SDD', 'DGSRAM', 'CoTS', 'TSM', 'PkC', \
            'PkD', 'FW', 'AND'
            ]

    out = False
    if ('[' in line):
        out = True

    return out

    # out = False;
    # for i in range(len(strs)):
    #     s = '[' + strs[i] + ']'
    #     if (s in line):
    #         out = True
    #         break

    # return out


if __name__ == '__main__':
    f = open('out', 'r')
    data = f.readlines()
    f.close()

    is_start = False
    is_end = False
    summary = False

    is_design = True
    print 
    print "<Design Setup>"

    f = open('process_out', 'w')
    for line in data:
        if(is_start):
            if ('Simulating') in line:
                f.write("\n")
                f.write(line)
            elif (check(line)):
                f.write("\t" + line)
            elif ('Simulation finished' in line):
                is_start = False
                is_end = True
                summary =True
                f.write ("\n" + line)
            else:
                pass
        elif(is_end):
            if (summary):
                if ('Activity and Events' in line):
                    summary = True
                    break
                else:
                    print line.strip('\n')
            f.write(line)

        else:
            if ('Scheduled Clock' in line):
                is_design = False
            elif ('Design summary' in line):
                is_design = True
            elif ('Starting simulation' in line) :
                is_start = True
                is_design = False
                f.write(line)
            
            if (is_design):
                print line.strip('\n')
                f.write(line)

    f.close()
