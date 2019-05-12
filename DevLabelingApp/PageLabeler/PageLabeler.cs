using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PageLabeler
{
    public partial class PageLabeler : Form
    {
        private TrainSetThumbs tdsThumbs_;
        public PageLabeler()
        {
            InitializeComponent();

            tdsThumbs_ = new TrainSetThumbs(new Size(100,100));
            tdsThumbs_.AddCallback((string s, TrainSetThumbs.EventType e) =>
            {
                MessageBox.Show(s + " -> " + e.ToString());
            });
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

    }
}
