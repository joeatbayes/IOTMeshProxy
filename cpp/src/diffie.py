# Diffie-Hellman Code
 
 
def prime_checker(p):
    # Checks If the number entered is a Prime Number or not
    if p < 1:
        return -1
    elif p > 1:
        if p == 2:
            return 1
        for i in range(2, p):
            if p % i == 0:
                return -1
            return 1
 
 
def primitive_check(g, p, L):
    # Checks If The Entered Number Is A Primitive Root Or Not
    for i in range(1, p):
        L.append(pow(g, i) % p)
    for i in range(1, p):
        if L.count(i) > 1:
            L.clear()
            return -1
        return 1
 

def diffie(P, G, x1, x2):
    l = []
    print(f"P={P} G={G} PrivKey1={x1} privKey2={x2}")
    if prime_checker(P) == -1:
        print(f"Number {P} Is Not Prime, Please Enter Again!")
        return -1

    if primitive_check(G, P, l) == -1:
        print(f"Number {G} Is Not A Primitive Root Of {P}")
        return -1
 
    if x1 >= P or x2 >= P:
        print(f"Private Key Of Both The Users Should Be Less Than {P}!")
        return -1 
    
    # Calculate Public Keys
    y1 = pow(G, x1) % P
    y2 = pow(G, x2) % P
    print (f"PubKey1={y1}  pubkey2={y2}\n")

 
    # Generate Secret Keys
    k1 = pow(y2, x1) % P
    k2 = pow(y1, x2) % P
 
    print(f"SecKey1={k1}\tSecKey2={k2}\n")
    if x1 == y1: 
       print("error pubkey1 == privkey1")
       return -1
    elif x2 == y2:
       print("error pubkey2 = privkey2")
       return -1 
    elif y1 == k1:
       print("error computed key1 == pub key1")
       return -1 
    elif y2 == k2:
        print("error computed key2 = public key2")
        return -1 
    elif k1 == k2:
       print("Success")
       return 1
    else:
      print("error compute keys not same")   
      return -1
    
diffie(23,19,17,8)
diffie(239,123,210,212)
# Thinking about this wondering if we can use 6 or 10 of these 
# smaller numbers and bring together and makes better than
# what we have in keys.h until we find a libarary that 
# truely solves the problem. I think it wouldn't really work
# because hackers  could separate and solve in isolation 
# unless there was no way for them to figure out the 
# grouping but if that is the case then there is no way 
# for the peer device to figure it out either and they would 
# need to do so to make diffie hellman work. For this to work 
# well we need a alternative pow function that can work against 
# 1000 bit numbers. Probably better to just wait until espressiff 
# offers the mpow function as part of security features.
# or just make it easy for devices to auto generate counter signed 
# TLS or PKI certificates.
diffie(641, 344, 619, 601)
diffie(526271,210504, 500001, 498117) # essentially crashes the process
diffie(53787863,25869888, 10007863, 33007863) # essentially crashes the process