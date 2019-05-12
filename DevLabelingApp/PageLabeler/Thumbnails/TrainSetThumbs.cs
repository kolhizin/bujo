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
        public enum ObsState { ToDo, Done, Exclude };
        private class ObsInfo
        {
            public uint idx;
            public ObsState state;
            public bool selected;
        };

        private ThumbPics thumbs_;
        private Dictionary<string, ObsInfo> observations_;

        
        private ContextMenu generateContextMenu(string fname, ObsState state)
        {
            ContextMenu res = new ContextMenu();
            if(state == ObsState.ToDo)
            {
                res.MenuItems.Add("Exclude", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.Exclude);
                }));
                res.MenuItems.Add("Done", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.Done);
                }));
            }
            else if (state == ObsState.Exclude)
            {
                res.MenuItems.Add("Back in to-do", new EventHandler((object obj, EventArgs arg) =>
                {
                    this.UpdateState(fname, ObsState.ToDo);
                }));
            }
            else if (state == ObsState.Done)
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
            thumbs_ = new ThumbPics(size, (uint id, string f) => { });
            observations_ = new Dictionary<string, ObsInfo>();
        }

        private void setStateToDo(string fname)
        {
            uint idx = observations_[fname].idx;
            thumbs_.ReloadThumb(idx);
            observations_[fname].state = ObsState.ToDo;
            thumbs_.GetPictureBox(idx).ContextMenu = generateContextMenu(fname, ObsState.ToDo);
        }
        private void setStateExclude(string fname)
        {
            uint idx = observations_[fname].idx;
            thumbs_.ReloadThumb(idx);
            Image img = thumbs_.GetPictureBox(idx).Image;
            using (Graphics g = Graphics.FromImage(img))
            {
                SolidBrush shadowBrush = new SolidBrush(Color.FromArgb(176, Color.Black));
                g.FillRectangle(shadowBrush, 0, 0, img.Width, img.Height);

                g.DrawString("EXCLUDED", new Font(FontFamily.GenericMonospace, 12.0f, FontStyle.Bold),
                    new SolidBrush(Color.PaleVioletRed), 0.0f, 0.0f);
            }
            observations_[fname].state = ObsState.Exclude;
            thumbs_.GetPictureBox(idx).ContextMenu = generateContextMenu(fname, ObsState.Exclude);
        }

        private void setStateDone(string fname)
        {
            uint idx = observations_[fname].idx;
            thumbs_.ReloadThumb(idx);
            Image img = thumbs_.GetPictureBox(idx).Image;
            using (Graphics g = Graphics.FromImage(img))
            {
                SolidBrush shadowBrush = new SolidBrush(Color.FromArgb(128, Color.Green));
                g.FillRectangle(shadowBrush, 0, 0, img.Width, img.Height);
                g.DrawString("DONE", new Font(FontFamily.GenericMonospace, 12.0f, FontStyle.Bold),
                    new SolidBrush(Color.White), 0.0f, 0.0f);
            }
            observations_[fname].state = ObsState.Done;
            thumbs_.GetPictureBox(idx).ContextMenu = generateContextMenu(fname, ObsState.Done);
        }

        public void UpdateState(string fname, ObsState state)
        {
            if (state == ObsState.ToDo)
                setStateToDo(fname);
            else if (state == ObsState.Done)
                setStateDone(fname);
            else setStateExclude(fname);
        }

        public void AddObservation(string fname, ObsState state = ObsState.ToDo)
        {
            if (observations_.ContainsKey(fname))
                throw new Exception("Observation with this filename already in data-set!");
            uint id = thumbs_.AddThumb(fname);
            ObsInfo res = new ObsInfo();
            res.idx = id;
            res.selected = false;
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
    };
}
