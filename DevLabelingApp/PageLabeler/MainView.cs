using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.Windows.Forms;
using BuJoDetector;
using System.IO;

namespace PageLabeler
{
    class MainView
    {
        private PictureBox pbox_;
        private ManagedDetector detector_;
        private ContextMenu cm_;
        private Image srcImage_;
        private string fname_;

        private Bitmap rotateImage_(Bitmap bmp, float degrees)
        {
            float rad_angle = (float)(degrees * Math.PI / 180.0f);
            float abs_cos = (float)(Math.Abs(Math.Cos(rad_angle)));
            float abs_sin = (float)(Math.Abs(Math.Sin(rad_angle)));
            float newW = abs_cos * bmp.Width + abs_sin * bmp.Height;
            float newH = abs_sin * bmp.Width + abs_cos * bmp.Height;
            Bitmap rotatedImage = new Bitmap((int)newW, (int)newH);
            using (Graphics g = Graphics.FromImage(rotatedImage))
            {
                g.TranslateTransform(newW / 2, newH / 2);
                g.RotateTransform(degrees);
                g.TranslateTransform(-bmp.Width / 2, -bmp.Height / 2);
                g.DrawImage(bmp, new Point(0, 0));
            }

            return rotatedImage;
        }

        public MainView(PictureBox pbox, ManagedDetector detector)
        {
            fname_ = "";
            pbox_ = pbox;
            detector_ = detector;
            cm_ = new ContextMenu();
            cm_.MenuItems.Add("Show original image", (object obj, EventArgs args)=>{
                ShowOriginal();
            });
            cm_.MenuItems.Add("Show aligned image", (object obj, EventArgs args) =>{
                ShowAligned();
            });
            cm_.MenuItems.Add("Show detector image");
            cm_.MenuItems.Add("Show filtered image");
            cm_.MenuItems.Add("Show text image");
            cm_.MenuItems.Add("-");
            cm_.MenuItems.Add("Show support lines").Checked = false;
            cm_.MenuItems.Add("Show all lines").Checked = false;
            cm_.MenuItems.Add("Show words").Checked = false;
            pbox_.ContextMenu = cm_;
            pbox_.SizeMode = PictureBoxSizeMode.Zoom;
        }

        public void SelectObservation(string imgFileName)
        {
            fname_ = imgFileName;
            srcImage_ = null;
            if (File.Exists(fname_))
            {
                srcImage_ = Image.FromFile(fname_);
                pbox_.Image = srcImage_;
            }
        }
        public string GetSelected() { return fname_; }
        public bool ObservationExists() { return File.Exists(fname_); }
        public Image GetOriginalImage() { return srcImage_; }
        public void ShowOriginal()
        {
            if (!File.Exists(fname_))
                return;
            pbox_.Image = srcImage_;
        }
        public void ShowAligned()
        {
            if (!File.Exists(fname_))
                return;
            pbox_.Image = rotateImage_((Bitmap)srcImage_, -detector_.GetAngle() * 180.0f / (float)Math.PI);
        }
    }
}
