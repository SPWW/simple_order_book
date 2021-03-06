/*
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

*/
#pragma once
#include<vector>
#include<unordered_map>
#include<iostream>
#include<functional>
#include <iomanip>


const int RESERVE_LEVEL_NUM = 100000;
const int RESERVE_ORDER_NUM = 1000000;


#define IOSTREAM_TYPE(VAR) os <<"(" << #VAR << ":"<< v.VAR <<")" << std::endl
#define IOSTREAM_CLASS(VAR) friend std::ostream& operator<<(std::ostream& os, const VAR& v){ os << "{\ntype:" << #VAR << "\n";
#define IOSTREAM_CLASS_END ; os << "}";return os;} 


#define PRICE_NONE -1
#define INDEX_NONE -1

struct node{
    int ind_next = INDEX_NONE;
    int ind_prev = INDEX_NONE;
    int m_index = INDEX_NONE;
    
    IOSTREAM_CLASS(node){
        IOSTREAM_TYPE(m_index);
        IOSTREAM_TYPE(ind_next);
        IOSTREAM_TYPE(ind_prev);
    }
    IOSTREAM_CLASS_END;
};

template<typename CONTAINER>
inline void idq_insert(CONTAINER& con, node& old_node, node& new_node){
    new_node.ind_next = old_node.m_index;
    new_node.ind_prev = old_node.ind_prev;

    old_node.ind_prev = new_node.m_index;
    if(new_node.ind_prev != INDEX_NONE){
        con[new_node.ind_prev].ind_next = new_node.m_index;
    }
}

template<typename CONTAINER>
inline void idq_insert_back(CONTAINER& con, node& old_node, node& new_node){
    new_node.ind_prev = old_node.m_index;
    new_node.ind_next = old_node.ind_next;

    old_node.ind_next = new_node.m_index;
    if(new_node.ind_next != INDEX_NONE){
        con[new_node.ind_next].ind_prev = new_node.m_index;
    }
}

template<typename CONTAINER>
inline void idq_remove(CONTAINER& con, node& n){
    if(n.ind_next != INDEX_NONE){
        con[n.ind_next].ind_prev = n.ind_prev;
    }
    if(n.ind_prev != INDEX_NONE){
        con[n.ind_prev].ind_next = n.ind_next;
    }
    n.ind_next = INDEX_NONE;
    n.ind_prev = INDEX_NONE;
}



enum class ORDER_SIDE{
    NONE,
    BID,
    ASK
};

std::ostream& operator <<(std::ostream& os, const ORDER_SIDE& s){
    switch(s){
        case ORDER_SIDE::BID: os << "Bid";return os;
        case ORDER_SIDE::ASK: os << "Ask";return os;
        case ORDER_SIDE::NONE: os << "None";return os;
    }
}


struct PriceLevel:node{
    int price = PRICE_NONE;

    int num_of_buy_orders = 0;
    int num_of_sell_orders = 0;

    int buy_order_begin_index = INDEX_NONE;
    int buy_order_end_index = INDEX_NONE;


    int sell_order_begin_index = INDEX_NONE;
    int sell_order_end_index = INDEX_NONE;


    IOSTREAM_CLASS(PriceLevel){
        IOSTREAM_TYPE(price);
        IOSTREAM_TYPE(num_of_buy_orders);
        IOSTREAM_TYPE(num_of_sell_orders);
        IOSTREAM_TYPE(buy_order_begin_index);
        IOSTREAM_TYPE(buy_order_end_index);
        IOSTREAM_TYPE(sell_order_begin_index);
        IOSTREAM_TYPE(sell_order_end_index);
        IOSTREAM_TYPE(m_index);
        IOSTREAM_TYPE(ind_next);
        IOSTREAM_TYPE(ind_prev);
    }
    IOSTREAM_CLASS_END;
};



struct Order: node{
    int id = INDEX_NONE;
    ORDER_SIDE side = ORDER_SIDE::NONE;
    short status = INDEX_NONE;  //order_tatus  0:init 1:available 2:completed.
    int quantity = INDEX_NONE;
    int price_level_index = INDEX_NONE;

    IOSTREAM_CLASS(Order){
        IOSTREAM_TYPE(id);
        IOSTREAM_TYPE(side);
        IOSTREAM_TYPE(quantity);
        IOSTREAM_TYPE(price_level_index);
        IOSTREAM_TYPE(status);
        IOSTREAM_TYPE(m_index);
        IOSTREAM_TYPE(ind_next);
        IOSTREAM_TYPE(ind_prev);
    }
    IOSTREAM_CLASS_END;
};

#undef IOSTREAM_TYPE
#undef IOSTREAM_CLASS
#undef IOSTREAM_CLASS_END


class OrderStore;





class PriceLadder{

    std::vector<PriceLevel> v_price_levels;
    std::unordered_map<int,int> price_level_index;
    int current_price_level_count = 0;

    int begin_index = INDEX_NONE; //point lowest price level.
    int end_index = INDEX_NONE; //point highest price level.


    //min/max price and index;  
    int min_ask_price = PRICE_NONE;
    int min_ask_index = INDEX_NONE;
    int max_bid_price = PRICE_NONE;
    int max_bid_index = INDEX_NONE;

public:
    PriceLadder(int reserved_level_count):v_price_levels(reserved_level_count){
        for(int i = 0;i<v_price_levels.size();i++){
            v_price_levels[i].m_index = i;
        }
        price_level_index.reserve(reserved_level_count);
    }


    int get_price_level_index(int price){
        if(auto it = price_level_index.find(price); it == price_level_index.end()){
            if(current_price_level_count >= RESERVE_LEVEL_NUM){
                v_price_levels.emplace_back();
                v_price_levels.back().m_index = current_price_level_count;
                v_price_levels.back().price = price;
            }else{
                v_price_levels[current_price_level_count].price = price;;
            }
            price_level_index.emplace(price,current_price_level_count); 
            return current_price_level_count++;
        }else{
            return it->second;
        }
    }


    inline void update_bbo(int price, ORDER_SIDE side, int index){
        if(side == ORDER_SIDE::BID){
            if( max_bid_price == PRICE_NONE || price > max_bid_price){
                max_bid_price = price;
                max_bid_index = index;
            }
        }else{
            if( min_ask_price == PRICE_NONE || price < min_ask_price){
                min_ask_price = price;
                min_ask_index = index;
            }
        }
    }


    int add_price_level(int price, ORDER_SIDE side){
        int index = get_price_level_index(price);
        if(v_price_levels[index].ind_next != INDEX_NONE || v_price_levels[index].ind_prev != INDEX_NONE){ //price alread in queue. 
            return index;
        }
        if(begin_index == INDEX_NONE) //empty ladder.
        {
            begin_index = index;
            end_index = index;
            return index;
        }
        else{
            for(int i = begin_index ;  i != INDEX_NONE ; i = v_price_levels[i].ind_next){
                if( v_price_levels[i].price > price){
                    idq_insert(v_price_levels,v_price_levels[i],v_price_levels[index]);
                    if(i == begin_index)begin_index = index;
                    return index;
                }
            }
            idq_insert_back(v_price_levels,v_price_levels[end_index],v_price_levels[index]);
            end_index = index;
            return index;
        }
    }

    PriceLevel& operator[](int index){
        return v_price_levels[index];
    }

    int get_max_bid_price(){return max_bid_price;}
    int get_min_ask_price(){return min_ask_price;}
    int get_max_bid_index(){return max_bid_index;}
    int get_min_ask_index(){return min_ask_index;}

    template<typename F>
    void for_each(F func, bool reverse = false) const{
        int start = begin_index;
        if(reverse) start = end_index;
        for(int i = start;  i != INDEX_NONE ; i = v_price_levels[i].ind_prev){
            func(v_price_levels[i]);
        }
    }

    friend class OrderStore;
};

//order storage, liner growth, and old slots will not be recycle. 
class OrderStore{

    
    std::vector<Order> v_orders;
    int init_order_id = INDEX_NONE; //first received order id;
    PriceLadder& m_ladder;
public:

    OrderStore(int reserve_number, PriceLadder& pl):m_ladder(pl){
        v_orders.reserve(reserve_number);
    }


    Order& add_order(int order_id, ORDER_SIDE side, int price, int quantity){
        if(init_order_id == INDEX_NONE)init_order_id = order_id;
        int index = order_id - init_order_id;

        if(index >= v_orders.size()){
            v_orders.emplace_back();
        }
        Order& o = v_orders[index];
        o.m_index = index;
        o.side = side;
        o.quantity = quantity;
        o.status = 1; //status available
        o.id = order_id;
        if(quantity == 0){
            o.status = 2;
            return o;
        }
        PriceLevel& lvl = m_ladder[m_ladder.get_price_level_index(price)];
        o.price_level_index = lvl.m_index; //back reference to price level.
        
        if(side == ORDER_SIDE::BID){
            if(lvl.buy_order_begin_index == INDEX_NONE){ //level empty
                lvl.buy_order_begin_index = index;
                lvl.buy_order_end_index = index;
            }else{
                idq_insert_back(v_orders,v_orders[lvl.buy_order_end_index],o);
                lvl.buy_order_end_index = index;
            }
            lvl.num_of_buy_orders++;
        }else{
            if(lvl.sell_order_begin_index == INDEX_NONE){ //level empty
                lvl.sell_order_begin_index = index;
                lvl.sell_order_end_index = index;
            }else{
                idq_insert_back(v_orders,v_orders[lvl.sell_order_end_index],o);
                lvl.sell_order_end_index = index;
            }
            lvl.num_of_sell_orders++;
        }

        m_ladder.update_bbo(price,side,lvl.m_index);
        #ifdef DEBUG
        std::cout << "add order" << std::endl;
        std::cout << o << std::endl;
        #endif
        return o;
    }


    Order& delete_order(int order_id){
        if(init_order_id == INDEX_NONE)init_order_id = order_id;
        int index = order_id - init_order_id;
        Order& o = v_orders[index];
        if(o.status != 1)return o;
        o.status = 2; //set order status to completed
        PriceLevel& lvl = m_ladder[o.price_level_index];
        
        
        
        auto delink_begin_or_end = [&o](int& begin, int& end, int index){
            if(begin == index){
                begin = o.ind_next;
            }
            if(end == index){
                end = o.ind_prev;
            }
        };
        
        
        if(o.side == ORDER_SIDE::BID){ //buy side
        
            
            delink_begin_or_end(lvl.buy_order_begin_index,lvl.buy_order_end_index,index);
            lvl.num_of_buy_orders--;


            //maintain bbo
            if(lvl.num_of_buy_orders == 0 && lvl.price == m_ladder.max_bid_price){
                int prev = lvl.ind_prev;
                while(prev != INDEX_NONE){
                    PriceLevel& plvl = m_ladder[prev];
                    if(plvl.num_of_buy_orders != 0){
                        m_ladder.max_bid_price = plvl.price;
                        m_ladder.max_bid_index = plvl.m_index;
                        break;
                    }
                    prev = plvl.ind_prev;
                }
                if(prev == INDEX_NONE){
                    m_ladder.max_bid_index = INDEX_NONE;
                    m_ladder.max_bid_price = PRICE_NONE;
                }
            }


        }else{ //sell side
            delink_begin_or_end(lvl.sell_order_begin_index,lvl.sell_order_end_index,index);
            lvl.num_of_sell_orders--;

            //maintain bbo
            if(lvl.num_of_sell_orders == 0 && lvl.price == m_ladder.min_ask_price){
                int next = lvl.ind_next;
                while(next != INDEX_NONE){
                    PriceLevel& nlvl = m_ladder[next];
                    if(nlvl.num_of_sell_orders != 0){
                        m_ladder.min_ask_price = nlvl.price;
                        m_ladder.min_ask_index = nlvl.m_index;
                        break;
                    }
                    next = nlvl.ind_next;
                }
                if(next == INDEX_NONE){
                    m_ladder.min_ask_index = INDEX_NONE;
                    m_ladder.min_ask_price = PRICE_NONE;
                }
            }
        }
        idq_remove(v_orders,o);  //unlink order from price level order list.
        #ifdef DEBUG
        std::cout << "delete order" << std::endl;
        std::cout << o << std::endl;
        #endif
        return o;
    }

    Order& operator[](int index){
        return v_orders[index];
    }

    const Order& operator[](int index) const {
        return v_orders[index];
    }

};




class Orderbook{


    PriceLadder m_ladder;
    OrderStore m_orders;
    
    std::function<void(int,int,int)> trade_callback;

public:
    Orderbook():
        m_ladder(RESERVE_LEVEL_NUM),
        m_orders(RESERVE_ORDER_NUM,m_ladder),
        trade_callback(nullptr)
    {
    }


    void set_trade_callback(std::function<void(int,int,int)> callback){
        trade_callback = callback;
    }

    /*return most arrgessive order based on side.*/
    int get_best_order(ORDER_SIDE side){
        if(side == ORDER_SIDE::BID && m_ladder.get_max_bid_index() != INDEX_NONE){
            auto& lvl = m_ladder[m_ladder.get_max_bid_index()];
            return lvl.buy_order_begin_index;
        }else if(side == ORDER_SIDE::ASK && m_ladder.get_min_ask_index() != INDEX_NONE){
            auto& lvl = m_ladder[m_ladder.get_min_ask_index()];
            return lvl.sell_order_begin_index;
        }
        return INDEX_NONE;
    }


    void match(int id, int& volume, Order& opp){
        int t_vol = volume;
        volume -= opp.quantity;
        opp.quantity = (volume < 0)?(-volume):0;
        volume = volume<0?0:volume;
        int t_px = m_ladder[opp.price_level_index].price;
        t_vol = t_vol-volume;
        if(trade_callback){
            trade_callback(id,t_px,t_vol);
            trade_callback(opp.id,t_px,t_vol);
        }

        if(opp.quantity == 0){
            m_orders.delete_order(opp.id);
        }
    }


    /*
    id, incremental order id.
    side, B:buy, S:sell
    price, integer price
    volume, order volume
    */
    void add_order(int id, char c_side, int price, int volume){
        
        ORDER_SIDE side = c_side == 'B'?ORDER_SIDE::BID : ORDER_SIDE::ASK;
        int index = m_ladder.add_price_level(price,side);
        PriceLevel& pl = m_ladder[index]; 
        
        
        if(side == ORDER_SIDE::BID){
            while(m_ladder.get_min_ask_price() != INDEX_NONE && m_ladder.get_min_ask_price() <= price && volume > 0){
                int index = get_best_order(ORDER_SIDE::ASK);    //get opposite side best order
                Order& opp = m_orders[index];
                match(id,volume,opp);
            }
        }else{
            while(m_ladder.get_max_bid_price() != INDEX_NONE && m_ladder.get_max_bid_price() >= price && volume > 0){
                int index = get_best_order(ORDER_SIDE::BID);    //get opposite side best order
                Order& opp = m_orders[index];
                match(id,volume,opp);
            }
        }
        m_orders.add_order(id,side,price,volume);

        #ifdef DEBUG
        std::cout << pl << std::endl;
        #endif
    }

    void delete_order(int id){
        m_orders.delete_order(id);

    }



    friend std::ostream& operator<<(std::ostream& os, const Orderbook& book){
        const PriceLadder& pl = book.m_ladder;
        const OrderStore& orders = book.m_orders;
        
        os  << "price" << "\t" << "orders" <<std::endl;;
        
        pl.for_each([&](auto& ll){
            if(ll.num_of_buy_orders + ll.num_of_sell_orders != 0) os << ll.price << '\t';
            if(ll.num_of_buy_orders != 0){
                os <<"buy\t num_of_orders:"<< ll.num_of_buy_orders << " \t[";
                for(int j = ll.buy_order_begin_index; j != INDEX_NONE; j = orders[j].ind_next){
                    os << "(id:" <<orders[j].id <<",vol:"<< orders[j].quantity << ")";
                }
                os << "]\t";
            }

            if(ll.num_of_sell_orders != 0){
                os << "sell\t num_of_orders:" << ll.num_of_sell_orders <<" \t[";
                for(int j = ll.sell_order_begin_index; j != INDEX_NONE; j = orders[j].ind_next){
                    os << "(id:" <<orders[j].id <<",vol:"<< orders[j].quantity << ")";
                }
                os << "]";
            }
            
            if(ll.num_of_buy_orders + ll.num_of_sell_orders != 0){
                os << std::endl;
            }
        }, true);
        return os;
    }

};
