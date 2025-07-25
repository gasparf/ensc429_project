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
        private TrackBar gainSlider;
        private Label gainLabel;
        private Process encoderProcess;
        private float currentGain = 0.8f; // Default gain value

        public MainForm()
        {
            Text = "Voice Encoder";
            Width = 400;
            Height = 350;

            startButton = new Button() { Text = "Start", Left = 10, Top = 10, Width = 75 };
            stopButton = new Button() { Text = "Stop", Left = 90, Top = 10, Width = 75 };
            
            // Gain control label
            gainLabel = new Label() 
            { 
                Text = $"Gain: {currentGain:0.00}", 
                Left = 180, 
                Top = 15, 
                Width = 100 
            };
            
            // Gain control slider
            gainSlider = new TrackBar() 
            { 
                Left = 10, 
                Top = 40, 
                Width = 360, 
                Height = 45,
                Minimum = 0,
                Maximum = 200, // 0.0 to 2.0 gain (scaled by 100)
                Value = (int)(currentGain * 100),
                TickFrequency = 25,
                LargeChange = 25,
                SmallChange = 5
            };
            
            logBox = new TextBox() { Left = 10, Top = 90, Width = 360, Height = 200, Multiline = true, ScrollBars = ScrollBars.Vertical };

            startButton.Click += StartClicked;
            stopButton.Click += StopClicked;
            
            gainSlider.ValueChanged += GainSliderChanged;

            Controls.Add(startButton);
            Controls.Add(stopButton);
            // Add gain control elements
            Controls.Add(gainLabel);
            Controls.Add(gainSlider);
            Controls.Add(logBox);
        }

        private void GainSliderChanged(object sender, EventArgs e)
        {
            currentGain = gainSlider.Value / 100.0f; // Convert back from scaled integer
            gainLabel.Text = $"Gain: {currentGain:0.00}";
        }

        private void StartClicked(object sender, EventArgs e)
        {
            if (encoderProcess != null && !encoderProcess.HasExited)
                return;

            encoderProcess = new Process();
            encoderProcess.StartInfo.FileName = "voice_encoder";
            encoderProcess.StartInfo.Arguments = currentGain.ToString("0.00", System.Globalization.CultureInfo.InvariantCulture);
            encoderProcess.StartInfo.RedirectStandardOutput = true;
            encoderProcess.StartInfo.RedirectStandardError = true;
            encoderProcess.StartInfo.UseShellExecute = false;
            encoderProcess.StartInfo.CreateNoWindow = true;
            
            encoderProcess.OutputDataReceived += (s, ev) =>
            {
                if (ev.Data != null)
                    logBox.Invoke(new Action(() => logBox.AppendText(ev.Data + Environment.NewLine)));
            };
            
            encoderProcess.ErrorDataReceived += (s, ev) =>
            {
                if (ev.Data != null)
                    logBox.Invoke(new Action(() => logBox.AppendText("Error: " + ev.Data + Environment.NewLine)));
            };

            try
            {
                logBox.AppendText($"Starting voice encoder with gain: {currentGain:0.00}" + Environment.NewLine);
                encoderProcess.Start();
                encoderProcess.BeginOutputReadLine();
                encoderProcess.BeginErrorReadLine();
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