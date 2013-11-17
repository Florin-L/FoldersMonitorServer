using System;

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

            Console.WriteLine("Bye ...");
        }
    }
}
