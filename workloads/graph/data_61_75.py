import random

array = []
rand_sum = 0

# Pick random number
for i in range(0, 14):
	rand_num = random.randrange(1, 10)
	array = array + [rand_num]
	rand_sum = rand_sum + rand_num

offset = (18.0/rand_sum)
print(offset)

y_axis = [839]

print(array)
print(len(array))

for i in range(0, 14):
	y_axis = y_axis + [y_axis[i] - offset * array[i]]

print(y_axis)
len1 = len(y_axis)
 
for i in range(0, len1):
	y_axis[i] = round((1036 - y_axis[i]) * 0.0015655577299413, 4)

print(len1)
print(y_axis)



