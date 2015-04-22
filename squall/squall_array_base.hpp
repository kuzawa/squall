#ifndef SQUALL_ARRAY_BASE_HPP_
#define SQUALL_ARRAY_BASE_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"

namespace squall {

class ArrayBase {
public:
    ArrayBase() : vm_(nullptr) {
        sq_resetobject(&arrayobj_);
    }

    ArrayBase(HSQUIRRELVM vm, const HSQOBJECT& to) : vm_(vm), arrayobj_(to) {
        if ( !sq_isnull(arrayobj_) ) {
            assert(vm_);
            sq_addref(vm_, &arrayobj_);
        }
    }

    ArrayBase(const ArrayBase& o) {
        vm_ = o.vm_;
        arrayobj_ = o.arrayobj_;
        if ( !sq_isnull(arrayobj_) ) {
            sq_addref(vm_, &arrayobj_);
        }
    }

    virtual ~ArrayBase() {
        if ( !sq_isnull(arrayobj_) ) {
            sq_release(vm_, &arrayobj_);
        }
    }

    ArrayBase& operator=(const ArrayBase& o) {
        if ( this != &o ) {
            HSQUIRRELVM tmpvm = vm_;
            HSQOBJECT tmpobj = arrayobj_;
            vm_ = o.vm_;
            arrayobj_ = o.arrayobj_;
            // thisが違っても、同じテーブルインスタンスをさしているかもしれない
            if ( !sq_isnull(arrayobj_) ) {
                sq_addref(vm_, &arrayobj_);
            }
            if ( !sq_isnull(tmpobj) ) {
                sq_release(tmpvm, &tmpobj);
            }
        }
        return *this;
    }

    template <class T>
    T get(SQInteger idx) {
		keeper k(vm_);
		sq_pushobject(vm_, arrayobj_);
		sq_pushinteger(vm_, idx);
		if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
			throw squirrel_error("array index '" + std::to_string(idx) + "' out of bound");
		}
		return T(detail::fetch<T, detail::FetchContext::TableEntry>(vm_, -1));
    }

    template <class T>
    bool get(SQInteger idx, T& r) {
		keeper k(vm_);
        sq_pushobject(vm_, arrayobj_);
        sq_pushinteger(vm_, idx);
        if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
            return false;
        }
        return detail::store<T, detail::FetchContext::ArrayEntry>(vm_, -1, r);
    }

    SQInteger size() const {
		keeper k(vm_);
        sq_pushobject(vm_, arrayobj_);
		return sq_getsize(vm_, -1);
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
	keeper k(vm_);
	sq_pushobject(vm_, tableobj_);
	sq_pushstring(vm_, name.data(), name.length());
	if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
		throw squirrel_error("slot '" + name + "' not found");
	}
	HSQOBJECT o = detail::fetch_obj<detail::FetchContext::TableEntry>(vm_, -1, OT_ARRAY);
	return ArrayBase(vm_, o);
}

template <>
inline bool TableBase::get(const string& name, ArrayBase& r) {
	keeper k(vm_);
	sq_pushobject(vm_, tableobj_);
	sq_pushstring(vm_, name.data(), name.length());
	if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
		return false;
	}
	HSQOBJECT o;
	bool ret = detail::store_obj<detail::FetchContext::TableEntry>(vm_, -1, o, OT_ARRAY);
	if ( ret ) {
		r = ArrayBase(vm_, o);
	}
	return ret;
}

template <>
inline TableBase ArrayBase::get(SQInteger idx) {
	keeper k(vm_);
	sq_pushobject(vm_, arrayobj_);
	sq_pushinteger(vm_, idx);
	if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
		throw squirrel_error("array index '" + std::to_string(idx) + "' out of bound");
	}
	HSQOBJECT o = detail::fetch_obj<detail::FetchContext::TableEntry>(vm_, -1, OT_TABLE);
	return TableBase(vm_, o);
}

template <>
inline ArrayBase ArrayBase::get(SQInteger idx) {
	keeper k(vm_);
	sq_pushobject(vm_, arrayobj_);
	sq_pushinteger(vm_, idx);
	if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
		throw squirrel_error("array index '" + std::to_string(idx) + "' out of bound");
	}
	HSQOBJECT o = detail::fetch_obj<detail::FetchContext::TableEntry>(vm_, -1, OT_ARRAY);
	return ArrayBase(vm_, o);
}

}

#endif // SQUALL_ARRAY_BASE_HPP_
