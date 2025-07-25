using System;
using System.Windows.Forms;
using System.Diagnostics;

namespace VoiceEncoderGui
{
    static class Program
    {
        [STAThread]
        static void Main()
        {
            Application.SetHighDpiMode(HighDpiMode.SystemAware);
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new MainForm());
        }
    }

    public class MainForm : Form
    {
        private Button startButton;
        private Button stopButton;
        private TextBox logBox;
        private Process encoderProcess;

        public MainForm()
        {
            Text = "Voice Encoder";
            Width = 400;
            Height = 300;

            startButton = new Button() { Text = "Start", Left = 10, Top = 10 };
            stopButton = new Button() { Text = "Stop", Left = 90, Top = 10 };
            logBox = new TextBox() { Left = 10, Top = 50, Width = 360, Height = 200, Multiline = true, ScrollBars = ScrollBars.Vertical };

            startButton.Click += StartClicked;
            stopButton.Click += StopClicked;

            Controls.Add(startButton);
            Controls.Add(stopButton);
            Controls.Add(logBox);
        }

        private void StartClicked(object sender, EventArgs e)
        {
            if (encoderProcess != null && !encoderProcess.HasExited)
                return;

            encoderProcess = new Process();
            encoderProcess.StartInfo.FileName = "voice_encoder";
            encoderProcess.StartInfo.RedirectStandardOutput = true;
            encoderProcess.StartInfo.UseShellExecute = false;
            encoderProcess.StartInfo.CreateNoWindow = true;
            encoderProcess.OutputDataReceived += (s, ev) =>
            {
                if (ev.Data != null)
                    logBox.Invoke(new Action(() => logBox.AppendText(ev.Data + Environment.NewLine)));
            };

            try
            {
                encoderProcess.Start();
                encoderProcess.BeginOutputReadLine();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Failed to start voice_encoder: {ex.Message}");
            }
        }

        private void StopClicked(object sender, EventArgs e)
        {
            try
            {
                if (encoderProcess != null && !encoderProcess.HasExited)
                {
                    encoderProcess.Kill();
                    encoderProcess.WaitForExit();
                    logBox.AppendText("Encoder stopped" + Environment.NewLine);
                }
            }
            catch (Exception ex)
            {
                logBox.AppendText("Error stopping encoder: " + ex.Message + Environment.NewLine);
            }
        }
    }
}