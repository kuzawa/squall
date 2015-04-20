#ifndef SQUALL_ARRAY_BASE_HPP_
#define SQUALL_ARRAY_BASE_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"

namespace squall {

class ArrayBase {
public:
    ArrayBase(HSQUIRRELVM vm, const HSQOBJECT& to) : vm_(vm), arrayobj_(to) {}

    template <class T>
    T get(SQInteger idx) {
        T r;
        if(get<T>(idx, r)) {
            return r;
        }
        throw squirrel_error("index '" + std::to_string(idx) + "'is out of range");
    }

    template <class T>
    bool get(SQInteger idx, T& r) {
        sq_pushobject(vm_, arrayobj_);
        sq_pushinteger(vm_, idx);
        if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
            return false;
        }
        r = detail::fetch<T, detail::FetchContext::ArrayEntry>(vm_, -1);
        sq_pop(vm_, 2);
        return true;
    }

    template <class T>
    bool get(SQInteger idx, T& r, SQObjectType type) {
        sq_pushobject(vm_, arrayobj_);
        sq_pushinteger(vm_, idx);
        if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
            return false;
        }
        r = detail::fetch<T, detail::FetchContext::ArrayEntry>(vm_, -1, type);
        sq_pop(vm_, 2);
        return true;
    }

protected:
    HSQUIRRELVM handle() { return vm_; }
    HSQOBJECT&  arrayobj() { return arrayobj_; }

private:
    HSQUIRRELVM vm_;
    HSQOBJECT   arrayobj_;

};

template <>
inline ArrayBase TableBase::get(const string& name) {
    HSQOBJECT obj;
    if (get<HSQOBJECT>(name, obj, OT_ARRAY)) {
        return ArrayBase(vm_, obj);
    }
    throw squirrel_error("slot '" + name + "' not found");
}

template <>
inline TableBase ArrayBase::get(SQInteger idx) {
    HSQOBJECT obj;
    if (get<HSQOBJECT>(idx, obj, OT_TABLE)) {
        return TableBase(vm_, obj);
    }
    throw squirrel_error("index '" + std::to_string(idx) + "'is out of range");
}

}

#endif // SQUALL_ARRAY_BASE_HPP_
