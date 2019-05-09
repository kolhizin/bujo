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

        private void Button1_Click(object sender, EventArgs e)
        {
            Image img = System.Drawing.Image.FromFile("D:/Data/bujo_sample/20190309_125139.jpg");
            Bitmap bmp = new Bitmap(img);
            managedDetector.LoadImage(bmp);
            uint res = managedDetector.GetTextLineDelta();
            MessageBox.Show("Test " + res.ToString());
        }
    }
}
