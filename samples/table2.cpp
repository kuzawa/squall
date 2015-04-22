#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_table.hpp"
#include <iostream>

int main() {
	try {
		squall::VMStd vm;
		vm.dofile("table2.nut");

//		int one;
//		bool ret = vm.root_table().get2<int>("one", one);
//		std::cout << one << std::endl;

//		std::string hello;
//		hello = vm.root_table().get<std::string>("hello");
//		bool ret = vm.root_table().get2<std::string>("hello", hello);
//		std::cout << hello << std::endl;

		squall::TableBase foo = vm.root_table().get<squall::TableBase>("foo");
		std::string bar;
		bool ret = foo.get2<std::string>("bar", bar);
		if ( ret ) {
			std::cout << foo.get<std::string>("bar") << std::endl;
		}
	}
	catch(squall::squirrel_error& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}



