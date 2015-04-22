#ifndef SQUALL_TABLE_BASE_HPP_
#define SQUALL_TABLE_BASE_HPP_

#include <squirrel.h>
#include "squall_stack_operation.hpp"
#include "squall_coroutine.hpp"

namespace squall {

class TableBase {
public:
    TableBase() : vm_(nullptr) {
        sq_resetobject(&tableobj_);
    }

    TableBase(HSQUIRRELVM vm, const HSQOBJECT& to) : vm_(vm), tableobj_(to) {
        if ( !sq_isnull(tableobj_) ) {
            assert(vm);
            sq_addref(vm_, &tableobj_);
        }
    }

    TableBase(const TableBase& o) {
        vm_ = o.vm_;
        tableobj_ = o.tableobj_;
        if ( !sq_isnull(tableobj_) ) {
            sq_addref(vm_, &tableobj_);
        }
    }

    virtual ~TableBase() {
        if ( !sq_isnull(tableobj_) ) {
            sq_release(vm_, &tableobj_);
        }
    }

    TableBase& operator=(const TableBase& o) {
        if ( this != &o ) {
            HSQUIRRELVM tmpvm = vm_;
            HSQOBJECT tmpobj = tableobj_;
            vm_ = o.vm_;
            tableobj_ = o.tableobj_;
            // thisが違っても、同じテーブルインスタンスをさしているかもしれない
            if ( !sq_isnull(tableobj_) ) {
                sq_addref(vm_, &tableobj_);
            }
            if ( !sq_isnull(tmpobj) ) {
                sq_release(tmpvm, &tmpobj);
            }
        }
        return *this;
    }

    template <class T>
    void set(const string& name, const T& v) {
        sq_pushobject(vm_, tableobj_);
        sq_pushstring(vm_, name.data(), name.length());
        detail::push(vm_, v);
        sq_newslot(vm_, -3, SQFalse);
        sq_pop(vm_, 1);
    }

    template <class T>
    T get(const string& name) {
		keeper k(vm_);
		sq_pushobject(vm_, tableobj_);
		sq_pushstring(vm_, name.data(), name.length());
		if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
			throw squirrel_error("slot '" + name + "' not found");
		}
		return T(detail::fetch<T, detail::FetchContext::TableEntry>(vm_, -1));
    }

	template <class T>
	bool get(const string& name, T& r) {
		keeper k(vm_);
		sq_pushobject(vm_, tableobj_);
		sq_pushstring(vm_, name.data(), name.length());
		if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
			return false;
		}
		return detail::fetch2<T, detail::FetchContext::TableEntry>(vm_, -1, r);
	}

    template <class R, class... T>
    R call(const string& name, T... args) {
        return detail::call<R>(vm_, tableobj_, name, args...);
    }

    template <class F>
    void defun(const string& name, F f) {
        detail::defun_global(vm_, tableobj_, name, to_function(f));
    }

    void defraw(const string& s, SQInteger (*f)(HSQUIRRELVM)) {
        detail::defraw(vm_, tableobj_, s, f);
    }

    template <class... T>
    Coroutine co_call(const string& name, T... args) {
        SQInteger top = sq_gettop(handle());
        detail::call_setup(vm_, tableobj_, name, args...);
        return Coroutine(vm_, top);
    }

    SQInteger size() const {
        sq_pushobject(vm_, tableobj_);
        SQInteger r = sq_getsize(vm_, -1);
        sq_pop(vm_, 1);
        return r;
    }

protected:
    HSQUIRRELVM handle() { return vm_; }
    HSQOBJECT&  tableobj() { return tableobj_; }

//	bool get(const string& name, HSQOBJECT& r, SQObjectType type) {
//		sq_pushobject(vm_, tableobj_);
//		sq_pushstring(vm_, name.data(), name.length());
//		if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
//			return false;
//		}
//		bool ret = detail::fetch2_obj<detail::FetchContext::TableEntry>(vm_, -1, r, type);
//		sq_pop(vm_, 2);
//		return ret;
//	}

private:
    HSQUIRRELVM vm_;
    HSQOBJECT   tableobj_;

};

template <>
inline TableBase TableBase::get(const string& name) {
	sq_pushobject(vm_, tableobj_);
	sq_pushstring(vm_, name.data(), name.length());
	if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
		throw squirrel_error("slot '" + name + "' not found");
	}
	HSQOBJECT r = detail::fetch_obj<detail::FetchContext::TableEntry>(vm_, -1, OT_TABLE);
	sq_pop(vm_, 2);
	return TableBase(vm_, r);
}

template <>
inline bool TableBase::get(const string& name, TableBase& r) {
	keeper k(vm_);
	sq_pushobject(vm_, tableobj_);
	sq_pushstring(vm_, name.data(), name.length());
	if (!SQ_SUCCEEDED(sq_get(vm_, -2))) {
		return false;
	}
	HSQOBJECT o;
	bool ret = detail::fetch2_obj<detail::FetchContext::TableEntry>(vm_, -1, o, OT_TABLE);
	if ( ret ) {
		r = TableBase(vm_, o);
	}
	return ret;
}

}

#endif // SQUALL_TABLE_BASE_HPP_
