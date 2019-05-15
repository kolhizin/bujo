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
        public uint wordId; //id of word
        public float x0, y0, x1, y1; //relative coordinated
        public WordStatus status;

        public WordInfo()
        {
            text = "";
            comment = "";
            wordId = 0;
            x0 = y0 = x1 = y1 = 0.0f;
            status = WordStatus.UNKNOWN;
        }

        public WordInfo(BuJoDetector.ManagedDetector detector, uint lineId, uint wordId)
        {
            this.wordId = wordId;
            System.ValueType[] fpts = detector.GetWord(lineId, wordId, 2);
            x0 = ((PointF)fpts[0]).X;
            y0 = ((PointF)fpts[0]).Y;
            x1 = ((PointF)fpts[1]).X;
            y1 = ((PointF)fpts[1]).Y;

            text = "";
            comment = "";

            status = WordStatus.UNKNOWN;
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
    }
}
