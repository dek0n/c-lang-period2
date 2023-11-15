sequence = [i * 0.1 for i in range(101)]
powered_sequence = [int(2 ** x) for x in sequence]


print("Sequence:")
print(sequence)

print("\nPowered Sequence:")
print(powered_sequence)

bright_levels = [0, 1, 2, 3, 4, 5, 6, 7,  8, 9,  10, 11, 12,  13, 14, 16, 17, 18, 19, 21, 22, 24, 25, 27, 29, 32, 34, 36, 39, 42, 45, 48, 51, 55, 59, 64, 68, 73, 78, 84, 90, 97, 103,
111, 119, 128, 137, 147, 157, 168, 181, 194, 207, 222, 238, 256, 274, 294, 315, 337, 362, 388, 415, 445, 477, 512, 548, 588, 630, 675, 724, 776, 831, 891, 955, 1000]

y = sum(1 for x in bright_levels)
print(y)

