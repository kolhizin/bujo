using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

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
    }
}
