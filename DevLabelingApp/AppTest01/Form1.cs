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

namespace AppTest01
{
    public partial class Form1 : Form
    {
        BuJoDetector.ManagedDetector managedDetector = new ManagedDetector();
        public Form1()
        {
            InitializeComponent();
        }

        private Bitmap RotateImage(Bitmap bmp, float angle)
        {
            float rad_angle = (float)(angle * Math.PI / 180.0f);
            float abs_cos = (float)(Math.Abs(Math.Cos(rad_angle)));
            float abs_sin = (float)(Math.Abs(Math.Sin(rad_angle)));
            float newW = abs_cos * bmp.Width + abs_sin * bmp.Height;
            float newH = abs_sin * bmp.Width + abs_cos * bmp.Height;
            Bitmap rotatedImage = new Bitmap((int)newW, (int)newH);
            using (Graphics g = Graphics.FromImage(rotatedImage))
            {
                g.TranslateTransform(newW / 2, newH / 2);
                g.RotateTransform(angle);
                g.TranslateTransform(-bmp.Width / 2, -bmp.Height / 2);
                g.DrawImage(bmp, new Point(0, 0));
            }

            return rotatedImage;
        }

        private void Button1_Click(object sender, EventArgs e)
        {
            System.Diagnostics.Stopwatch sw = System.Diagnostics.Stopwatch.StartNew();
            Image img = System.Drawing.Image.FromFile("D:/Data/bujo_sample/20190309_125158.jpg");

            Bitmap bmp = new Bitmap(img, new Size(img.Width/5, img.Height/5));
            managedDetector.LoadImage(bmp);

            pictureBox1.Image = RotateImage(bmp, -managedDetector.GetAngle() * 180.0f / (float)Math.PI);
            pictureBox1.SizeMode = PictureBoxSizeMode.Zoom;

            using (Graphics g = Graphics.FromImage(pictureBox1.Image))
            {
                Color customColor = Color.FromArgb(50, Color.Black);
                SolidBrush shadowBrush = new SolidBrush(customColor);
                for(int i = 0; i < managedDetector.GetNumLines(); i++)
                    for(int j = 0; j < managedDetector.GetNumWords((uint)i); j++)
                        g.FillRectangle(shadowBrush,
                            managedDetector.GetWordBBox((uint)i, (uint)j, pictureBox1.Image.Size, 1.3f));
            }

            MessageBox.Show("BuJoLoad: " + managedDetector.GetTimeLoad().ToString()
                + "; BuJoCompute: " + managedDetector.GetTimeCompute().ToString()
                + "; Total: " + sw.ElapsedMilliseconds.ToString());
            bmp.Dispose();
        }
    }
}
