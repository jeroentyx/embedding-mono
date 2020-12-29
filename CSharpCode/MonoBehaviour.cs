using System;
using System.Collections.Generic;
using System.Text;

namespace CSharpCode
{
    class MonoBehaviour
    {
        string message = "Hello from Mono c#";
        MonoBehaviour()
        {
            Console.WriteLine("Constructor");
        }
        void OnStart()
        {
            Console.WriteLine("Start");
        }
        void Update()
        {
            Console.WriteLine("OnUpdate");
        }
        string Message
        {
            get
            {
                return message;
            }
        }
    }
}













