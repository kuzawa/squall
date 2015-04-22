#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_table.hpp"
#include "../squall/squall_array.hpp"
#include <iostream>

int main() {
	try {
		squall::VMStd vm;
		vm.dofile("complex.nut");

		auto one = vm.root_table().get<std::string>("one");
		std::cout << one << std::endl;

//		std::string hello;
//		hello = vm.root_table().get<std::string>("hello");
//		bool ret = vm.root_table().get2<std::string>("hello", hello);
//		std::cout << hello << std::endl;

		squall::TableBase foo = vm.root_table().get<squall::TableBase>("foo");
		{
			std::string bar;
			bool ret = foo.get<std::string>("bar", bar);
			assert(ret==false);
		}
		{
			squall::TableBase bar;
			bool ret = foo.get<squall::TableBase>("bar", bar);
			assert(ret==true);
			std::cout << bar.get<std::string>("baz") << std::endl;
		}
		{
			squall::ArrayBase color;
			bool ret = vm.root_table().get<squall::ArrayBase>("color", color);
			assert(ret);
			std::cout << color.get<std::string>(1) << std::endl;
		}
	}
	catch(squall::squirrel_error& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}



