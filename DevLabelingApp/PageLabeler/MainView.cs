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
        private PageInfo.PageNavigator navigator_;
        private MenuItem flgShowSupportLines_, flgShowAllLines_, flgShowWords_, flgShowStatus_;
        private ContextMenu cm_;
        private Image srcImage_, rotImage_;
        private Image rawImage_;
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

        public MainView(PictureBox pbox, ManagedDetector detector, PageInfo.PageNavigator navigator)
        {
            fname_ = "";
            pbox_ = pbox;
            detector_ = detector;
            navigator_ = navigator;

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

            flgShowStatus_ = new MenuItem("Show status", (object obj, EventArgs args) => {
                flgShowStatus_.Checked = !flgShowStatus_.Checked;
                Show();
            });

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
            cm_.MenuItems.Add("-");
            cm_.MenuItems.Add(flgShowStatus_);
            pbox_.ContextMenu = cm_;
            pbox_.SizeMode = PictureBoxSizeMode.Zoom;
            Show = new ShowDelegate(ShowOriginal);
        }

        private void drawOverlay_()
        {
            if(pbox_.Image != rawImage_)
                pbox_.Image.Dispose();
            pbox_.Image = (Image)rawImage_.Clone();
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
                if(flgShowStatus_.Checked)
                {
                    SolidBrush bUnk = new SolidBrush(Color.FromArgb(100, Color.Gray));
                    SolidBrush bCor = new SolidBrush(Color.FromArgb(100, Color.Green));
                    SolidBrush bIncor = new SolidBrush(Color.FromArgb(100, Color.Red));
                    SolidBrush bSel = new SolidBrush(Color.FromArgb(100, Color.Blue));
                    SolidBrush bErr = new SolidBrush(Color.FromArgb(200, Color.Yellow));
                    for (int i = 0; i < navigator_.NumLines(); i++)
                        for(int j = 0; j < navigator_.NumWords(i); j++)
                        {
                            SolidBrush used = bUnk;
                            var status = navigator_.GetWordStatus(i, j);
                            var relrect = navigator_.GetWordBBox(i, j, 1.3f);
                            if (navigator_.IsSelected(i, j))
                                used = bSel;
                            else
                            {
                                if (status == WordInfo.WordStatus.CORRECT)
                                    used = bCor;
                                else if (status == WordInfo.WordStatus.INCORRECT)
                                    used = bIncor;
                            }
                            g.FillRectangle(used, new RectangleF(relrect.X*rawImage_.Width, relrect.Y *rawImage_.Height,
                                relrect.Width * rawImage_.Width, relrect.Height * rawImage_.Height));
                        }
                    for(int i=0; i < navigator_.NumErrors(); i++)
                    {
                        Brush b = bErr;
                        var err = navigator_.GetError(i);
                        if (err == navigator_.GetError())
                            b = bSel;
                        float size = 0.02f * Math.Max(rawImage_.Width, rawImage_.Height);

                        g.FillEllipse(b,
                            err.x * rawImage_.Width - size * 0.5f,
                            err.y * rawImage_.Height - size * 0.5f,
                            size, size);
                    }
                }
            }
        }

        public void SelectObservation(string imgFileName, float angle)
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
                rotImage_ = rotateImage_((Bitmap)srcImage_, -angle * 180.0f / (float)Math.PI);
                rawImage_ = srcImage_;
                pbox_.Image = srcImage_;
            }
            
        }
        public void UpdateAngle()
        {
            rotImage_ = rotateImage_((Bitmap)srcImage_, -detector_.GetAngle() * 180.0f / (float)Math.PI);
        }
        public string GetSelected() { return fname_; }
        public bool ObservationExists() { return File.Exists(fname_); }
        public Image GetOriginalImage() { return srcImage_; }
        public Image GetAlignedImage() { return rotImage_; }
        public void ShowOriginal()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            rawImage_ = srcImage_;
            Show = new ShowDelegate(ShowOriginal);
        }
        public void ShowAligned()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            rawImage_ = rotImage_;
            drawOverlay_();
            Show = new ShowDelegate(ShowAligned);
        }
        public void ShowMain()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            rawImage_ = detector_.GetMainImage();
            drawOverlay_();
            Show = new ShowDelegate(ShowMain);
        }
        public void ShowFiltered()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            rawImage_ = detector_.GetFilteredImage();
            drawOverlay_();
            Show = new ShowDelegate(ShowFiltered);
        }
        public void ShowText()
        {
            if (!File.Exists(fname_))
                return;
            if (pbox_.Image != srcImage_)
                pbox_.Image.Dispose();
            rawImage_ = detector_.GetTextImage();
            drawOverlay_();
            Show = ShowText;
            Show = new ShowDelegate(ShowText);
        }

        public void UpdateOverlay()
        {
            drawOverlay_();
        }

    }
}
