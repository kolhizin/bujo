using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Drawing;

namespace PageLabeler
{
    class ThumbPics
    {
        private List<PictureBox> thumbs_;
        private Dictionary<uint, PictureBox> idThumb_;
        private Size size_;
        private Action<uint, string> onClick_;
        uint idCounter_;
        ContextMenu cm;

        public ThumbPics()
        {
            cm = new ContextMenu();
            cm.MenuItems.Add("Test");
            thumbs_ = new List<PictureBox>();
            idThumb_ = new Dictionary<uint, PictureBox>();
            idCounter_ = 0;
        }
        public void SetSize(Size size)
        {
            size_ = size;
        }
        public void SetOnClick(Action<uint, string> f)
        {
            onClick_ = f;
        }
        public void Clear()
        {
            thumbs_.Clear();
            idThumb_.Clear();
            idCounter_ = 0;
        }
        private Image LoadThumb(string fname)
        {
            Image img = Image.FromFile(fname);
            float maxSize = Math.Max(size_.Width, size_.Height);
            float maxSizeF = Math.Max(img.Width, img.Height);
            float dSize = Math.Max(1.0f, maxSizeF / maxSize);
            Bitmap bmp = new Bitmap(img, new Size((int)(img.Width / dSize), (int)(img.Height / dSize)));
            img.Dispose();
            return bmp;
        }
        public PictureBox GetPictureBox(uint id)
        {
            return idThumb_[id];
        }
        public uint AddThumb(string fname)
        {
            PictureBox res = new PictureBox();
            res.Size = size_;
            res.SizeMode = PictureBoxSizeMode.Zoom;
            uint idx = idCounter_;

            res.Image = LoadThumb(fname);
            res.ContextMenu = cm;

            res.Click += (object sender, EventArgs e) =>
            {
                onClick_(idx, fname);
            };

            idCounter_++;
            thumbs_.Add(res);
            idThumb_.Add(idx, res);
            return idx;
        }
        
    }
}
