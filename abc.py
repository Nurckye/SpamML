def main():
    outfile = open('counting.txt', 'w')

    print('This program will create a text file with counting numbers')

    N = int(input('How many numbers would you like to store in this file: '))

    for number in range(N):  # the variable number will get every value from 0 to N-1 in each iteration
        outfile.write(str(number + 1) + '\n')

    outfile.close()
    print('Data has been written to counting.txt')


if __name__ == "__main__":
    main()