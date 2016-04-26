huge = 10 ** 8

big = open('big', 'w')
big.write("9" * huge)
big.write("\n")
big.write("1" * (1000))
big.write("\n")
big.write("9" * huge)
big.write("\n")
big.write("2" * (300))
big.write("\n")
big.write("9" * huge)
big.write("\n")
big.write("3" * (30))
big.write("\n")
big.write("9" * huge)
big.write("\n")
big.write("4" * (7))
big.write("\n")
big.close()

petite = 10 * 4

small = open('small', 'w')
small.write("1" * petite)
small.write("\n")
small.close()