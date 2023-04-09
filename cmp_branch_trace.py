import sys

n = len(sys.argv)

if (n > 3):
    exit()

f1 = open(sys.argv[1], "r")
f2 = open(sys.argv[2], "r")

diff = []
avg_access_size = 0

lines1 = f1.readlines()
lines2 = f2.readlines()

if(len(lines1) != len(lines2)):
    print("outputs not the same!")
    exit()

outlen = min(len(lines1), len(lines2))

for i in range(outlen):
    op_arr1 = lines1[i].split()
    op_arr2 = lines2[i].split()
    if(op_arr1[0] != "Branch" or op_arr2[0] != "Branch"):
        continue
    if (op_arr1[1] != op_arr2[1] or op_arr1[3] != op_arr2[3]):
        diff.append(lines1[i] + " diff " + lines2[i])

diffs = len(diff)

for i in range(diffs):
    print(diff[i])

print("Diffs: " + str(diffs) + " out of " + str(len(lines1)))
