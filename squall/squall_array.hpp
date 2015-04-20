#ifndef SQUALL_ARRAY_HPP_
#define SQUALL_ARRAY_HPP_

#include "squall_table_base.hpp"

namespace squall {

class ArrayBase : public ArrayBase {
public:
    Array(VM& vm) : ArrayBase(vm.handle(), make_array(vm.handle())) {}
    ~Array() { sq_release(handle(), &tableobj()); }

private:
    static HSQOBJECT make_array(HSQUIRRELVM vm) {
        HSQOBJECT arrayobj;
        sq_newarray(vm, 1);
        sq_getstackobj(vm, -1, &arrayobj);
        sq_addref(vm, &arrayobj);
        sq_pop(vm, 1);
        return arryobj;
    }

};

}

#endif // SQUALL_ARRAY_HPP_
