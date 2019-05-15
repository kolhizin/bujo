using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using BuJoDetector;

namespace PageLabeler.PageInfo
{
    class LineInfo
    {
        public enum LineStatus { UNKNOWN, CORRECT, INCORRECT };
        public string comment; //any kind of comment
        public uint lineId; //id of word
        public LineStatus status;
        public LinkedList<WordInfo> words;

        public LineInfo()
        {
            words = new LinkedList<WordInfo>();
        }

        public LineInfo(BuJoDetector.ManagedDetector detector, uint lineId)
        {
            this.lineId = lineId;
            words = new LinkedList<WordInfo>();
            for (uint i = 0; i < detector.GetNumWords(lineId); i++)
                words.AddLast(new WordInfo(detector, lineId, i));
        }
    }
}
