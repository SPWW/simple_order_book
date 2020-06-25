OrderBook.h

this order book is based on a pre allocated vector. all price levels and orders are stored in Vector and sequence are linked by index. 
so if we know order book price high/low band and book tick size, we can very efficiently pre allocate memory. there will be zero run time malloc. 

PriceLadder is a ordered queue sit on top of array. so if new order added we will simply emplace one new level on back of the memory and it will be linked 
to list based on it price. it only happend in the first time of new price appears, again if high/low band and tick size is a known factor this can be done in initial peroid. 

Orders are stored also in a vector with RESERVE_ORDER_NUMBER size reserve. it will simply grow every time a new order is added, and old slot after order been deleted will not be
reused in current implementation (it a little bit wast of memory but we can index by orderid simple use vector offset.) further more, if memory size is a problem, it's easy to add
a empty slot collection by push empty slot index to a stack and every time just try to use recycled slots first. 


Based one the fact that vector is pre allocate and vector memory is continuesly, run time cache/TLB miss will be minimal. And if order book is heated up, for operation add/delete will
both be O(1) complexity.

Add order:  1 hash map search find price level + 1 vector emplace back  + price level order list tail index change. 
Delete order: order_id - first order_id get offset of vector + unlink order from price level order list. 
