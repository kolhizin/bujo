using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;

namespace PageLabeler
{
    class TrainSetThumbs
    {
        public enum ObsState { ToDo, Success, Fail, Exclude };
        public enum EventType { SetSuccess, SetFail, SetToDo, SetExclude, Select };
        private class ObsInfo
        {
            public uint idx;
            public ObsState state;
        };

        private ThumbPics thumbs_;
        private Dictionary<string, ObsInfo> observations_;
        private string selectedObservation_;
        private List<Action<string, EventType>> callbacks_;

        
        private ContextMenu generateContextMenu(string fname, ObsState state)
        {
            ContextMenu res = new ContextMenu();
            if(state == ObsState.ToDo)
            {
                res.MenuItems.Add("Exclude", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.Exclude);
                }));
                res.MenuItems.Add("Success", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.Success);
                }));
                res.MenuItems.Add("Fail", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.Fail);
                }));
            }
            else if (state == ObsState.Exclude)
            {
                res.MenuItems.Add("Back in to-do", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.ToDo);
                }));
            }
            else if (state == ObsState.Success || state == ObsState.Fail)
            {
                res.MenuItems.Add("Back in to-do", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.ToDo);
                }));
                res.MenuItems.Add("Exclude", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.Exclude);
                }));
            }
            return res;
        }

        public TrainSetThumbs(Size size)
        {
            thumbs_ = new ThumbPics(size, (uint id, string f) => { selectObs(f); });
            observations_ = new Dictionary<string, ObsInfo>();
            selectedObservation_ = "";
            callbacks_ = new List<Action<string, EventType>>();
        }

        private void notifyCallbacks(string fname, EventType e)
        {
            foreach (var v in callbacks_)
                v(fname, e);
        }

        private void selectObs(string fname)
        {
            if(observations_[fname].state == ObsState.Exclude)
            {
                MessageBox.Show("Cannot select excluded observation!");
                return;
            }
            string tmp = selectedObservation_;
            selectedObservation_ = fname;
            if (observations_.ContainsKey(tmp))
                updateThumb(tmp);
            updateThumb(selectedObservation_);
            notifyCallbacks(fname, EventType.Select);
        }
        private void updateThumb(string fname)
        {
            uint idx = observations_[fname].idx;
            thumbs_.ReloadThumb(idx);
            thumbs_.GetPictureBox(idx).ContextMenu = generateContextMenu(fname, observations_[fname].state);
            if(observations_[fname].state == ObsState.Exclude)
            {
                Image img = thumbs_.GetPictureBox(idx).Image;
                using (Graphics g = Graphics.FromImage(img))
                {
                    SolidBrush shadowBrush = new SolidBrush(Color.FromArgb(176, Color.Black));
                    g.FillRectangle(shadowBrush, 0, 0, img.Width, img.Height);

                    g.DrawString("EXCLUDED", new Font(FontFamily.GenericMonospace, 12.0f, FontStyle.Bold),
                        new SolidBrush(Color.PaleVioletRed), 0.0f, 0.0f);
                }
            }else if(observations_[fname].state == ObsState.Success)
            {
                Image img = thumbs_.GetPictureBox(idx).Image;
                using (Graphics g = Graphics.FromImage(img))
                {
                    SolidBrush shadowBrush = new SolidBrush(Color.FromArgb(128, Color.Green));
                    g.FillRectangle(shadowBrush, 0, 0, img.Width, img.Height);
                    g.DrawString("SUCCESS", new Font(FontFamily.GenericMonospace, 12.0f, FontStyle.Bold),
                        new SolidBrush(Color.White), 0.0f, 0.0f);
                }
            }
            else if (observations_[fname].state == ObsState.Fail)
            {
                Image img = thumbs_.GetPictureBox(idx).Image;
                using (Graphics g = Graphics.FromImage(img))
                {
                    SolidBrush shadowBrush = new SolidBrush(Color.FromArgb(128, Color.DarkRed));
                    g.FillRectangle(shadowBrush, 0, 0, img.Width, img.Height);
                    g.DrawString("FAIL", new Font(FontFamily.GenericMonospace, 12.0f, FontStyle.Bold),
                        new SolidBrush(Color.White), 0.0f, 0.0f);
                }
            }
            if (selectedObservation_ == fname)
            {
                Image img = thumbs_.GetPictureBox(idx).Image;
                using (Graphics g = Graphics.FromImage(img))
                {
                    g.DrawRectangle(new Pen(Brushes.Red, 5),
                        new Rectangle(0, 0, img.Width, img.Height));
                }
            }
        }
        private void setStateToDo(string fname)
        {
            observations_[fname].state = ObsState.ToDo;
            updateThumb(fname);
            notifyCallbacks(fname, EventType.SetToDo);
        }
        private void setStateExclude(string fname)
        {
            observations_[fname].state = ObsState.Exclude;
            updateThumb(fname);
            notifyCallbacks(fname, EventType.SetExclude);
        }

        private void setStateSuccess(string fname)
        {
            observations_[fname].state = ObsState.Success;
            updateThumb(fname);
            notifyCallbacks(fname, EventType.SetSuccess);
        }
        private void setStateFail(string fname)
        {
            observations_[fname].state = ObsState.Fail;
            updateThumb(fname);
            notifyCallbacks(fname, EventType.SetFail);
        }

        public void UpdateState(string fname, ObsState state)
        {
            if (state == ObsState.ToDo)
                setStateToDo(fname);
            else if (state == ObsState.Success)
                setStateSuccess(fname);
            else if (state == ObsState.Fail)
                setStateFail(fname);
            else setStateExclude(fname);
        }
        public void Clear()
        {
            observations_.Clear();
        }
        public void AddObservation(string fname, ObsState state = ObsState.ToDo)
        {
            if (observations_.ContainsKey(fname))
                throw new Exception("Observation with this filename already in data-set!");
            uint id = thumbs_.AddThumb(fname);
            ObsInfo res = new ObsInfo();
            res.idx = id;
            res.state = ObsState.ToDo;
            thumbs_.GetPictureBox(id).ContextMenu = generateContextMenu(fname, ObsState.ToDo);
            observations_[fname] = res;
            if (state != ObsState.ToDo)
                UpdateState(fname, state);
        }

        public PictureBox GetPictureBox(string fname)
        {
            return thumbs_.GetPictureBox(observations_[fname].idx);
        }

        public void UpdateSize(Size size)
        {
            thumbs_.UpdateSize(size);
        }

        public void AddCallback(Action<string, EventType> callback)
        {
            callbacks_.Add(callback);
        }
    };
}
