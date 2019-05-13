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
        private Dictionary<uint, Tuple<string, PictureBox>> thumbs_;
        private Size size_;
        private Action<uint, string> onClick_;
        uint idCounter_;

        public ThumbPics(Size size, Action<uint, string> onClick)
        {
            onClick_ = onClick;
            size_ = size;
            thumbs_ = new Dictionary<uint, Tuple<string, PictureBox>>();
            idCounter_ = 0;
        }
        public void UpdateSize(Size size)
        {
            size_ = size;
            foreach (var item in thumbs_)
                item.Value.Item2.Size = size;
        }
        public void Clear()
        {
            thumbs_.Clear();
            idCounter_ = 0;
        }
        private Image LoadThumb(string fname)
        {
            Image img = Image.FromFile(fname);

            foreach (var prop in img.PropertyItems)
            {
                if (prop.Id == 0x112)
                {
                    if (prop.Value[0] == 0x08)
                        img.RotateFlip(RotateFlipType.Rotate270FlipNone);
                    else if (prop.Value[0] == 0x03)
                        img.RotateFlip(RotateFlipType.Rotate180FlipNone);
                    else if (prop.Value[0] == 0x06)
                        img.RotateFlip(RotateFlipType.Rotate90FlipNone);
                    else if (prop.Value[0] != 0x01)
                        MessageBox.Show("Unkonwn image orientation!", "Warning!");
                }
            }

            float maxSize = Math.Max(size_.Width, size_.Height);
            float maxSizeF = Math.Max(img.Width, img.Height);
            float dSize = Math.Max(1.0f, maxSizeF / maxSize);
            Bitmap bmp = new Bitmap(img, new Size((int)(img.Width / dSize), (int)(img.Height / dSize)));
            img.Dispose();
            return bmp;
        }
        public PictureBox GetPictureBox(uint id)
        {
            return thumbs_[id].Item2;
        }
        public void ReloadThumb(uint id)
        {
            thumbs_[id].Item2.Image = LoadThumb(thumbs_[id].Item1);
        }
        public uint AddThumb(string fname)
        {
            PictureBox res = new PictureBox();
            res.Size = size_;
            res.SizeMode = PictureBoxSizeMode.Zoom;
            uint idx = idCounter_;

            res.Image = LoadThumb(fname);
            res.Click += (object sender, EventArgs e) =>
            {
                MouseEventArgs me = (MouseEventArgs)e;
                if(me.Button == MouseButtons.Left)
                    onClick_(idx, fname);
            };

            thumbs_[idx] = new Tuple<string, PictureBox>(fname, res);
            idCounter_++;
            return idx;
        }
        
    }
}
