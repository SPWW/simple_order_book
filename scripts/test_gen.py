import random


price_list = [str(x) for x in  range(100,1000,10)]
volume_range = [str(x) for x in range(1,100)]
orderid = 234123;
side = ['B','S']
action = ['A','X']

orders = []


for i in range(10000):
    a = random.choice(action)
    if a == 'A':
        oo = ['A',str(orderid),random.choice(side),random.choice(volume_range),random.choice(price_list)]
        print(",".join(oo))
        orders.append(oo)
        orderid+=1
    else:
        if(len(orders) == 0):continue;
        ox = random.choice(orders)
        if(ox[0] == 'A'):
            ox[0] = 'X'
            print(",".join(ox))

