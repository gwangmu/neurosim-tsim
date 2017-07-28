import matplotlib.pyplot as plt
import numpy as np

# Parameters
x_max = 1000
y_max = 1.6
linewidth = 0.3

x_axis = [	   1,     2,      3,     4,    5,      6,     7,     8,    9,   10]
y_axis = [0.0015, 0.003, 0.0045, 0.376, 0.02, 0.3745, 0.006, 0.373, 0.27, 0.078]

x_axis = x_axis + [    11,    12,     13,     14,   15,     16,     17,    18,     19,     20]
y_axis = y_axis + [  0.22, 0.181, 0.1935, 0.2285, 0.23, 0.2079, 0.1827, 0.156, 0.1843, 0.2268]

for i in range(21, 76):
	x_axis = x_axis + [i]

y_axis = y_axis + [0.2896, 0.3616, 0.4211, 0.4493, 0.4556, 0.4524, 0.4352, 0.4055, 0.3836, 0.3726, 0.3742, 0.3789, 0.4039, 0.4697, 0.6059, 0.8188, 1.074, 1.2853, 1.3965, 1.3777, 1.2728, 1.1147, 0.944, 0.7671, 0.598, 0.4509, 0.335, 0.2474, 0.1816, 0.1346, 0.1065, 0.0939, 0.1018, 0.1409, 0.2067, 0.2818, 0.3303, 0.3413, 0.3319, 0.3162]

y_axis = y_axis + [0.3084, 0.3104, 0.3127, 0.3151, 0.3162, 0.3198, 0.3202, 0.3229, 0.3245, 0.3256, 0.3276, 0.3307, 0.3339, 0.3358, 0.3366]

print(x_axis)
print(len(x_axis))
print(y_axis)
print(len(y_axis))

plt.rcParams['lines.linewidth'] = linewidth
plt.rcParams['lines.color'] = 'k'
plt.scatter(x_axis, y_axis, marker = '+', s = 120, edgecolor = 'k', lw = linewidth)
plt.plot(x_axis, y_axis)

plt.axis([0, x_max, 0, y_max])
plt.show()
