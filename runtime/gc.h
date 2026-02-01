#pragma once
#include <vector>
#include <unordered_set>

struct GCObject {
    bool marked = false;
};

class GC {
public:
    std::vector<GCObject*> heap;

    GCObject* alloc() {
        GCObject* obj = new GCObject();
        heap.push_back(obj);
        return obj;
    }

    void mark(GCObject* obj) {
        if (!obj || obj->marked) return;
        obj->marked = true;
    }

    void sweep() {
        auto it = heap.begin();
        while (it != heap.end()) {
            if (!(*it)->marked) {
                delete *it;
                it = heap.erase(it);
            } else {
                (*it)->marked = false;
                ++it;
            }
        }
    }

    void collect(const std::vector<GCObject*>& roots) {
        for (auto* r : roots) mark(r);
        sweep();
    }

    size_t count() const {
        return heap.size();
    }
};
