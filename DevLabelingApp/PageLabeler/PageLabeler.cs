using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using BuJoDetector;

namespace PageLabeler
{
    public partial class PageLabeler : Form
    {
        private TrainSetThumbs tdsThumbs_;
        private ManagedDetector detector_;
        private MainView mainView_;
        public PageLabeler()
        {
            InitializeComponent();

            detector_ = new ManagedDetector();

            tdsThumbs_ = new TrainSetThumbs(new Size(100,100));
            tdsThumbs_.AddCallback((string s, TrainSetThumbs.EventType e) =>
            {
                if (e == TrainSetThumbs.EventType.Select)
                    mainView_.SelectObservation(s);
            });

            mainView_ = new MainView(pbMain, detector_);
            SetInputFiles(System.IO.Directory.GetFiles("D:/Data/bujo_sample/", "*.jpg"));
        }

        private void SetInputFiles(string [] files)
        {
            for(int i = 0; i < files.Length; i++)
            {
                tdsThumbs_.AddObservation(files[i]);
                thumbFlowPanel.Controls.Add(tdsThumbs_.GetPictureBox(files[i]));
            }
        }

        private void PageLabeler_Load(object sender, EventArgs e)
        {
        }

        private void BtnInput_Click(object sender, EventArgs e)
        {
            using (var fbd = new FolderBrowserDialog())
            {
                DialogResult result = fbd.ShowDialog();

                if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
                {
                    string[] files = System.IO.Directory.GetFiles(fbd.SelectedPath, "*.jpg");
                    if(files.Length > 100)
                        MessageBox.Show("Found " + files.Length.ToString() + " files, which is too many!", "Message");
                    else
                    {
                        SetInputFiles(files);
                    }
                }
            }
        }

        private void BtnDetect_Click(object sender, EventArgs e)
        {
            if(!mainView_.ObservationExists())
            {
                MessageBox.Show("Observation is not selected or it does not exist!");
                return;
            }
            Image img = mainView_.GetOriginalImage();
            detector_.LoadImage(new Bitmap(img, img.Width/5, img.Height/5), 0.5f);
            MessageBox.Show("Detected in " + (detector_.GetTimeCompute()/1000.0f).ToString() + "s.", "Task complete");
        }
    }
}
