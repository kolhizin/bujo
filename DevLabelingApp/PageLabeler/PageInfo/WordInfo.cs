using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using BuJoDetector;

namespace PageLabeler
{
    class WordInfo
    {
        public enum WordStatus { UNKNOWN, CORRECT, INCORRECT };
        public string text; //transcription of image
        public string comment; //any kind of comment
        public string filename;
        public uint wordId, lineId; //id of word
        public float neg_offset, pos_offset; //relative coordinated
        public float [] xcoords;
        public float [] ycoords;
        public WordStatus status;

        public WordInfo()
        {
            text = "";
            comment = "";
            filename = "";
            wordId = 0;
            lineId = 0;
            status = WordStatus.UNKNOWN;
        }

        private Image ExtractWord(System.Drawing.Imaging.BitmapData bmpData,
            int alignedImageWidth, int alignedImageHeight, float yScale)
        {
            float minx = xcoords[0], maxx = xcoords[xcoords.Length-1];
            for (int i = 0; i < xcoords.Length; i++)
            {
                minx = Math.Min(xcoords[i], minx);
                maxx = Math.Max(xcoords[i], maxx);
            }
            int iminx = (int)Math.Floor(minx * alignedImageWidth);
            int imaxx = (int)Math.Ceiling(maxx * alignedImageWidth);
            int w = imaxx - iminx;
            int h = Math.Max(1, (int)Math.Ceiling((neg_offset + pos_offset) * yScale * alignedImageHeight));
            Bitmap res = new Bitmap(w, h, bmpData.PixelFormat);
            var r = res.LockBits(new Rectangle(0, 0, w, h), System.Drawing.Imaging.ImageLockMode.WriteOnly,
                bmpData.PixelFormat);

            float[] xs = new float[w];
            float[] ys = new float[w];

            for(int i = 0; i < w; i++)
            {
                if(i == w-1)
                {
                    xs[i] = xcoords[xcoords.Length - 1];
                    ys[i] = ycoords[ycoords.Length - 1];
                    continue;
                }
                float rel = (float)i / (float)(w - 1);
                int id = (int)Math.Floor(rel * (xcoords.Length - 1));
                float frac = rel * (xcoords.Length - 1) - (float)id;
                xs[i] = xcoords[id] * (1 - frac) + xcoords[id + 1] * frac;
                ys[i] = ycoords[id] * (1 - frac) + ycoords[id + 1] * frac;
            }

            int pixelSize = 1;
            if (bmpData.PixelFormat == System.Drawing.Imaging.PixelFormat.Format24bppRgb)
                pixelSize = 3;
            else if (bmpData.PixelFormat == System.Drawing.Imaging.PixelFormat.Format32bppArgb ||
                bmpData.PixelFormat == System.Drawing.Imaging.PixelFormat.Format32bppRgb)
                pixelSize = 4;
            else throw new ArgumentException("Unexpected pixel format!");

            byte []buff = new byte[16];

            for(int i = 0; i < h; i++)
            {
                var rrow = r.Scan0 + r.Stride * i;
                float row_off = (neg_offset + pos_offset) * (float)i/(float)(h-1) - neg_offset;
                for(int j = 0; j < w; j++)
                {
                    float x = xs[j] * (alignedImageWidth-1);
                    float y = (ys[j] + row_off * yScale) * (alignedImageHeight - 1);
                    int fj = (int)Math.Max(0, Math.Min(alignedImageWidth - 1, (int)Math.Round(x)));
                    int fi = (int)Math.Max(0, Math.Min(alignedImageHeight - 1, (int)Math.Round(y)));

                    var dst = rrow + pixelSize * j;
                    var src = (bmpData.Scan0 + fi * bmpData.Stride) + fj * pixelSize;
                    System.Runtime.InteropServices.Marshal.Copy(src, buff, 0, pixelSize);
                    System.Runtime.InteropServices.Marshal.Copy(buff, 0, dst, pixelSize);
                }
            }
            res.UnlockBits(r);
            return res;
        }

        public WordInfo(BuJoDetector.ManagedDetector detector, uint lineId, uint wordId, string path,
            System.Drawing.Imaging.BitmapData alignedImageBMPData, int alignedImageWidth, int alignedImageHeight)
        {
            this.wordId = wordId;
            this.lineId = lineId;
            filename = "line" + lineId.ToString("D2") + "_word" + wordId.ToString("D2") + ".jpg";
            string fname = System.IO.Path.Combine(path, filename);

            System.ValueType []tmpXY = detector.GetWord(lineId, wordId, 10);
            xcoords = new float[tmpXY.Length];
            ycoords = new float[tmpXY.Length];
            for(int i = 0; i < tmpXY.Length; i++)
            {
                xcoords[i] = ((PointF)tmpXY[i]).X;
                ycoords[i] = ((PointF)tmpXY[i]).Y;
            }
            pos_offset = detector.GetWordPosOffset(lineId, wordId);
            neg_offset = -detector.GetWordNegOffset(lineId, wordId);
            text = "";
            comment = "";

            status = WordStatus.UNKNOWN;

            var tmp = ExtractWord(alignedImageBMPData, alignedImageWidth, alignedImageHeight, 1.3f);
            tmp.Save(fname);
            tmp.Dispose();
        }
        public void ResetPath(string oldpath, string newpath, bool deleteOld = false)
        {
            string oldname = System.IO.Path.Combine(oldpath, filename);
            string newname = System.IO.Path.Combine(newpath, filename);
            if (System.IO.File.Exists(oldname))
            {
                System.IO.File.Copy(oldname, newname);
                if (deleteOld)
                    System.IO.File.Delete(oldname);
            }
        }
        public void SetCorrect(string text)
        {
            this.text = text;
            this.status = WordStatus.CORRECT;
        }
        public void SetIncorrect(string comment = "")
        {
            this.comment = comment;
            this.text = "";
            this.status = WordStatus.INCORRECT;
        }
        public RectangleF GetBBox(float yScale)
        {
            float ymin = Math.Min(ycoords[0], ycoords[ycoords.Length - 1]);
            float ymax = Math.Max(ycoords[0], ycoords[ycoords.Length - 1]);
            float xmin = Math.Min(xcoords[0], xcoords[xcoords.Length - 1]);
            float xmax = Math.Max(xcoords[0], xcoords[xcoords.Length - 1]);
            return new RectangleF(xmin, ymin - neg_offset * yScale,
                xmax - xmin, ymax - ymin + (pos_offset + neg_offset) * yScale);
        }
    }
}
