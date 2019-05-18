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

        public WordInfo(BuJoDetector.ManagedDetector detector, uint lineId, uint wordId, string path)
        {
            this.wordId = wordId;
            this.lineId = lineId;
            filename = "line" + lineId.ToString("D2") + "_word" + wordId.ToString("D2") + ".jpg";
            string fname = System.IO.Path.Combine(path, filename);

            Image tmpImg = detector.GetWordImage(lineId, wordId, 1.3f);
            tmpImg.Save(fname);

            System.ValueType []tmpXY = detector.GetWord(lineId, wordId, 10);
            xcoords = new float[tmpXY.Length];
            ycoords = new float[tmpXY.Length];
            for(int i = 0; i < tmpXY.Length; i++)
            {
                xcoords[i] = ((PointF)tmpXY[i]).X;
                ycoords[i] = ((PointF)tmpXY[i]).Y;
            }
            pos_offset = detector.GetWordPosOffset(lineId, wordId);
            neg_offset = detector.GetWordPosOffset(lineId, wordId);
            text = "";
            comment = "";

            status = WordStatus.UNKNOWN;
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
