#ifndef PYLIST_H
#define PYLIST_H

#include <iostream>
#include <vector>
#include <memory>
#include <set>

class pylist {
private:
    struct ListData;

    static std::set<ListData*>& get_all_lists() {
        static std::set<ListData*> lists;
        return lists;
    }

    static void register_gc() {
        struct GarbageCollector {
            ~GarbageCollector() {
                auto& lists = get_all_lists();
                while (!lists.empty()) {
                    auto it = lists.begin();
                    ListData* ptr = *it;
                    lists.erase(it);
                    auto sp = ptr->shared_from_this();
                    sp->vec.clear();
                }
            }
        };
        static GarbageCollector gc;
    }

    struct ListData : public std::enable_shared_from_this<ListData> {
        std::vector<pylist> vec;
        ListData() {
            get_all_lists().insert(this);
        }
        ~ListData() {
            get_all_lists().erase(this);
        }
    };

    std::shared_ptr<ListData> ptr;
    int val;
    bool is_int;

public:
    pylist() : ptr(std::make_shared<ListData>()), val(0), is_int(false) {
        register_gc();
    }
    
    pylist(int v) : ptr(nullptr), val(v), is_int(true) {
        register_gc();
    }
    
    pylist(const pylist &other) = default;
    
    pylist &operator=(const pylist &other) = default;

    void append(const pylist &x) {
        if (!is_int && ptr) {
            ptr->vec.push_back(x);
        }
    }

    pylist pop() {
        if (!is_int && ptr && !ptr->vec.empty()) {
            pylist back = ptr->vec.back();
            ptr->vec.pop_back();
            return back;
        }
        return pylist();
    }

    pylist &operator[](size_t i) {
        return ptr->vec[i];
    }

    const pylist &operator[](size_t i) const {
        return ptr->vec[i];
    }

    operator int() const {
        return val;
    }

    friend std::ostream &operator<<(std::ostream &os, const pylist &ls) {
        static std::vector<ListData*> visited;
        if (ls.is_int) {
            os << ls.val;
        } else if (!ls.ptr) {
            os << "[]";
        } else {
            for (auto p : visited) {
                if (p == ls.ptr.get()) {
                    os << "[...]";
                    return os;
                }
            }
            visited.push_back(ls.ptr.get());
            os << "[";
            for (size_t i = 0; i < ls.ptr->vec.size(); ++i) {
                if (i > 0) os << ", ";
                os << ls.ptr->vec[i];
            }
            os << "]";
            visited.pop_back();
        }
        return os;
    }
};

#endif //PYLIST_H