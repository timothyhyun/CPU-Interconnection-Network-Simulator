import sys

n = len(sys.argv)

if (n > 2):
    exit()

f = open(sys.argv[1], "r")

load = 0
store = 0
avg_access_size = 0

lines = f.readlines()

for line in lines:
    op_arr = line.split()
    if (op_arr[0] == 'L'):
      load += 1
    else:
      store += 1
    avg_access_size = (avg_access_size + int(op_arr[2]))/2

print("Loads: ", load, "\n")
print("Stores: ", store, "\n")
print("Average Access Size: ", avg_access_size, "\n")
