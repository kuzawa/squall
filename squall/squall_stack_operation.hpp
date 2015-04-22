// -*- coding: utf-8-unix -*-

#ifndef SQUALL_STACK_OPERATION_HPP_
#define SQUALL_STACK_OPERATION_HPP_

#include <squirrel.h>
#include "squall_exception.hpp"
#include "squall_closure_decl.hpp"

//#include "demangle.hpp"

namespace squall {

namespace detail {

////////////////////////////////////////////////////////////////
// construct object on stack
template <class T>
void construct_object(HSQUIRRELVM vm, const T& v) {
    SQUserPointer p = sq_newuserdata(vm, sizeof(T*));
    *((T**)p) = new T(v);
    SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
        delete *((T**)p);
        return 0;
    };
    sq_setreleasehook(vm, -1, hook);
}

////////////////////////////////////////////////////////////////
// push
template <class T> inline
void push_aux(HSQUIRRELVM vm, T v) {
    HSQOBJECT sqo;
    if (klass_table(vm).find_klass_object<T>(sqo)) {
        sq_pushobject(vm, sqo);
        sq_createinstance(vm, -1);
        sq_remove(vm, -2);
        sq_setinstanceup(vm, -1, new T(v));
        SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
            delete (T*)p;
            return 0;
        };
        sq_setreleasehook(vm, -1, hook);
    } else {
        SQUserPointer p = sq_newuserdata(vm, sizeof(T));
        new(p) T(v);
        SQRELEASEHOOK hook = [](SQUserPointer p, SQInteger)->SQInteger {
            ((T*)p)->~T();
            return 0;
        };
        sq_setreleasehook(vm, -1, hook);
    }
}
template <class T> inline
void push_aux(HSQUIRRELVM vm, T* v) {
    HSQOBJECT sqo;
    if (klass_table(vm).find_klass_object<T>(sqo)) {
        sq_pushobject(vm, sqo);
        sq_createinstance(vm, -1);
        sq_setinstanceup(vm, -1, v);
        sq_remove(vm, -2);
    } else {
        sq_pushuserpointer(vm, v);
    }
}

template <class R, class... A>
void push_closure(HSQUIRRELVM vm, std::function<R (A...)> v);

template <class R, class... A> inline
void push_aux(HSQUIRRELVM vm, std::function<R (A...)> v) {
    push_closure(vm, v);
}

template <> inline
void push_aux<std::int8_t>(HSQUIRRELVM vm, std::int8_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::int16_t>(HSQUIRRELVM vm, std::int16_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::int32_t>(HSQUIRRELVM vm, std::int32_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::int64_t>(HSQUIRRELVM vm, std::int64_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint8_t>(HSQUIRRELVM vm, std::uint8_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint16_t>(HSQUIRRELVM vm, std::uint16_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint32_t>(HSQUIRRELVM vm, std::uint32_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<std::uint64_t>(HSQUIRRELVM vm, std::uint64_t v) {
    sq_pushinteger(vm, static_cast<SQInteger>(v));
}
template <> inline
void push_aux<float>(HSQUIRRELVM vm, float v) {
    sq_pushfloat(vm, v);
}
template <> inline
void push_aux<bool>(HSQUIRRELVM vm, bool v) {
    sq_pushbool(vm, v ? SQTrue : SQFalse);
}
template <> inline
void push_aux<string_wrapper>(HSQUIRRELVM vm, string_wrapper v) {
    sq_pushstring(vm, v, -1);
}

template <class T> inline
void push(HSQUIRRELVM vm, const T& v) {
    push_aux(vm, wrap_type(v));
}

////////////////////////////////////////////////////////////////
// fetch
enum class FetchContext {
    Argument,
    ReturnValue,
    TableEntry,
	ArrayEntry,
    YieldedValue,
};

template <FetchContext> string fetch_context_string();

template <>
inline string fetch_context_string<FetchContext::Argument>() {
    return "argument";
}

template <>
inline string fetch_context_string<FetchContext::ReturnValue>() {
    return "return value";
}

template <>
inline string fetch_context_string<FetchContext::TableEntry>() {
    return "table entry";
}

template <>
inline string fetch_context_string<FetchContext::ArrayEntry>() {
	return "array entry";
}

template <>
inline string fetch_context_string<FetchContext::YieldedValue>() {
    return "yielded value";
}

template <FetchContext FC>
struct ExceptionError {
	static bool check_argument_type(HSQUIRRELVM vm, SQInteger index, SQObjectType t) {
		SQObjectType at = sq_gettype(vm, index);
		if (at != t) {
			throw_error(
								 fetch_context_string<FC>() + " must be " + get_type_text(t) +
								 ", actual value is " + get_type_text(at));
			return false;
		}
		return true;
	}
	static void throw_error(const std::string& s) {
		throw squirrel_error(s);
	}
};

template <FetchContext FC>
struct FallthroughError {
	static bool check_argument_type(HSQUIRRELVM vm, SQInteger index, SQObjectType t) {
		SQObjectType at = sq_gettype(vm, index);
		return (at == t);
	}
	static void throw_error(const std::string& s) {
	}
};

template <FetchContext FC>
void check_argument_type(HSQUIRRELVM vm, SQInteger index, SQObjectType t) {
	ExceptionError<FC>(vm, index, t);
}

template <FetchContext FC>
bool test_argument_type(HSQUIRRELVM vm, SQInteger index, SQObjectType t) {
	SQObjectType at = sq_gettype(vm, index);
	if (at != t) {
		return false;
	}
	return true;
}

//template <class T, FetchContext FC, class F>
//T getdata(HSQUIRRELVM vm, SQInteger index, SQObjectType t, F f) {
//    check_argument_type<FC>(vm, index, t);
//    T r;
//    f(vm, index, &r);
//    return r;
//}

template <class T, FetchContext FC, class F, class ErrorRecovery>
bool getdata2(HSQUIRRELVM vm, SQInteger index, SQObjectType t, F f, T* v) {
	if ( ErrorRecovery::check_argument_type(vm, index, t) ) {
		f(vm, index, v);
		return true;
	}
	return false;
}

template <class T, FetchContext FC, class ErrorRecovery>
struct Fetch {
public:
//    static T doit(HSQUIRRELVM vm, SQInteger index) {
//        check_argument_type<FC>(vm, index, OT_USERDATA);
//        SQUserPointer r;
//        sq_getuserdata(vm, index, &r, NULL);
//        return **((T**)r);
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, T v) {
		if ( ErrorRecovery::check_argument_type(vm, index, OT_USERDATA) ) {
			SQUserPointer r;
			sq_getuserdata(vm, index, &r, NULL);
			v = **((T**)r);
			return true;
		}
		return false;
	}
};

template <class T, FetchContext FC, class ErrorRecovery>
struct Fetch<T*, FC, ErrorRecovery> {
//    static T* doit(HSQUIRRELVM vm, SQInteger index) {
//        HSQOBJECT sqo;
//        if (klass_table(vm).find_klass_object<T>(sqo)) {
//            check_argument_type<FC>(vm, index, OT_INSTANCE);
//            SQUserPointer r;
//            sq_getinstanceup(vm, index, &r, NULL);
//            return (T*)r;
//        } else {
//            return (T*)getdata<SQUserPointer, FC>(
//                vm, index, OT_USERPOINTER, sq_getuserpointer);
//        }
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, T* v) {
		HSQOBJECT sqo;
		if (klass_table(vm).find_klass_object<T>(sqo)) {
			if ( ErrorRecovery::check_argument_type(vm, index, OT_INSTANCE) ) {
				SQUserPointer r;
				sq_getinstanceup(vm, index, &r, NULL);
				*v = (T*)r;
				return true;
			}
		} else {
			return getdata2<SQUserPointer, FC, ErrorRecovery>(
				vm, index, OT_USERPOINTER, sq_getuserpointer, v);
		}
	}
};

template <class R, class... A, FetchContext FC, class ErrorRecovery>
struct Fetch<std::function<R (A...)>, FC, ErrorRecovery> {
//    static std::function<R (A...)> doit(HSQUIRRELVM vm, SQInteger index) {
//        auto t = sq_gettype(vm, index);
//        if (t == OT_NATIVECLOSURE || t == OT_CLOSURE) {
//            return Closure<R (A...)>(vm, index);
//        } else {
//            throw squirrel_error("value must be closure or native closure");
//        }
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, std::function<R (A...)>& v) {
        auto t = sq_gettype(vm, index);
        if (t == OT_NATIVECLOSURE || t == OT_CLOSURE) {
            v = Closure<R (A...)>(vm, index);
			return true;
        } else {
			ErrorRecovery::throw_error("value must be closure or native closure");
        }
		return false;
	}
};

template <FetchContext FC, class Int, class ErrorRecovery>
struct FetchInt {
//    static Int doit(HSQUIRRELVM vm, SQInteger index) {
//        return static_cast<Int>(
//            getdata<SQInteger, FC, ErrorRecovery>(vm, index, OT_INTEGER, sq_getinteger));
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, Int& v) {
		SQInteger r;
		bool ret = getdata2<SQInteger, FC, ErrorRecovery>(vm, index, OT_INTEGER, sq_getinteger, &r);
		v = static_cast<Int>(r);
		return ret;
	}
};

template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::int8_t, FC, ErrorRecovery> : public FetchInt<FC, std::int8_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::int16_t, FC, ErrorRecovery> : public FetchInt<FC, std::int16_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::int32_t, FC, ErrorRecovery> : public FetchInt<FC, std::int32_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::int64_t, FC, ErrorRecovery> : public FetchInt<FC, std::int64_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::uint8_t, FC, ErrorRecovery> : public FetchInt<FC, std::uint8_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::uint16_t, FC, ErrorRecovery> : public FetchInt<FC, std::uint16_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::uint32_t, FC, ErrorRecovery> : public FetchInt<FC, std::uint32_t, ErrorRecovery> {};
template <FetchContext FC, class ErrorRecovery>
struct Fetch<std::uint64_t, FC, ErrorRecovery> : public FetchInt<FC, std::uint64_t, ErrorRecovery> {};

template <FetchContext FC, class ErrorRecovery>
struct Fetch<float, FC, ErrorRecovery> {
//    static float doit(HSQUIRRELVM vm, SQInteger index) {
//		SQObjectType at = sq_gettype(vm, index);
//		if ( at == OT_INTEGER ) {
//			return getdata<SQInteger, FC>(vm, index, OT_INTEGER, sq_getinteger);
//		}
//		return getdata<SQFloat, FC>(vm, index, OT_FLOAT, sq_getfloat);
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, float& v) {
		SQObjectType at = sq_gettype(vm, index);
		if ( at == OT_INTEGER ) {
			return getdata2<SQInteger, FC, ErrorRecovery>(vm, index, OT_INTEGER, sq_getinteger, v);
		}
		return getdata2<SQFloat, FC, ErrorRecovery>(vm, index, OT_FLOAT, sq_getfloat, v);
	}
};

template <FetchContext FC, class ErrorRecovery>
struct Fetch<bool, FC, ErrorRecovery> {
//    static bool doit(HSQUIRRELVM vm, SQInteger index) {
//        return getdata<SQBool, FC>(
//            vm, index, OT_BOOL, sq_getbool);
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, bool& v) {
		return getdata2<SQBool, FC, ErrorRecovery>(
		   vm, index, OT_BOOL, sq_getbool, v);
	}
};

template <FetchContext FC, class ErrorRecovery>
struct Fetch<HSQOBJECT, FC, ErrorRecovery> {
//	static HSQOBJECT doit(HSQUIRRELVM vm, SQInteger index, SQObjectType type) {
//		return getdata<HSQOBJECT, FC>(
//		vm, index, type, sq_getstackobj);
//	}
	static bool doit2(HSQUIRRELVM vm, SQInteger index, SQObjectType type, HSQOBJECT& v) {
		return true;
//		return getdata2<HSQOBJECT, FC, ErrorRecovery>(
//				  vm, index, type, sq_getstackobj, &v);
	}
};

template <FetchContext FC, class ErrorRecovery>
struct Fetch<string_wrapper, FC, ErrorRecovery> {
//    static string_wrapper doit(HSQUIRRELVM vm, SQInteger index) {
//        return getdata<const SQChar*, FC>(
//            vm, index, OT_STRING, sq_getstring);
//    }
	static bool doit2(HSQUIRRELVM vm, SQInteger index, string_wrapper& v) {
		const SQChar* p = nullptr;
		bool ret = true;
//		bool ret = getdata2<const SQChar*, FC, ErrorRecovery>(
//            vm, index, OT_STRING, sq_getstring, &p);
		if ( p ) {
			v = string_wrapper(p);
		}
		return ret;
	}
};

template <class T, FetchContext FC, class ErrorRecovery=ExceptionError<FC>>
typename wrapped_type<T>::wrapper_type
fetch(HSQUIRRELVM vm, SQInteger index) {
    return Fetch<typename wrapped_type<T>::wrapper_type, FC, ErrorRecovery>::doit(vm, index);
}

template <class T, FetchContext FC, class ErrorRecovery=ExceptionError<FC>>
bool fetch2(HSQUIRRELVM vm, SQInteger index, T& v) {
	typename wrapped_type<T>::wrapper_type t(v);
	bool ret = Fetch<typename wrapped_type<T>::wrapper_type, FC, ErrorRecovery>::doit2(vm, index, t);
	v = t;
	return ret;
}

template <FetchContext FC, class ErrorRecovery=ExceptionError<FC>>
bool fetch2_obj(HSQUIRRELVM vm, SQInteger index, HSQOBJECT& v, SQObjectType type) {
	typename wrapped_type<HSQOBJECT>::wrapper_type t(v);
	bool ret = Fetch<typename wrapped_type<HSQOBJECT>::wrapper_type, FC, ErrorRecovery>::doit2(vm, index, type, t);
	v = t;
	return ret;
}

}

}

#endif // SQUALL_STACK_OPERATION_HPP_
