using Microsoft.Win32;
using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Configuration.Install;
using System.Diagnostics;
using System.IO;
using System.Reflection;

namespace keepalived_setup_action
{

    [RunInstaller(true)]
    public partial class Installer1 : Installer
    {
        string serviceName = "keepalived-service";
        string exe = "keepalived-windows.exe";
        static string logPath = @"C:\log\log.txt";
        public Installer1()
        {
            InitializeComponent();
        }

        protected override void OnAfterInstall(IDictionary savedState)
        {
            string path = this.Context.Parameters["targetdir"];
            string binPath = path + exe;
            Logger("安装...");
            Logger(path + exe);
            Process scProcess = new Process();
            scProcess.StartInfo.FileName = "sc.exe";
            scProcess.StartInfo.Arguments = "create " + serviceName + " binPath= \"" + binPath + "\" start= auto DisplayName= \"" + serviceName + "\"";
            scProcess.Start();
            scProcess.WaitForExit();

            if (scProcess.ExitCode != 0)
            {
                Logger("服务注册成功");
                // Failed to create service
                // Handle error
            }
            else
            {
                Logger("服务注册失败");
                // Service created successfully
            }
            base.OnAfterInstall(savedState);
        }

        protected override void OnBeforeUninstall(IDictionary savedState)
        {
            Process scProcess = new Process();
            scProcess.StartInfo.FileName = "sc.exe";
            scProcess.StartInfo.Arguments = "delete \"" + serviceName + "\"";
            scProcess.Start();
            scProcess.WaitForExit();

            if (scProcess.ExitCode != 0)
            {
                Logger("服务删除成功");
                // Failed to create service
                // Handle error
            }
            else
            {
                Logger("服务删除失败");
                // Service created successfully
            }
            base.OnBeforeUninstall(savedState);
        }

        protected override void OnAfterUninstall(IDictionary savedState)
        {
            Logger("OnAfterUninstall...");

            var savedStateValue = savedState.Contains("savedState") ? savedState["savedState"] : "未获取到安装的目录";
            Logger($"OnAfterUninstall从OnAfterInstall获取 savedState，值为:{savedStateValue}");
            string path = this.Context.Parameters["targetdir"];
            Logger($"targetdir:{path}");
            Logger($"开始删除目录：{path}");
            if (Directory.Exists(path))
            {
                RemoveSubDirectory(new DirectoryInfo(path));
                Logger($"删除目录：{path} 成功");
            }
            Logger("OnAfterUninstall  完成了...");
            base.OnAfterUninstall(savedState);
        }

        protected override void OnCommitted(IDictionary savedState)
        {
            base.OnCommitted(savedState);
        }

        /// 卸载完成后删除多余的文件
        private static void RemoveSubDirectory(DirectoryInfo directory)
        {
            Logger($"目录信息 directory:{directory}");
            foreach (DirectoryInfo sub in directory.GetDirectories())
            {
                if (sub.GetFiles().Length > 0 || sub.GetDirectories().Length > 0)
                    RemoveSubDirectory(sub);
                sub.Delete(true);
                Logger($"要删除的目录信息 sub:{sub}");
            }
            Logger($"目录成功");
        }



        /// <summary>
        /// 记录日志
        /// </summary>
        /// <param name="message"></param>
        private static void Logger(string message)
        {
            string directory = Path.GetDirectoryName(logPath);
            if (!Directory.Exists(directory))
            {
                Directory.CreateDirectory(directory);
            }
            if (!File.Exists(logPath))
            {
                File.Create(logPath).Close();
            }
            Trace.Listeners.Clear();
            Trace.AutoFlush = true;
            Trace.Listeners.Add(new TextWriterTraceListener(logPath));
            Trace.WriteLine($"{DateTime.Now:yyyy-MM-dd HH:mm:ss}" + message);
        }
    }
}
