#include <initializer_list>
#include <stdexcept>
#include <utility>
#include <vector>

using std::vector;

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
    typedef std::pair<KeyType, ValueType> node;
    typedef std::pair<const KeyType, ValueType> cnode;
    typedef typename std::vector<vector<node>>::iterator out_iter;
    typedef typename std::vector<node>::iterator in_iter;
    typedef typename std::vector<vector<node>>::const_iterator c_out_iter;
    typedef typename std::vector<node>::const_iterator c_in_iter;

    static const size_t REALLOC_SIZE = 2;

    vector<vector<node>> data;
    size_t length, num;
    Hash hasher;

public:
    class iterator {
        out_iter out, o_end;
        in_iter in;

    public:
        iterator() {};
        iterator(out_iter out0, out_iter o_end0) : out(out0), o_end(o_end0) {
            while (out != o_end) {
                in = out->begin();
                if (in != out->end())
                    break;
                ++out;
            }
            if (out == o_end) {
                in = in_iter();
            }
        }

        iterator& operator++() {
            ++in;
            if (in == out->end()) {
                while (out != o_end) {
                    ++out;
                    if (out != o_end) {
                        in = out->begin();
                        if (in != out->end())
                            break;
                    }
                }
            }
            if (out == o_end) {
                in = in_iter();
            }
            return *this;
        }

        iterator operator++(int) {
            iterator copy = *this;
            ++(*this);
            return copy;
        }

        friend bool operator==(const iterator& a, const iterator& b) {
            return a.out == b.out && a.in == b.in;
        }

        friend bool operator!=(const iterator& a, const iterator& b) {
            return !(a == b);
        }

        cnode& operator*() {
            return reinterpret_cast<cnode&>(*in);
        }

        cnode* operator->() {
            return reinterpret_cast<cnode*>(&*in);
        }
    };

    
    class const_iterator {
        c_out_iter out, o_end;
        c_in_iter in;

    public:
        const_iterator() {};
        const_iterator(c_out_iter out0, c_out_iter o_end0) : out(out0), o_end(o_end0) {
            while (out != o_end) {
                in = out->begin();
                if (in != out->end())
                    break;
                ++out;
            }
            if (out == o_end) {
                in = c_in_iter();
            }
        }

        const_iterator& operator++() {
            ++in;
            if (in == out->end()) {
                while (out != o_end) {
                    ++out;
                    if (out != o_end) {
                        in = out->begin();
                        if (in != out->end())
                            break;
                    }
                }
            }
            if (out == o_end) {
                in = c_in_iter();
            }
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator copy = *this;
            ++(*this);
            return copy;
        }

        friend bool operator==(const const_iterator& a, const const_iterator& b) {
            return a.out == b.out && a.in == b.in;
        }

        friend bool operator!=(const const_iterator& a, const const_iterator& b) {
            return !(a == b);
        }

        const cnode& operator*() {
            return reinterpret_cast<const cnode&>(*in);
        }

        const cnode* operator->() {
            return reinterpret_cast<const cnode*>(&*in);
        }
    };

    HashMap(Hash hasher0 = Hash()) : hasher(hasher0) {
        length = 10;
        num = 0;
        data.resize(length);
    }

    template<typename TBegin, typename TEnd>
    HashMap(TBegin begin, TEnd end, Hash hasher0 = Hash()) : HashMap(hasher0) {
        *this = HashMap(hasher);
        for (; begin != end; ++begin) {
            insert(*begin);
        }
    }

    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> list, Hash hasher0 = Hash()) : HashMap(list.begin(), list.end(), hasher0) {};

    void insert(const std::pair<KeyType, ValueType>& elem) {
        if (contains(elem.first)) {
            return;
        }

        if (2 * num >= length) {
            resize();
        }

        data[hasher(elem.first) % length].push_back(elem);
        num++;
    }

    void insert(std::pair<KeyType, ValueType>&& elem) {
        if (contains(elem.first)) {
            return;
        }

        if (2 * num >= length) {
            resize();
        }

        data[hasher(elem.first) % length].push_back(elem);
        num++;
    }

    void erase(const KeyType& key) {
        if (!contains(key)) {
            return;
        }

        auto it = data[hasher(key) % length].begin();
        for (; !(it->first == key); ++it);
        data[hasher(key) % length].erase(it);

        num--;
    }

    size_t size() const {
        return num;
    }

    bool empty() const {
        return size() == 0;
    }

    Hash hash_function() const {
        return hasher;
    }

    iterator begin() {
        return iterator(data.begin(), data.end());
    }

    iterator end() {
        return iterator(data.end(), data.end());
    }

    const_iterator begin() const {
        return const_iterator(data.begin(), data.end());
    }

    const_iterator end() const {
        return const_iterator(data.end(), data.end());
    }

    iterator find(const KeyType& key) {
        if (!contains(key)) {
            return end();
        }
        iterator ans(data.begin() + hasher(key) % length, data.end());
        for (; !(ans->first == key); ++ans);
        return ans;
    }

    const_iterator find(const KeyType& key) const {
        if (!contains(key)) {
            return end();
        }
        const_iterator ans(data.begin() + hasher(key) % length, data.end());
        for (; !(ans->first == key); ++ans);
        return ans;
    }

    ValueType& operator[](const KeyType& key) {
        insert({key, ValueType()});
        return find(key)->second;
    }

    const ValueType& at(const KeyType& key) const {
        if (!contains(key)) {
            throw std::out_of_range("No such key");
        }

        return find(key)->second;
    }

    void clear() {
        length = 0;
        num = 0;
        data.clear();
        *this = HashMap(hasher);
    }

private:
    void resize() {
        length *= REALLOC_SIZE;
        vector<vector<node>> temp(length);
        num = 0;
        std::swap(temp, data);

        for (auto& vec : temp) {
            for (auto& p : vec) {
                insert(std::move(p));
            }
        }
    }

    bool contains(const KeyType& key) const {
        for (auto& p : data[hasher(key) % length]) {
            if (p.first == key) {
                return true;
            }
        }
        return false;
    }
};
