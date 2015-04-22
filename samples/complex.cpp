#include "../squall/squall_vmstd.hpp"
#include "../squall/squall_table.hpp"
#include "../squall/squall_array.hpp"
#include <iostream>

void check1(squall::VM& vm)
{
	squall::TableBase& root_table = vm.root_table();

	try {
		int one = root_table.get<int>("one");
		assert(one==1);
		one=-1;
		root_table.get<int>("one", one);
		assert(one=1);
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		auto one = root_table.get<std::string>("one");
		assert(0);
	}
	catch(squall::squirrel_error& e) {
		puts("ok");
	}

	try {
		std::string s;
		bool ret = root_table.get<std::string>("one", s);
		assert(ret==false);
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		auto s = root_table.get<std::string>("not exist");
		assert(0);
	}
	catch(squall::squirrel_error& e) {
		puts("ok");
	}

	try {
		std::string s;
		bool ret = vm.root_table().get<std::string>("one", s);
		assert(ret==false);
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}
}

void check2(squall::VM& vm)
{
	squall::TableBase& root_table = vm.root_table();
	squall::TableBase foo = root_table.get<squall::TableBase>("foo");

	try {
		foo.get<int>("bar");
		assert(0);
	}
	catch(squall::squirrel_error& e) {
		puts("ok");
	}

	try {
		int bar;
		bool ret = foo.get<int>("bar", bar);
		assert(ret==false);
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	squall::TableBase bar;

	try {
		bool ret = foo.get<squall::TableBase>("bar", bar);
		assert(ret==true);
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		foo.get<float>("baz");
		assert(0);
	}
	catch(squall::squirrel_error& e) {
		puts("ok");
	}

	try {
		float baz;
		bool ret = foo.get<float>("baz", baz);
		assert(ret==false);
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	std::string baz;
	bool ret = bar.get<std::string>("baz", baz);
	assert(ret==true);
	assert(baz=="baz");
}

void check3(squall::VM& vm)
{
	squall::TableBase& root_table = vm.root_table();

	squall::ArrayBase colors;
	try {
		bool ret = root_table.get<squall::ArrayBase>("colors", colors);
		assert(ret);
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		colors = root_table.get<squall::ArrayBase>("colors");
		puts("ok");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		colors.get<int>(0);
		assert(0);
	}
	catch(squall::squirrel_error& e) {
		puts("ok");
	}

	try {
		int out;
		bool ret = colors.get<int>(0, out);
		assert(ret==false);
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		colors.get<std::string>(10);
		assert(0);
	}
	catch(squall::squirrel_error& e) {
		puts("ok");
	}

	try {
		std::string out;
		bool ret = colors.get<std::string>(10, out);
		assert(ret==false);
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		std::string color;
		bool ret = colors.get<std::string>(1, color);
		assert(ret);
		assert(color=="green");
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}

	try {
		std::string color = colors.get<std::string>(1);
		assert(color=="green");
		std::cout << color << std::endl;
	}
	catch(squall::squirrel_error& e) {
		assert(0);
	}
}

int main() {
	try {
		squall::VMStd vm;
		vm.dofile("complex.nut");

		check1(vm);
		check2(vm);
		check3(vm);

//		std::string hello;
//		hello = vm.root_table().get<std::string>("hello");
//		bool ret = vm.root_table().get2<std::string>("hello", hello);
//		std::cout << hello << std::endl;
	}
	catch(squall::squirrel_error& e) {
		std::cerr << e.what() << std::endl;
	}

    return 0;
}



