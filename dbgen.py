from random import randint
filenames = {}
numlines = {}
numvars = {}
dbsize = 100000
lowerBound = 1
# upperBound = dbsize // 10
upperBound = 10000

with open("db/filenames.txt", "r") as f:
    for line in f:
        line = line.strip().split(" ")
        filenames[line[0]] = line[1]

with open("db/numlines.txt", "r") as f:
    for line in f:
        line = line.strip().split(" ")
        print(line[0], line[1])
        numlines[line[0]] = int(line[1]) if int(line[1]) > 0 else dbsize

with open("db/relations.txt", "r") as f:
    for line in f:
        line = line.strip().split("(")
        numvars[line[0]] = len(line[1].split(","))

print(filenames)
print(numlines)
print(numvars)
        

def gen_data(relation):
    filename = filenames[relation]
    n = numlines[relation]
    k = numvars[relation]
    S = set()
    with open(filename, "w") as f:
        for i in range(n):
            line = []
            for j in range(k):
                line.append(str(int(randint(lowerBound, upperBound))))
            while tuple(line) in S:
                line = []
                for j in range(k):
                    line.append(str(int(randint(lowerBound, upperBound))))
                    # normal distribution
                    # temp = np.random.normal((upperBound+lowerBound) / 2, (upperBound-lowerBound) / 6)
                    # temp = np.round(temp).astype(int)
                    # # if temp < lowerBound:
                    # #     temp = lowerBound
                    # # if temp > upperBound:
                    # #     temp = upperBound
                    # line.append(str(temp))
            S.add(tuple(line))
            f.write("|".join(line) + "\n")
        # output lines in S in lex order
        # S = sorted(S)
        # for line in S:
        #     f.write("|".join(line) + "\n")

for relation in filenames:
    gen_data(relation)