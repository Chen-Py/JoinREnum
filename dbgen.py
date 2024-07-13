from random import randint
filenames = {}
numlines = {}
numvars = {}
dbsize = 20
lowerBound = 1
upperBound = 10

with open("db/filenames.txt", "r") as f:
    for line in f:
        line = line.strip().split(" ")
        filenames[line[0]] = line[1]

with open("db/numlines.txt", "r") as f:
    for line in f:
        line = line.strip().split(" ")
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
    with open(filename, "w") as f:
        for i in range(n):
            line = []
            for j in range(k):
                line.append(str(int(randint(lowerBound, upperBound))))
            f.write("|".join(line) + "\n")

for relation in filenames:
    gen_data(relation)