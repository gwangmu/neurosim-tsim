
f = open('third_party/ramulator/configs/DDR4-config.cfg', 'r')
cfg = f.readlines()
f.close()

fs = []
for i in range(4):
    fs.append(open('third_party/ramulator/configs/DDR4_' + str(i) \
                    + '-config.cfg', 'w'))

for line in cfg:
    if ('record_cmd_trace = off' in line):
        is_record = True
    else:
        is_record = False

    for i, f in enumerate(fs):
        if(is_record):
            f.write(' record_cmd_trace = on\n')
            f.write(' cmd_trace_prefix = result/DRAM' + str(i) + '-\n')
        else:
            f.write(line)
