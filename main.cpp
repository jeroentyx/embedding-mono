#include <iostream>
#include <mono\jit\jit.h>
#include <mono\metadata\loader.h>
#include <mono\metadata\assembly.h>
#include <mono\metadata\environment.h>
#include <mono\metadata\mono-config.h>
#include <string>


//Extras
#include <windows.h>
#include <mono\metadata\mono-gc.h>
#include <mono\metadata\threads.h>



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

MonoAssembly* LoadAssemblyFromFile(const char* filepath)
{
	if (filepath == NULL)
	{
		return NULL;
	}

	HANDLE file = CreateFileA(filepath, FILE_READ_ACCESS, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	DWORD file_size = GetFileSize(file, NULL);
	if (file_size == INVALID_FILE_SIZE)
	{
		CloseHandle(file);
		return NULL;
	}

	void* file_data = malloc(file_size);
	if (file_data == NULL)
	{
		CloseHandle(file);
		return NULL;
	}

	DWORD read = 0;
	ReadFile(file, file_data, file_size, &read, NULL);
	if (file_size != read)
	{
		free(file_data);
		CloseHandle(file);
		return NULL;
	}

	MonoImageOpenStatus status;
	MonoImage* image = mono_image_open_from_data_full(reinterpret_cast<char*>(file_data), file_size, 1, &status, 0);
	if (status != MONO_IMAGE_OK)
	{
		return NULL;
	}
	auto assemb = mono_assembly_load_from_full(image, filepath, &status, 0);
	free(file_data);
	CloseHandle(file);
	mono_image_close(image);
	return assemb;
}


bool LoadRuntimeAssembly(const std::string& path)
{
	MonoDomain* domain = nullptr;
	bool cleanup = false;
	MonoAssembly* assembly = LoadAssemblyFromFile(path.c_str());
	MonoImage* image = mono_assembly_get_image(assembly);

	//Reload all internal calls
	mono_add_internal_call("CSharpCode.Class1::PrintMethod", &Printer::PrintMethod);

	if (cleanup)
	{
		mono_domain_unload(domain);
		return false;
	}
	return true;
}


void ReloadAssembly(const char* path)
{
	LoadRuntimeAssembly(path);
}

int main()
{
	bool isGameRunning = true;
	int retval = 0;
	//Init
	mono_config_parse(NULL);
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

	//Reload
	MonoAssembly* sCoreAssembly = nullptr;
	MonoImage* CoreAssemblyImage = nullptr;
	MonoDomain* appDomain = nullptr;


	while (isGameRunning)
	{
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


		if (GetAsyncKeyState(VK_LCONTROL))
		{
			bool cleanup = false;

			//LoadRunTimeAssembly
			if (domain)
			{
				//App Domain
				char* appDomainName = (char*)"MonoRuntime";
				appDomain = mono_domain_create_appdomain(appDomainName, NULL);
				mono_domain_set(appDomain, false);
				cleanup = true;

			}
			sCoreAssembly = mono_domain_assembly_open(domain, "CSharpCode\\bin\\Debug\\netstandard2.0\\CSharpCode.dll");
			CoreAssemblyImage = mono_assembly_get_image(sCoreAssembly);

			//Set up internal calls		
			mono_add_internal_call("CSharpCode.Class1::PrintMethod", &Printer::PrintMethod);

			if (cleanup)
			{
				//mono_domain_unload(domain);
				domain = appDomain;
			}
			csharpAssembly = sCoreAssembly;
			image = CoreAssemblyImage;
		}


		if (GetAsyncKeyState(VK_ESCAPE))
		{
			isGameRunning = false;
		}

	}
	retval = mono_environment_exitcode_get();
	mono_jit_cleanup(domain); // clean up all domains
	return retval;
}