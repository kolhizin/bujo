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
        private PageInfo.DatasetInfo dataset_;
        private PageInfo.PageNavigator navigator_;

        private bool checkDataset_()
        {
            if (dataset_ == null)
            {
                MessageBox.Show("Need to setup output first!");
                return false;
            }
            return true;
        }

        private TrainSetThumbs.ObsState convert_page2thumb(PageInfo.PageInfo.PageStatus status)
        {
            switch (status)
            {
                case PageInfo.PageInfo.PageStatus.FAIL: return TrainSetThumbs.ObsState.Fail;
                case PageInfo.PageInfo.PageStatus.SUCCESS: return TrainSetThumbs.ObsState.Success;
                case PageInfo.PageInfo.PageStatus.EXCLUDE: return TrainSetThumbs.ObsState.Exclude;
                case PageInfo.PageInfo.PageStatus.UNKNOWN: return TrainSetThumbs.ObsState.ToDo;
            }
            return TrainSetThumbs.ObsState.ToDo;
        }
        private PageInfo.PageInfo.PageStatus convert_thumb2page(TrainSetThumbs.ObsState status)
        {
            switch (status)
            {
                case TrainSetThumbs.ObsState.Success: return PageInfo.PageInfo.PageStatus.SUCCESS;
                case TrainSetThumbs.ObsState.Fail: return PageInfo.PageInfo.PageStatus.FAIL;
                case TrainSetThumbs.ObsState.Exclude: return PageInfo.PageInfo.PageStatus.EXCLUDE;
                case TrainSetThumbs.ObsState.ToDo: return PageInfo.PageInfo.PageStatus.UNKNOWN;
            }
            return PageInfo.PageInfo.PageStatus.UNKNOWN;
        }
        public PageLabeler()
        {
            InitializeComponent();
            
            detector_ = new ManagedDetector();
            navigator_ = new PageInfo.PageNavigator();

            tdsThumbs_ = new TrainSetThumbs(new Size(100,100));
            tdsThumbs_.AddCallback((string s, TrainSetThumbs.EventType e) =>
            {
                if(!checkDataset_())
                    return;
                if (e == TrainSetThumbs.EventType.Select)
                {
                    mainView_.SelectObservation(s, dataset_.GetPage(s).angle);
                    navigator_.SetPage(dataset_.GetPage(s), mainView_.GetAlignedImage());
                }
                if (e == TrainSetThumbs.EventType.SetFail)
                    dataset_.GetPage(s).status = PageInfo.PageInfo.PageStatus.FAIL;
                if (e == TrainSetThumbs.EventType.SetSuccess)
                    dataset_.GetPage(s).status = PageInfo.PageInfo.PageStatus.SUCCESS;
                if (e == TrainSetThumbs.EventType.SetExclude)
                    dataset_.GetPage(s).status = PageInfo.PageInfo.PageStatus.EXCLUDE;
                if (e == TrainSetThumbs.EventType.SetToDo)
                    dataset_.GetPage(s).status = PageInfo.PageInfo.PageStatus.UNKNOWN;
            });

            mainView_ = new MainView(pbMain, detector_);
            dataset_ = new PageInfo.DatasetInfo("D:/Data/bujo_sample/dataset.json");
            foreach (var v in dataset_.pages)
            {
                tdsThumbs_.AddObservation(v.Key, convert_page2thumb(v.Value.status));
                thumbFlowPanel.Controls.Add(tdsThumbs_.GetPictureBox(v.Key));
                mainView_.SelectObservation("", 0.0f);
            }
            //AddInputFiles(System.IO.Directory.GetFiles("D:/Data/bujo_sample/", "*.jpg"));
        }

        private void AddInputFiles(string [] files)
        {
            for(int i = 0; i < files.Length; i++)
            {
                tdsThumbs_.AddObservation(files[i]);
                thumbFlowPanel.Controls.Add(tdsThumbs_.GetPictureBox(files[i]));
                if (dataset_ != null)
                    dataset_.AddPage(files[i]);
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
                        AddInputFiles(files);
                    }
                }
            }
        }

        private void BtnDetect_Click(object sender, EventArgs e)
        {
            if (!checkDataset_())
                return;
            if (!mainView_.ObservationExists())
            {
                MessageBox.Show("Observation is not selected or it does not exist!");
                return;
            }
            Image img = mainView_.GetOriginalImage();
            Bitmap bmp = new Bitmap(img, img.Width / 5, img.Height / 5);
            detectorStatus.Text = "Loading in detector...";
            detectorStatus.Update();
            detector_.LoadImage(bmp, 0.5f);
            detectorStatus.Text = "Loaded in detector. Running detection...";
            detectorStatus.Update();
            dataset_.GetPage(mainView_.GetSelected()).angle = detector_.GetAngle();
            bmp.Dispose();
            bool fSuccess = true;
            try
            {
                detector_.RunDetection();
                dataset_.ResetPage(mainView_.GetSelected(), detector_);
            }catch
            {
                fSuccess = false;
            }
            mainView_.UpdateAngle();
            navigator_.SetPage(dataset_.GetPage(mainView_.GetSelected()), mainView_.GetAlignedImage());
            if (!fSuccess)
            {
                detectorStatus.Text = "Detection failed.";
                return;
            }
            detectorStatus.Text = "Successfull detection in " + (detector_.GetTimeCompute() / 1000.0f).ToString() + "s.";
            
        }

        private void BtnOuput_Click(object sender, EventArgs e)
        {
            if (!checkDataset_())
                return;
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "JSON files (.json)|*.json";
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                dataset_.UpdateOutput(sfd.FileName);
            }
        }

        private void BtnLoadDataset_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            sfd.Filter = "JSON files (.json)|*.json";
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                dataset_ = new PageInfo.DatasetInfo(sfd.FileName);
                thumbFlowPanel.Controls.Clear();
                tdsThumbs_.Clear();
                foreach(var v in dataset_.pages)
                {
                    tdsThumbs_.AddObservation(v.Key, convert_page2thumb(v.Value.status));
                    thumbFlowPanel.Controls.Add(tdsThumbs_.GetPictureBox(v.Key));
                    mainView_.SelectObservation("", 0.0f);
                }
            }
        }

        private void BtnSave_Click(object sender, EventArgs e)
        {
            if (!checkDataset_())
                return;
            dataset_.Save();
        }

        private void PageLabeler_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Right)
            {
                navigator_.NextWord();
                pbWord.Image = navigator_.GetWordImage();
                e.Handled = true;
            }
        }
    }
}
