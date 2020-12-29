
#include <iostream>
#include <mono\jit\jit.h>
#include <mono\metadata\loader.h>
#include <mono\metadata\assembly.h>
#include <string>

struct Printer
{
	static void PrintMethod(MonoString* string)
	{
		char* cppString = mono_string_to_utf8(string);
		std::cout << cppString;
		mono_free(cppString);
	}
};

MonoMethod* find_method(MonoClass* klass, const char* name)
{
	MonoMethod* method = nullptr;
	MonoMethod* m = nullptr;
	void* iter = nullptr;
	while ((m = mono_class_get_methods(klass, &iter)))
	{
		if (strcmp(mono_method_get_name(m), name) == 0)
		{
			method = m;
		}
	}
	return method;
}

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
	//Invoking Method
	MonoImage* image = mono_assembly_get_image(csharpAssembly);
	MonoClass* klass = mono_class_from_name(image, "CSharpCode", "MonoBehaviour");
	if (!klass)
	{
		return -1;
	}
	MonoObject* instance = mono_object_new(domain, klass);
	mono_runtime_object_init(instance); //explicitly call constructor
	MonoMethod* method = find_method(klass, "OnStart");
	if (!method)
	{
		return -1;
	}
	mono_runtime_invoke(method, instance, nullptr, nullptr);

	method = find_method(klass, "Update");
	if (!method)
	{
		return -1;
	}
	mono_runtime_invoke(method, instance, nullptr, nullptr);

	//Set up internal calls
	mono_add_internal_call("CSharpCode.Class1::PrintMethod", &Printer::PrintMethod);

	int argc = 1;
	char* argv[1] = { (char*)"CSharp" };

	mono_jit_exec(domain, csharpAssembly, argc, argv);

	mono_jit_cleanup(domain); // clean up all domains
	return 0;
}