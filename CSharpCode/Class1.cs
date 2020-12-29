using System;
using System.Runtime.CompilerServices;
namespace CSharpCode
{
    public class Class1
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        extern static void PrintMethod(string msg);
        
        static void Main(string[] args)
        {
            Console.WriteLine("This main is being called from cpp");
            PrintMethod("Im Executing this code from CSharp!\n");
        }
    }
}
