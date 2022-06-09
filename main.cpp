#include <vector>
#include <memory>
#include <optional>
#include <iostream>
#include <functional>
#include <string>
#include <unordered_map>
#include <array>
#include <algorithm>

template<typename Container>
void print_container(const Container& container) {
    for(auto&& elem : container) {
        std::cout << elem << ",";
    }

    std::cout << "\n";
}

template<typename T>
struct Elem {
    Elem(const T& value, std::shared_ptr<Elem<T>> next_element) 
        : v(value)
        , next(next_element) {}

    T v;
    std::shared_ptr<Elem<T>> next;
};

template<typename T>
class Stack {
    private:
        std::shared_ptr<Elem<T>> m_head;

    public:
        void push(const T& elem) {
            auto new_head = std::make_shared<Elem<T>>(elem, m_head);
            m_head = new_head;
        }

        std::optional<T> pop() {
            if (!m_head) {
                return std::nullopt;
            }

            auto new_head = m_head->next;
            auto current_head = m_head;
            m_head = new_head;

            return std::optional<T>{current_head->v};
        }

        std::optional<T> peek() const {
            if (!m_head) {
                return std::nullopt;
            }

            return std::optional<T>{m_head->v};
        }
};

template<typename T>
struct IndexedElem {
    T value;
    size_t index;
};

std::vector<int> find_next_prices(const std::vector<int>& prices) {
    auto result = std::vector<int>(prices.size(), 0);

    Stack<IndexedElem<int>> stack{};
    for(size_t i = prices.size(); i > 0; i--) {
        auto last_value = stack.peek();
        while(last_value) {
            if (prices[i-1] < last_value->value) {
                result[i-1] = last_value->index;
                break;                
            }

            stack.pop();
            last_value = stack.peek();
        }

        stack.push({prices[i-1], i-1});
    }

    return result;
}

template<typename T>
class Heap {
    public:
        Heap() {}

        void from(const std::vector<T>& data) {
            m_data = data;
            for(size_t i = m_data.size()/2; i > 0; i--) {
                heapify(i);
            }
        }

        void push(const T& elem) {
            m_data.push_back(elem);
            heapify(m_data.size());
        }

        T pop() {
            auto result = m_data[0];
            m_data[0] = m_data.back();
            m_data.pop_back();
            heapify(1);
            return result;
        }

        std::optional<T> peek() const {
            if (this->size() == 0) {
                return std::nullopt;
            }
            
            return m_data[0];
        }

        size_t size() const {
            return m_data.size();
        }

    protected:
        std::vector<T> m_data;

        virtual void heapify(size_t index) = 0;
};

template<typename T, class Compare = std::greater<T>>
class MaxHeap : public Heap<T> {
    public:
        MaxHeap() : Heap<T>() {}

        MaxHeap(const std::vector<T>& v) : Heap<T>(v) {}

    protected:
        void heapify(size_t index) override {
            while(index <= Heap<T>::m_data.size()/2) {
                auto child_index_left = index*2;
                auto child_index_right = index*2 + 1;

                size_t max_index = index;
                if (child_index_left <= Heap<T>::m_data.size()) {
                    if (m_compare(Heap<T>::m_data[child_index_left-1], Heap<T>::m_data[max_index-1])) {
                        max_index = child_index_left;
                    }
                }

                if (child_index_right <= Heap<T>::m_data.size()) {
                    if (m_compare(Heap<T>::m_data[child_index_right-1], Heap<T>::m_data[max_index-1])) {
                        max_index = child_index_right;
                    }
                }

                if (max_index != index) {
                    std::swap(Heap<T>::m_data[max_index-1], Heap<T>::m_data[index-1]);
                    index = max_index;
                } else {
                    break;
                }
            }
        }

    private:
        Compare m_compare;
};

template<typename T, class Compare = std::less<T>>
class MinHeap : public Heap<T> {
    public:
        MinHeap() : Heap<T>() {}

        MinHeap(const std::vector<T>& v) : Heap<T>(v) {}

    protected:
        void heapify(size_t index) override {
            while(index <= Heap<T>::m_data.size()/2) {
                auto child_index_left = index*2;
                auto child_index_right = index*2 + 1;

                size_t max_index = index;
                if (child_index_left <= Heap<T>::m_data.size()) {
                    if(m_compare(Heap<T>::m_data[child_index_left-1], Heap<T>::m_data[max_index-1])) {
                        max_index = child_index_left;
                    }
                }

                if (child_index_right <= Heap<T>::m_data.size()) {
                    if (m_compare(Heap<T>::m_data[child_index_right-1], Heap<T>::m_data[max_index-1])) {
                        max_index = child_index_right;
                    }
                }

                if (max_index != index) {
                    std::swap(Heap<T>::m_data[max_index-1], Heap<T>::m_data[index-1]);
                    index = max_index;
                } else {
                    break;
                }
            }
        }
    
    private:
        Compare m_compare;    
};

/*
Stock exchange matching engine. Continuous limit order book: traders continuously post bids to buy or sell stock. 
A limit order means that a buyer (seller) places an order to buy (sell) a specified amount of a given stock at or below) 
(at or above) a given price. The order book displays buy and sell orders, and ranks them by price and then by time. 
Matching engine matches compatible buyers and sellers; if there are multiple possible buyers, break ties by choosing the 
buyer that placed the bid earliest. Use two priority queues for each stock, one for buyers and one for sellers. 
*/
enum OrderType : uint8_t {
    Buy = 1,
    Sell = 2
};

struct Order {
    uint64_t id;
    uint32_t price;
    uint32_t quantity;
    OrderType type;
    uint64_t time_unix_nano;
};

struct OrderLessComparator {
    bool operator()(const Order& o1, const Order& o2) {
        if (o1.price == o2.price) {
            return o1.time_unix_nano < o2.time_unix_nano;
        }

        return o1.price < o2.price;
    }
};

struct OrderGreaterComparator {
    bool operator()(const Order& o1, const Order& o2) {
        if (o1.price == o2.price) {
            return o1.time_unix_nano < o2.time_unix_nano;
        }

        return o1.price > o2.price;
    }
};

class MatchingEngine {
    public:
        void add(const Order& order) {
            if (order.type == OrderType::Buy) {
                auto min_sell_order = m_sell_orders.peek();

                uint32_t quantity = order.quantity;
                while (min_sell_order) {
                    if (order.price < min_sell_order->price) {
                        break;
                    }

                    if (min_sell_order->quantity >= quantity) {
                        std::cout << "Matched " << quantity << " shares of " << min_sell_order->id << std::endl;

                        min_sell_order->quantity -= quantity;
                        quantity = 0;     
                    } else {
                        std::cout << "Matched " << min_sell_order->quantity << " shares of " << min_sell_order->id << std::endl;

                        quantity -= min_sell_order->quantity;
                        min_sell_order->quantity = 0;
                    }

                    if (min_sell_order->quantity == 0) {
                        m_sell_orders.pop();
                    }

                    if (quantity == 0) {
                        break;
                    }

                    min_sell_order = m_sell_orders.peek();
                }

                if (quantity > 0) {
                    auto unfilled_order = order;
                    unfilled_order.quantity = quantity;
                    m_buy_orders.push(std::move(unfilled_order));
                }

                return;
            }

            if (order.type == OrderType::Sell) {
                auto max_buy_order = m_buy_orders.peek();

                uint32_t quantity = order.quantity;
                while (max_buy_order) {
                    if (order.price > max_buy_order->price) {
                        break;
                    }

                    if (max_buy_order->quantity >= order.quantity) {
                        std::cout << "Matched " << order.quantity << " shares of " << max_buy_order->id << std::endl;

                        max_buy_order->quantity -= order.quantity;
                        quantity = 0;
                    } else {
                        std::cout << "Matched " << max_buy_order->quantity << " shares of " << max_buy_order->id << std::endl;

                        quantity -= max_buy_order->quantity;
                        max_buy_order->quantity = 0;
                    }

                    if (max_buy_order->quantity == 0) {
                        m_buy_orders.pop();
                    }

                    if (quantity == 0) {
                        break;
                    }

                    max_buy_order = m_buy_orders.peek();
                }

                if (quantity > 0) {
                    auto unfilled_order = order;
                    unfilled_order.quantity = quantity;
                    m_sell_orders.push(std::move(unfilled_order));
                }

                return;
            }
        }

    private:
        MinHeap<Order, OrderLessComparator> m_sell_orders;
        MaxHeap<Order, OrderGreaterComparator> m_buy_orders;
};

template<typename T>
struct Node {
    Node(const T& data) : data(data) {}

    T data;
    std::shared_ptr<Node<T>> left;
    std::shared_ptr<Node<T>> right;
    std::shared_ptr<Node<T>> parent;
};


template<typename T>
std::shared_ptr<Node<T>> leftmost_bst(std::shared_ptr<Node<T>> node) {
    while (node->left) {
        node = node->left;
    }

    return node;
}

template<typename T>
std::shared_ptr<Node<T>> successor_bst(std::shared_ptr<Node<T>> node) {
    if (node->right) {
        return leftmost_bst(node->right);
    }

    if (node->parent && node->parent->right != node) {
        return node->parent;
    }

    return nullptr;
}

template<typename T>
class BinarySearchTreeIterator {
    public:
        using self_type = BinarySearchTreeIterator;
        using value_type = T;
        typedef int difference_type;
        typedef std::forward_iterator_tag iterator_category;

        BinarySearchTreeIterator() {}

        BinarySearchTreeIterator(std::shared_ptr<Node<T>> ptr) : m_ptr(ptr) { }

        self_type operator++() { 
            if (!m_ptr) {
                return *this;
            }

            auto next = successor_bst(m_ptr);
            m_ptr = next;

            return *this;
        }

        self_type operator++(int) {
            if (!m_ptr) {
                return *this;
            }

            auto next = successor_bst(m_ptr);
            m_ptr = next;

            return *this; 
        }

        const value_type operator*() { return m_ptr->data; }

        const value_type operator->() { return m_ptr->data; }
        
        bool operator==(const self_type& rhs) { 
            if (!m_ptr || !rhs.m_ptr) {
                return false;
            }

            return m_ptr->data == rhs.m_ptr->data; 
        }

        bool operator!=(const self_type& rhs) { 
            if (!m_ptr && !rhs.m_ptr) {
                return false;
            } else if (!m_ptr || !rhs.m_ptr) {
                return true;
            }

            return m_ptr->data != rhs.m_ptr->data; 
        } 

private:
    std::shared_ptr<Node<T>> m_ptr;
};

template<typename T>
class BinarySearchTree {
    private:
        std::shared_ptr<Node<T>> m_root;

        void add_internal(std::shared_ptr<Node<T>> parent, std::shared_ptr<Node<T>>& node, const T& data) {
            if (!node) {
                node = std::make_shared<Node<T>>(data);
                node->parent = parent;
                return;
            } else if (node->data == data) {
                return;
            }

            if (node->data > data) {
                add_internal(node, node->left, data);
            } else {
                add_internal(node, node->right, data);
            }
        }

    public:
        BinarySearchTree() {
            m_root = nullptr;
        }

        void add(const T& data) {
            add_internal(nullptr, m_root, data);
        }

        BinarySearchTreeIterator<T> begin() const
        {
            auto node = leftmost_bst(m_root);  
            return BinarySearchTreeIterator<T>(node);
        }

        BinarySearchTreeIterator<T> end() const
        {
            return BinarySearchTreeIterator<T>();
        }
};

//
// Flow task
//

class IPNetAddress {
    private:
        std::array<std::uint8_t, 4> octets;
        std::uint8_t mask;
        
    public:
        IPNetAddress(const std::string& s) {
            size_t cidr_delim_index = s.size();
            auto pos = s.find("/");
            if (pos == std::string::npos) {
                mask = 32;
            } else {       
                cidr_delim_index = pos;

                auto cidr_block_str = s.substr(cidr_delim_index+1, s.size());
                auto cidr_block = std::stoi(cidr_block_str, 0, 10);
                if (cidr_block > 32 || cidr_block < 0) {
                    throw std::runtime_error("CIDR block is larger than 32 or lower than 0");
                }

                mask = std::uint8_t(cidr_block);
            }

            size_t last_pos = 0;
            size_t addr_index = 0;
            while ((pos = s.find(".", last_pos)) != std::string::npos) {
                auto octet_str = s.substr(last_pos, pos);
                auto octet = std::stoi(octet_str, 0, 10);
                octets[addr_index] = std::uint8_t(octet);

                addr_index++;
                last_pos = pos + 1;
            }

            auto octet_str = s.substr(last_pos, cidr_delim_index);
            auto octet = std::stoi(octet_str, 0, 10);
            octets[addr_index] = std::uint8_t(octet);
        }

        bool contains(const IPNetAddress& ip) {
            if (mask > ip.mask) {
                return false;
            }

            std::uint8_t remaining_bits = mask;
            size_t index = 0;
            while (remaining_bits != 0) {
                std::uint8_t bits_to_check = std::min(std::uint8_t(8), remaining_bits);
                std::uint8_t masked_octet_first = ((2 ^ bits_to_check) - 1) & this->octets[index];
                std::uint8_t masked_octet_second = ((2 ^ bits_to_check) - 1) & ip.octets[index];
                if (masked_octet_first != masked_octet_second) {
                    return false;
                }

                index++;
                remaining_bits -= bits_to_check;
            }

            return true;
        }
};

struct Flow {
    std::string address;
    int bps;
};

bool is_address_in_cidr(const std::string& address, const std::string& cidr) {
    auto pos = cidr.find("/");
    if (pos == std::string::npos) {
        // TODO
        throw std::runtime_error("invalid cidr");
    }

    IPNetAddress cidr_ip_block(cidr);
    IPNetAddress src_ip_addr(address);

    return cidr_ip_block.contains(src_ip_addr);
}

std::vector<Flow> aggregate_flows(const std::vector<Flow>& flows, const std::vector<std::string>& routes) {
    std::unordered_map<std::string, int> aggregated_map{};

    for (auto flow : flows) {
        for (auto route : routes) {
            if (is_address_in_cidr(flow.address, route)) {
                aggregated_map[route] += flow.bps;
            }
        }
    }

    std::vector<Flow> result{};
    for (auto& pair : aggregated_map) {
        result.push_back(Flow{pair.first, pair.second});
    }

    std::sort(result.begin(), result.end(), [](const Flow& f1, const Flow& f2) {
        if (f1.bps == f2.bps) {
            IPNetAddress f1_addr(f1.address);
            IPNetAddress f2_addr(f2.address);

            return f2_addr.contains(f1_addr);
        }

        return f1.bps > f2.bps;
    });

    return result;
}

int main() {
    auto flows = std::vector<Flow>{};
    flows.push_back({"192.168.0.1", 100});
    flows.push_back({"192.168.10.1", 50});
    flows.push_back({"10.0.0.0", 200});

    auto routes = std::vector<std::string>{
        "192.168.0.0/16",
        "192.168.0.0/24",
        "10.0.0.0/8",
    };

    auto agg = aggregate_flows(flows, routes);

    for (auto flow : agg) {
        std::cout << flow.address << ", " << flow.bps << "\n";
    }

    return 0;
}
