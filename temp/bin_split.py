from gmpy import mpq, mpz, mpf
import time
import math


def mod1(x):
    return x-mpz(x)


def pi():
    x = 0
    n = 1
    while 1:
        p = mpq((120*n-89)*n+16, (((512*n-1024)*n+712)*n-206)*n+21)
        x = mod1(16*x + p)
        n += 1
        yield int(16*x)


def binary_split(a, b):
    if b - a == 1:
        Pk = 120*a*a + 151*a + 47
        Qk = 512*a**4 + 1024*a**3 + 712*a**2 + 194*a + 15
        return (Pk, Qk)
    else:
        m = (a + b) // 2
        (P1, Q1) = binary_split(a, m)
        (P2, Q2) = binary_split(m, b)
    return (P1 * Q2 + P2 * Q1, Q1 * Q2)


def pi2():
    x = mpf(0)
    n = 1
    while 1:
        p = mpf((120*n-89)*n+16)
        q = mpf((((512*n-1024)*n+712)*n-206)*n+21)
        x = (16*x + (p/q)) % 1
        n += 1
        yield int(16*x)


def allpi(to, gen):
    pi_gen = enumerate(gen())

    start = time.time()
    for _ in range(to):
        (_, next_element) = next(pi_gen)

    end = time.time()
    print(f"Elapsed time for {to} positions: {(end - start):.4f}s\n")

    res = "Result: "

    for _ in range(to, to + 10):
        (_, next_element) = next(pi_gen)
        res = res + f"{next_element:X}"

    print(res)
