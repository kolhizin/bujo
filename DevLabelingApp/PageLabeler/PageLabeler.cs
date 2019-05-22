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
        private HashSet<WordInfo.WordStatus> skipSet_;

        private bool checkDataset_(bool skipOutput = false)
        {
            if (dataset_ == null)
            {
                MessageBox.Show("Need to load dataset first!");
                return false;
            }
            if((!skipOutput) && (!dataset_.HasOutput()))
            {
                MessageBox.Show("Need to set output first!");
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
            skipSet_ = new HashSet<WordInfo.WordStatus>();
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

                    UpdateWordView();
                    UpdateErrorView();
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

            mainView_ = new MainView(pbMain, detector_, navigator_);
            dataset_ = new PageInfo.DatasetInfo();
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
            }catch
            {
                fSuccess = false;
            }

            if (!fSuccess)
            {
                detectorStatus.Text = "Detection failed.";
                return;
            }
            detectorStatus.Text = "Successfull detection in " + (detector_.GetTimeCompute() / 1000.0f).ToString() + "s.";
            mainView_.UpdateAngle();
            dataset_.ResetPage(mainView_.GetSelected(), detector_, mainView_.GetAlignedImage());
            navigator_.SetPage(dataset_.GetPage(mainView_.GetSelected()), mainView_.GetAlignedImage());
        }

        private void BtnOuput_Click(object sender, EventArgs e)
        {
            if (!checkDataset_(true))
                return;
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "JSON files (.json)|*.json";
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                dataset_.UpdateOutput(sfd.FileName);
                navigator_.SetPath(dataset_.GetPath());
            }
        }

        private void BtnLoadDataset_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            sfd.Filter = "JSON files (.json)|*.json";
            if (sfd.ShowDialog() == DialogResult.OK)
            {
                dataset_ = new PageInfo.DatasetInfo(sfd.FileName);
                navigator_.SetPath(dataset_.GetPath());
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
            if(e.Modifiers == Keys.Control)
            {
                if(e.KeyCode == Keys.OemOpenBrackets)
                {
                    if (!navigator_.PrevError())
                        MessageBox.Show("Reached errors end!", "Notification");
                    UpdateErrorView();
                    e.Handled = true;
                }
                if (e.KeyCode == Keys.OemCloseBrackets)
                {
                    
                    if (!navigator_.NextError())
                        MessageBox.Show("Reached errors end!", "Notification");
                    UpdateErrorView();
                    e.Handled = true;
                }
                if (e.KeyCode == Keys.Right || e.KeyCode == Keys.Left
                    || e.KeyCode == Keys.Up || e.KeyCode == Keys.Down)
                {
                    if (e.KeyCode == Keys.Right)
                       if (!navigator_.NextWord(skipSet_))
                            MessageBox.Show("Reached end!", "Notification");
                    if (e.KeyCode == Keys.Left)
                        if (!navigator_.PrevWord(skipSet_))
                            MessageBox.Show("Reached start!", "Notification");
                    if (e.KeyCode == Keys.Down)
                        if (!navigator_.NextLine(skipSet_))
                            MessageBox.Show("Reached end!", "Notification");
                    if (e.KeyCode == Keys.Up)
                        if (!navigator_.PrevLine(skipSet_))
                            MessageBox.Show("Reached start!", "Notification");

                    UpdateWordView();
                    e.Handled = true;
                }
                if (e.KeyCode == Keys.I && (e.Modifiers == Keys.Control))
                {
                    var status = navigator_.GetWordStatus();
                    if (status == WordInfo.WordStatus.INCORRECT)
                        navigator_.SetWordStatus(WordInfo.WordStatus.CORRECT);
                    else
                        navigator_.SetWordStatus(WordInfo.WordStatus.INCORRECT);
                    cbWordStatus.Text = navigator_.GetWordStatus().ToString();
                    e.Handled = true;
                }
                if (e.KeyCode == Keys.O && (e.Modifiers == Keys.Control))
                {
                    var status = PageInfo.PageError.CycleError(navigator_.GetError().type);
                    navigator_.GetError().type = status;
                    cbErrorType.Text = status.ToString();
                    e.Handled = true;
                }
                if (e.KeyCode == Keys.S && (e.Modifiers == Keys.Control))
                {
                    BtnSave_Click(sender, e);
                    e.Handled = true;
                }
            }
        }

        private void UpdateWordView()
        {
            pbWord.Image = navigator_.GetWordImage(dataset_.GetPath());

            cbWordStatus.Text = navigator_.GetWordStatus().ToString();
            txtWordComment.Text = navigator_.GetWordComment();

            cbLineStatus.Text = navigator_.GetLineStatus().ToString();
            txtLineComment.Text = navigator_.GetLineComment();

            txtWord.Text = navigator_.GetWordText();

            mainView_.UpdateOverlay();
        }
        private void UpdateErrorView()
        {
            var err = navigator_.GetError();
            if(err == null)
            {
                cbErrorType.Text = "";
                txtErrorDescription.Text = "";
            }
            else
            {
                cbErrorType.Text = err.type.ToString();
                txtErrorDescription.Text = err.description;
            }
            mainView_.UpdateOverlay();
        }


        private void TxtWordComment_TextChanged(object sender, EventArgs e)
        {
            navigator_.SetWordComment(txtWordComment.Text);
        }

        private void CbWordStatus_SelectedIndexChanged(object sender, EventArgs e)
        {
            if(cbWordStatus.Text == "CORRECT")
                navigator_.SetWordStatus(WordInfo.WordStatus.CORRECT);
            if (cbWordStatus.Text == "INCORRECT")
                navigator_.SetWordStatus(WordInfo.WordStatus.INCORRECT);
            if (cbWordStatus.Text == "UNKNOWN")
                navigator_.SetWordStatus(WordInfo.WordStatus.UNKNOWN);
        }

        private void TxtWord_TextChanged(object sender, EventArgs e)
        {
            navigator_.SetWordText(txtWord.Text);
            if(txtWord.Text != "")
            {
                navigator_.SetWordStatus(WordInfo.WordStatus.CORRECT);
                cbWordStatus.Text = navigator_.GetWordStatus().ToString();
            }
        }

        private void CbLineStatus_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (cbLineStatus.Text.ToUpper() == "CORRECT")
                navigator_.SetLineStatus(PageInfo.LineInfo.LineStatus.CORRECT);
            if (cbLineStatus.Text.ToUpper() == "INCORRECT")
                navigator_.SetLineStatus(PageInfo.LineInfo.LineStatus.INCORRECT);
            if (cbLineStatus.Text.ToUpper() == "UNKNOWN")
                navigator_.SetLineStatus(PageInfo.LineInfo.LineStatus.UNKNOWN);
        }

        private void TxtLineComment_TextChanged(object sender, EventArgs e)
        {
            navigator_.SetLineComment(txtLineComment.Text);
        }

        private void CbSkipCorrect_CheckedChanged(object sender, EventArgs e)
        {
            if (cbSkipCorrect.Checked)
                skipSet_.Add(WordInfo.WordStatus.CORRECT);
            else
                skipSet_.Remove(WordInfo.WordStatus.CORRECT);
        }

        private void CbSkipIncorrect_CheckedChanged(object sender, EventArgs e)
        {
            if (cbSkipIncorrect.Checked)
                skipSet_.Add(WordInfo.WordStatus.INCORRECT);
            else
                skipSet_.Remove(WordInfo.WordStatus.INCORRECT);
        }

        private void PbMain_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            float x = 0.0f, y = 0.0f;
            if((float)pbMain.Width / (float)pbMain.Height < (float)pbMain.Image.Width / (float)pbMain.Image.Height)
            {
                float w = pbMain.Width;
                float h = pbMain.Width * pbMain.Image.Height / pbMain.Image.Width;
                x = (e.Location.X - (pbMain.Width - w) / 2) / w;
                y = (e.Location.Y - (pbMain.Height - h) / 2) / h;
            }else
            {
                float w = pbMain.Height * pbMain.Image.Width / pbMain.Image.Height;
                float h = pbMain.Height;
                x = (e.Location.X - (pbMain.Width - w) / 2) / w;
                y = (e.Location.Y - (pbMain.Height - h) / 2) / h;
            }
            if ((x < 0.0f) || (y < 0.0f) || (x > 1.0f) || (y > 1.0f))
            {
                MessageBox.Show("Location (" + x.ToString() + ", " + y.ToString() + ") is out of image!", "Error!");
                return;
            }
            navigator_.AddError(x, y, PageInfo.PageError.ErrorType.OTHER);
            navigator_.SelectLastError();
            UpdateErrorView();
        }

        private void CbErrorType_SelectedIndexChanged(object sender, EventArgs e)
        {
            var err = navigator_.GetError();
            if (cbErrorType.Text.ToUpper() == "MISSING_WORD")
                err.type = PageInfo.PageError.ErrorType.MISSING_WORD;
            if (cbErrorType.Text.ToUpper() == "MISSING_LINE")
                err.type = PageInfo.PageError.ErrorType.MISSING_LINE;
            if (cbErrorType.Text.ToUpper() == "MISSING_REGION")
                err.type = PageInfo.PageError.ErrorType.MISSING_REGION;
            if (cbErrorType.Text.ToUpper() == "OTHER")
                err.type = PageInfo.PageError.ErrorType.OTHER;
        }

        private void TxtErrorDescription_TextChanged(object sender, EventArgs e)
        {
            var err = navigator_.GetError();
            if (err == null)
                return;
            err.description = txtErrorDescription.Text;
        }

        private void BtnRemoveError_Click(object sender, EventArgs e)
        {
            navigator_.RemoveError();
            UpdateErrorView();
        }
    }
}
