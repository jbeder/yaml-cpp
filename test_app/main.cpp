// Compile with "g++ main.cpp -lyaml-cpp --std=c++14"
#include <yaml-cpp/yaml.h>

#include <iostream>
#include <memory>
#include <type_traits>

// Base class
class A
{
public:
	A() : a(-1) {}
	A(int a) : a(a) {}

	// virtual load/emit methods
	virtual void load(const YAML::Node &node)
	{
		a = node["a"].as<decltype(a)>();
	}
	virtual YAML::Node emit() const
	{
		YAML::Node node;
		node["a"] = a;
		return node;
	}
	int a;
};

// Derived class
class B : public A
{
public:
	B() : b(-1) {}
	B(int a, int b) : A(a), b(b) {}

	// override virtual load/emit methods
	virtual void load(const YAML::Node &node) override
	{
		A::load(node);
		b = node["b"].as<decltype(b)>();
	}
	virtual YAML::Node emit() const override
	{
		YAML::Node node = A::emit();
		node["b"] = b;
		return node;
	}
	int b;
};

// Implementation of convert::{encode,decode} for all classes derived from or being A
namespace YAML {
	template<typename T> struct convert<T, typename std::enable_if<std::is_base_of<A, T>::value>::type>
	{
		static Node encode(const T &rhs)
		{
			Node node = rhs.emit();
			return node;
		}
		static bool decode(const Node &node, T &rhs)
		{
			try {
				rhs.load(node);
			} catch(...) {
				return false;
			}
			return true;
		}
	};
}

int main(int argc, char **argv)
{
	{ // Polymorphic emit
		std::unique_ptr<A> base_ptr = std::make_unique<B>(1, 2);
		YAML::Node node;
		node.push_back(*base_ptr);
		std::cout << YAML::Dump(node) << std::endl;
	}
	{ // load
		std::unique_ptr<B> derived_ptr = std::make_unique<B>(1, 2);
		*derived_ptr = YAML::Load("a: 3\nb: 4").as<B>();
		std::cout << "a: " << derived_ptr->a << " b: " << derived_ptr->b << std::endl;
	}
	return 0;
}
