# Parameter
y_pixel = 1027 # 2 - 1029
y_range = 1.6
y_unit = 1.6 / 1027

x_pixel = 1605 # 2 - 1607
x_range = 1000
x_unit = 1000 / 1605
x_unit_neuron_pixel = 1605 / 1000

axis = []

def pix_to_val (pix):
	return round((1029 - pix) * y_unit, 4)

pix_array = [1029, 1029, 1027, 791, 1025, 793, 1024, 793, 978, 856, 887, 913]
pix_array = pix_array + [905, 882, 880, 897, 911, 918]
pix_array = pix_array + [910, 885, 844, 797, 758, 741, 737]
pix_array = pix_array + [739, 750, 769, 783, 790]
pix_array = pix_array + [786, 769, 728, 641, 505, 343, 207, 136]
pix_array = pix_array + [147, 215, 316, 425, 538, 646, 740, 814, 870, 912, 941, 960, 968]
pix_array = pix_array + [964, 939, 896, 849, 818, 810]
pix_array = pix_array + [817, 827, 831]

# random number
import random

def make_dense_data(p_array, num, y_range):
	rand_array = []
	rand_sum = 0
	pos = len(p_array) - 1
	for i in range(0, num):
		rand_num = random.randrange(1, 10)
		rand_array = rand_array + [rand_num]
		rand_sum = rand_sum + rand_num

	offset = (y_range / rand_sum)
	for i in range(0, num):
		p_array = p_array + [p_array[pos] - offset * rand_array[i]]
		pos = pos + 1
	return p_array

pix_array = make_dense_data(pix_array, 13, 19.0)

###########
pix_array = pix_array + [806, 790, 740, 630, 435, 190, 10, 2]
pix_array = pix_array + [129, 283, 410, 511, 599, 676, 745, 803, 849, 884, 910, 928, 935]
pix_array = pix_array + [929, 909, 879, 852, 833, 824, 823]
pix_array = pix_array + [827, 822, 841, 847]
pix_array = pix_array + [847, 842, 836, 828, 823, 820, 817]

pix_array = make_dense_data(pix_array, 7, 8.0)

############
pix_array = pix_array + [802, 781, 733, 637, 484, 305, 178, 159]
pix_array = pix_array + [231, 331, 425, 504, 574, 644, 711, 771, 823, 865, 899, 924, 941, 948]
pix_array = pix_array + [945, 928, 899, 868, 846, 838]
pix_array = pix_array + [840, 847, 852, 856]
pix_array = pix_array + [854, 847, 837, 830, 825]

pix_array = make_dense_data(pix_array, 10, 15.0)

##########
pix_array = pix_array + [803, 781, 719, 600, 440, 289, 216]
pix_array = pix_array + [244, 334, 432, 519, 593, 662, 723, 779, 829, 870, 904, 930, 950, 963, 965]
pix_array = pix_array + [952, 920, 876, 842, 828]
pix_array = pix_array + [832, 841, 847, 849]
pix_array = pix_array + [846, 841, 833, 828]

pix_array = make_dense_data(pix_array, 6, 16.0)
pix_array = make_dense_data(pix_array, 4, 4.0)


########
pix_array = pix_array + [799, 770, 692, 547, 355, 188, 129]
pix_array = pix_array + [193, 312, 426, 524, 607, 677, 741, 797, 844, 882, 914, 938, 953, 961]
pix_array = pix_array + [955, 931, 891, 851, 828, 824]
pix_array = pix_array + [833, 840, 845, 847, 847]
pix_array = pix_array + [847, 845, 840, 833, 829, 825, 821]

pix_array = make_dense_data(pix_array, 6, 12.0)

############
pix_array = pix_array + [804, 785, 728, 611, 431, 241, 131]
pix_array = pix_array + [155, 268, 390, 494, 579, 652, 716, 773, 822, 865, 898, 925, 946, 957, 958]

###########
def min (a,b):
	if a > b: return b
	else: return a
def max (a,b):
	if a > b: return a
	else: return b

def make_data (p_array):
	# 820-840 
	pre_range = int(p_array[len(p_array) - 1])
	print(pre_range)
	new_range = random.randrange(820, 840) # 820-840
	y_range = (pre_range - new_range) * 1.0
	print(new_range)
	print(y_range)
	num = random.randrange(5, 7)
	p_array = make_dense_data(p_array, num, y_range)

	# 845 - 857 and need gap from pre_range 15~30
	pre_range = int((p_array[len(p_array) - 1]))
	print(pre_range)
	a = max(845, pre_range + 15)
	b = min(857, pre_range + 30)
	new_range = random.randrange(min(a, b), max(a, b))
	y_range = (pre_range - new_range) * 1.0
	print(new_range)
	print(y_range)
	num = random.randrange(4, 5)
	p_array = make_dense_data(p_array, num, y_range)

	# 820 - 830 from pre_range 20-30
	pre_range = int(p_array[len(p_array) - 1])
	print(pre_range)
	a = max(820, pre_range - 30)
	b = min(830, pre_range - 20)
	new_range = random.randrange(min(a, b), max(a, b))
	y_range = (pre_range - new_range) * 1.0
	print(new_range)
	print(y_range)
	num = random.randrange(6, 8)
	p_array = make_dense_data(p_array, num, y_range)

	# 798 - 810 from pre_range 10 - 22
	pre_range = int(p_array[len(p_array) - 1])
	print(pre_range)
	a = max(798, pre_range - 10)
	b = min(810, pre_range - 22)
	new_range = random.randrange(min(a, b), max(a, b))
	y_range = (pre_range - new_range) * 1.0
	print(new_range)
	print(y_range)
	num = random.randrange(7, 10)
	p_array = make_dense_data(p_array, num, y_range)

	# 130 - 153 (215 possible)
	pre_range = int(p_array[len(p_array) - 1])
	print(pre_range)
	new_range = random.randrange(130, 153)
	y_range = (pre_range - new_range) * 1.0
	print(new_range)
	print(y_range)
	num = random.randrange(7, 8)
	p_array = make_dense_data(p_array, num, y_range)

	# 945 - 965
	pre_range = int(p_array[len(p_array) - 1])
	print(pre_range)
	new_range = random.randrange(945, 965)
	y_range = (pre_range - new_range) * 1.0
	print(new_range)
	print(y_range)
	num = random.randrange(13, 15)
	p_array = make_dense_data(p_array, num, y_range)


	return p_array


pix_array = make_data(pix_array)
pix_array = make_data(pix_array)
pix_array = make_data(pix_array)
pix_array = make_data(pix_array)
pix_array = make_data(pix_array)
pix_array = make_data(pix_array)

plen = len(pix_array)

for i in range(0, plen):
	axis = axis + [(i, pix_to_val(pix_array[i]))]

print(pix_array)
print("\n\n")
print(axis)
