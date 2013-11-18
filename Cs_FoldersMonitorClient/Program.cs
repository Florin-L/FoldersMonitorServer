using System;
using System.Reflection;

namespace Cs_FoldersMonitorClient
{
    //
    class FoldersMonitorEventHandler : FoldersMonitorServer.IFoldersMonitorEvents
    {
        public void OnChanged(int action, string fileName)
        {
            Console.WriteLine("action {0} detected on file {1}", action, fileName);
        }
    }

    //
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                // early binding         
                Console.WriteLine("Early binding");

                // Run the following command in console:
                // > tlbimp FoldersMonitorServer.tlb /out:FoldersMonitorServer.dll
                //
                // Add FoldersMonitorServer.dll as a reference to this project.

                FoldersMonitorServer.CoFoldersMonitor fm = new FoldersMonitorServer.CoFoldersMonitor();
                fm.Start(10000);

                // notifications filter: FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE
                string taskId = fm.CreateTask("d:\\tmp", 0x00000001 | 0x00000010);
                Console.WriteLine("task id : {0}", taskId);

                int error = 0;
                fm.StartTask(taskId, out error);

                //
                FoldersMonitorEventHandler handler = new FoldersMonitorEventHandler();
                fm.OnChanged += handler.OnChanged;

                Console.WriteLine("Working ...");
                Console.ReadKey();

                //
                fm.StopTask(taskId, out error);
                fm.Stop();
            }
            catch (Exception e)
            {
                String errorMessage;
                errorMessage = "Error: ";
                errorMessage = String.Concat(errorMessage, e.Message);
                errorMessage = String.Concat(errorMessage, " Line: ");
                errorMessage = String.Concat(errorMessage, e.Source);

                Console.WriteLine(errorMessage);
                return;
            }

            try
            {
                // late binding
                Console.WriteLine("Late binding");

                Type objClass = Type.GetTypeFromProgID("CoFoldersMonitor.1");
                object objFM = Activator.CreateInstance(objClass);

                object[] arguments = null;

                arguments = new object[1];
                arguments[0] = 10000;
                objFM.GetType().InvokeMember("Start", BindingFlags.InvokeMethod, null, objFM, arguments);

                arguments = new object[2];
                arguments[0] = "d:\\tmp";
                arguments[1] = 0x00000001 | 0x00000010;

                string taskId = (objFM.GetType().InvokeMember("CreateTask", BindingFlags.InvokeMethod,
                    null, objFM, arguments)) as string;

                Console.WriteLine("task {0}", taskId);

                arguments = new object[2];
                arguments[0] = taskId;
                arguments[1] = 0;

                ParameterModifier parametersOut = new ParameterModifier(2);
                parametersOut[0] = false;
                parametersOut[1] = true;    // the second argument is passed on by reference !!!!

                objFM.GetType().InvokeMember("StartTask", BindingFlags.InvokeMethod,
                    null, objFM, arguments,
                    new ParameterModifier[] { parametersOut }, null, null);

                Console.WriteLine("Working ...");
                Console.ReadKey();

                objFM.GetType().InvokeMember("StopTask", BindingFlags.InvokeMethod,
                   null, objFM, arguments,
                   new ParameterModifier[] { parametersOut }, null, null);

                objFM.GetType().InvokeMember("Stop", BindingFlags.InvokeMethod,
                    null, objFM, null);

            }
            catch (Exception e)
            {
                String errorMessage;
                errorMessage = "Error: ";
                errorMessage = String.Concat(errorMessage, e.Message);
                errorMessage = String.Concat(errorMessage, " Line: ");
                errorMessage = String.Concat(errorMessage, e.Source);

                Console.WriteLine(errorMessage);
                Console.WriteLine(e.InnerException.ToString());
            }

            Console.WriteLine("Bye ...");
        }
    }
}
