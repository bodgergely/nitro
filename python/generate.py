

import sys
import random
import os

def generate(alphabet , size, output_file_name):
    def do_generate(outfile):
        for _ in xrange(size):
            c = random.choice(alphabet)
            outfile.write(c)

    with open(output_file_name, 'w') as outfile:
        do_generate(outfile)    


alphabet = ['a', 'b','c','d']

if __name__ == "__main__":
    count = int(sys.argv[1])
    outfile = os.path.expanduser(sys.argv[2])
    generate(alphabet, count, outfile)
    