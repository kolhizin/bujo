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
        private MenuItem flgShowSupportLines_, flgShowAllLines_, flgShowWords_;
        private ContextMenu cm_;
        private Image srcImage_;
        private string fname_;
        public delegate void ShowDelegate();
        public ShowDelegate Show;

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
                g.TranslateTransform(newW/2, newH/2);
                g.RotateTransform(degrees);
                g.TranslateTransform(-newW/2, -newH/2);
                g.DrawImage(bmp, (int)(newW - bmp.Width)/2, (int)(newH - bmp.Height)/2, bmp.Width, bmp.Height);
            }

            return rotatedImage;
        }

        public MainView(PictureBox pbox, ManagedDetector detector)
        {
            fname_ = "";
            pbox_ = pbox;
            detector_ = detector;

            flgShowSupportLines_ = new MenuItem("Show support lines", (object obj, EventArgs args) => {
                flgShowSupportLines_.Checked = !flgShowSupportLines_.Checked;
                Show();
            });
            flgShowSupportLines_.Checked = false;

            flgShowAllLines_ = new MenuItem("Show all lines", (object obj, EventArgs args) => {
                flgShowAllLines_.Checked = !flgShowAllLines_.Checked;
                Show();
            });
            flgShowAllLines_.Checked = false;

            flgShowWords_ = new MenuItem("Show words", (object obj, EventArgs args) => {
                flgShowWords_.Checked = !flgShowWords_.Checked;
                Show();
            });
            flgShowWords_.Checked = false;

            cm_ = new ContextMenu();
            cm_.MenuItems.Add("Show original image", (object obj, EventArgs args)=>{
                ShowOriginal();
            });
            cm_.MenuItems.Add("Show aligned image", (object obj, EventArgs args) =>{
                ShowAligned();
            });
            cm_.MenuItems.Add("Show filtered image", (object obj, EventArgs args) => {
                ShowFiltered();
            });
            cm_.MenuItems.Add("Show detector image", (object obj, EventArgs args) => {
                ShowMain();
            });
            cm_.MenuItems.Add("Show text image", (object obj, EventArgs args) => {
                ShowText();
            });
            cm_.MenuItems.Add("-");
            cm_.MenuItems.Add(flgShowSupportLines_);
            cm_.MenuItems.Add(flgShowAllLines_);
            cm_.MenuItems.Add(flgShowWords_);
            pbox_.ContextMenu = cm_;
            pbox_.SizeMode = PictureBoxSizeMode.Zoom;
            Show = new ShowDelegate(ShowOriginal);
        }

        private void drawOverlay_()
        {
            float factor =  pbox_.Image.Width / srcImage_.Width;
            using (Graphics g = Graphics.FromImage(pbox_.Image))
            {
                if (flgShowSupportLines_.Checked)
                {
                    Pen p = new Pen(Color.Red, 25.0f * factor);
                    for (uint k = 0; k < detector_.GetNumSupportLines(); k++)
                    {
                        System.ValueType[] fpts = detector_.GetSupportLine(k, 10);
                        Point[] pts = new Point[fpts.Length];
                        for (int i = 0; i < pts.Length; i++)
                        {
                            PointF fpt = (PointF)fpts[i];
                            pts[i].X = (int)(fpt.X * (pbox_.Image.Width - 1));
                            pts[i].Y = (int)(fpt.Y * (pbox_.Image.Height - 1));
                        }
                        //MessageBox.Show(pts[1].ToString());
                        g.DrawLines(p, pts);
                    }
                }
                if (flgShowAllLines_.Checked)
                {
                    Pen p = new Pen(Color.Green, 25.0f * factor);
                    for (uint k = 0; k < detector_.GetNumLines(); k++)
                    {
                        System.ValueType[] fpts = detector_.GetLine(k, 10);
                        Point[] pts = new Point[fpts.Length];
                        for (int i = 0; i < pts.Length; i++)
                        {
                            PointF fpt = (PointF)fpts[i];
                            pts[i].X = (int)(fpt.X * (pbox_.Image.Width - 1));
                            pts[i].Y = (int)(fpt.Y * (pbox_.Image.Height - 1));
                        }
                        g.DrawLines(p, pts);
                    }
                }
                if(flgShowWords_.Checked)
                {
                    SolidBrush []brushes = new SolidBrush[2];
                    brushes[0] = new SolidBrush(Color.FromArgb(50, Color.Yellow));
                    brushes[1] = new SolidBrush(Color.FromArgb(50, Color.DeepSkyBlue));
                    for (int i = 0; i < detector_.GetNumLines(); i++)
                        for (int j = 0; j < detector_.GetNumWords((uint)i); j++)
                            g.FillRectangle(brushes[i % 2],
                                detector_.GetWordBBox((uint)i, (uint)j, pbox_.Image.Size, 1.3f));
                }

            }
        }

        public void SelectObservation(string imgFileName)
        {
            fname_ = imgFileName;
            srcImage_ = null;
            if (File.Exists(fname_))
            {
                srcImage_ = Image.FromFile(fname_);
                foreach (var prop in srcImage_.PropertyItems)
                {
                    if (prop.Id == 0x112)
                    {
                        if (prop.Value[0] == 0x08)
                            srcImage_.RotateFlip(RotateFlipType.Rotate270FlipNone);
                        else if (prop.Value[0] == 0x03)
                            srcImage_.RotateFlip(RotateFlipType.Rotate180FlipNone);
                        else if (prop.Value[0] == 0x06)
                            srcImage_.RotateFlip(RotateFlipType.Rotate90FlipNone);
                        else if (prop.Value[0] != 0x01)
                            MessageBox.Show("Unkonwn image orientation!", "Warning!");
                    }
                }
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
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            pbox_.Image = srcImage_;
            Show = new ShowDelegate(ShowOriginal);
        }
        public void ShowAligned()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            pbox_.Image = rotateImage_((Bitmap)srcImage_, -detector_.GetAngle() * 180.0f / (float)Math.PI);
            drawOverlay_();
            Show = new ShowDelegate(ShowAligned);
        }
        public void ShowMain()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            pbox_.Image = detector_.GetMainImage();
            drawOverlay_();
            Show = new ShowDelegate(ShowMain);
        }
        public void ShowFiltered()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            pbox_.Image = detector_.GetFilteredImage();
            drawOverlay_();
            Show = new ShowDelegate(ShowFiltered);
        }
        public void ShowText()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            pbox_.Image = detector_.GetTextImage();
            drawOverlay_();
            Show = ShowText;
            Show = new ShowDelegate(ShowText);
        }
    }
}
