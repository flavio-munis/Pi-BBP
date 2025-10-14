import time


def fac_rec(n, acc=1):
    if n == 1:
        return acc
    return fac_rec(n - 1, acc * n)

def fac(n):
    acc = 1

    while n > 1:
        acc *= n
        n -= 1

    return acc


def count_set_bits (n):
    count = 0

    while (n != 0):
        count += 1
        n &= n - 1

    return count

def calc_odd_factors(start, stop):
    num_factors = (stop - start) >> 1

    # Only two factors left (start is always odd)
    if num_factors == 2:
        return start * (start + 2)

    if num_factors == 1:
        return start

    if num_factors > 1:
        mid = (start + num_factors) | 1
        lft = calc_odd_factors(start, mid)
        rght = calc_odd_factors(mid, stop)
        return lft * rght

    return 1


def fac_bin_split(n):
    acc = block = 1

    for i in range(n.bit_length(), -1, -1):
        start = (n >> i + 1) + 1 | 1
        stop = (n >> i) + 1 | 1
        block *= calc_odd_factors(start, stop)
        acc *= block

    return acc << (n - count_set_bits(n))


def calc_fac(n, f):
    start = time.time()
    res = f(n)
    end = time.time()
    print(f"Elapsed time for fac {n}: {(end - start):.4f}s\n")

    print(f"Result Bit Lenght: {res.bit_length()}")
