using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PageLabeler.PageInfo
{
    class LineInfo
    {
        public enum LineStatus { UNKNOWN, CORRECT, INCORRECT };
        public string comment; //any kind of comment
        public uint lineId; //id of word
        public LineStatus status;
        public LinkedList<WordInfo> words;
    }
}
