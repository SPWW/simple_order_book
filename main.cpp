#include "orderbook.h"
#include <fstream>
#include <array>


template<typename T>
void for_each_update(std::ifstream& f, T callback){
    std::string line;

    if(f.is_open()){
        while( std::getline(f, line)){
            size_t init_p = 0;

            char side,action;
            int price,volume,orderid;
            
            for(int i = 0;i<5;i++){
                size_t p = line.find(',',init_p);
                if(p == std::string::npos){
                    std::string_view ss(line.c_str()+init_p);
                    price=std::atoi(ss.data());
                    init_p = p+1;
                }else{
                    std::string_view ss(line.c_str()+init_p,p-init_p);
                    init_p = p+1;
                    switch(i){
                        case 0:action = ss.at(0);break;
                        case 1:orderid = std::atoi(ss.data());break;
                        case 2:side = ss.at(0);break;
                        case 3:volume = std::atoi(ss.data());break;
                    }
                }
            }
            // std::cout << line << std::endl;
            callback(action,orderid,side,price,volume);
        }
    }   
}






int main(int argc, char* argv[]){
    if(argc < 2){
        std::cout << "usage: [file name]" << std::endl;
    }

    Orderbook M_OrderBook;
    M_OrderBook.set_trade_callback([](int orderid, int price, int volume)->void{
        std::cout << "[trade]order_id:" << orderid << " \tprice:" << price << " \tvolume:" << volume << std::endl;
    });


    std::ifstream f(argv[1]);
    for_each_update(f,[&](char action, int orderid, char side, int price, int volume){
        if(action == 'A'){
            M_OrderBook.add_order(orderid,side,price,volume);
        }else{
            M_OrderBook.delete_order(orderid);
        }
        // std::cout << M_OrderBook << std::endl;
    });
    std::cout << M_OrderBook << std::endl;
    
    return 0;
}