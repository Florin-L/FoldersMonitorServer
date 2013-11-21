using System;
using System.Reflection;
using System.Runtime.InteropServices.ComTypes;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;

namespace Cs_FoldersMonitorClient
{

    class FoldersMonitorEventHandler : FoldersMonitorServer.IFoldersMonitorEvents
    {
        public void OnChanged(int action, string fileName)
        {
            Console.WriteLine("action {0} detected on file {1}", action, fileName);
        }
    }

    //
    [Guid("231A27E0-55B9-44A2-9025-F82B8ED5F40F"),
    InterfaceType(ComInterfaceType.InterfaceIsIUnknown),
    TypeLibType(TypeLibTypeFlags.FOleAutomation)]
    [ComImport]
    public interface IFoldersMonitorEvents
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        void OnChanged([In] int action, [MarshalAs(UnmanagedType.BStr)][In] string fileName);
    }

    //
    //[ComVisible(false), TypeLibType(TypeLibTypeFlags.FHidden)]
    //public delegate void IFoldersMonitorEvents_OnChangedEventHandler([In] int action, 
    //    [MarshalAs(UnmanagedType.BStr)] [In] string fileName);

    //[ClassInterface(ClassInterfaceType.None), TypeLibType(TypeLibTypeFlags.FHidden)]
    //public sealed class IFoldersMonitorEvents_SinkHelper : IFoldersMonitorEvents
    //{
    //    public IFoldersMonitorEvents_OnChangedEventHandler m_OnChangedDelegate;
    //    public int m_dwCookie;

    //    public override void OnChanged(int action, string fileName)
    //    {
    //        if (this.m_OnChangedDelegate != null)
    //        {
    //            this.m_OnChangedDelegate(action, fileName);
    //            return;
    //        }
    //    }

    //    internal IFoldersMonitorEvents_SinkHelper()
    //    {
    //        this.m_dwCookie = 0;
    //        this.m_OnChangedDelegate = null;
    //    }
    //}

    [ClassInterface(ClassInterfaceType.None), TypeLibType(TypeLibTypeFlags.FHidden)]
    public class FoldersMonitorEvents_Sink : IFoldersMonitorEvents
    {
        public void OnChanged(int action, string fileName)
        {
            Console.WriteLine(fileName);
        }
    }

    //
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("Usage:\n{0} {1} {2}", "Cs_FoldersMonitorClient.exe", "dir_1", "dir_2");
                return;
            }

            string directoryName1 = args[0];
            string directoryName2 = args[1];

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
                string taskId = fm.CreateTask(directoryName1, 0x00000001 | 0x00000010);
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
                arguments[0] = directoryName2;
                arguments[1] = 0x00000001 | 0x00000010;

                //
                int cookie = -1;
                Guid IID_IFoldersMonitorEvents = new Guid("231A27E0-55B9-44A2-9025-F82B8ED5F40F");

                IConnectionPointContainer cpc = objFM as IConnectionPointContainer;

                IConnectionPoint cp;
                cpc.FindConnectionPoint(IID_IFoldersMonitorEvents, out cp);

                IFoldersMonitorEvents sink = new FoldersMonitorEvents_Sink();

                cp.Advise(sink, out cookie);

                //
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

                //
                cp.Unadvise(cookie);

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
