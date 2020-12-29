#include <iostream>
#include <mono\jit\jit.h>
#include <mono\metadata\assembly.h>

struct Printer
{
	static void PrintMethod(MonoString* string)
	{
		char* cppString = mono_string_to_utf8(string);
		std::cout << cppString;
		mono_free(cppString);
	}
};

int main()
{
	mono_set_dirs("Mono\\lib", "Mono\\etc");
	MonoDomain* domain = mono_jit_init("CSharp_Domain");
	MonoAssembly* csharpAssembly =
		mono_domain_assembly_open(domain, "CSharpCode\\bin\\Debug\\netstandard2.0\\CSharpCode.dll");
	if (!csharpAssembly)
	{
		return -1;
	}

	//Set up internal calls
	mono_add_internal_call("CSharpCode.Class1::PrintMethod", &Printer::PrintMethod);

	int argc = 1;
	char* argv[1] = { (char*)"CSharp" };

	mono_jit_exec(domain, csharpAssembly, argc, argv);

	return 0;
}