using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using BuJoDetector;

namespace PageLabeler.PageInfo
{
    public class PageError
    {
        public enum ErrorType { MISSING_WORD, MISSING_LINE, OTHER };
        public ErrorType type;
        public string description;
        public float x, y;
    }
    class PageInfo
    {
        public enum PageStatus { UNKNOWN, EXCLUDE, SUCCESS, FAIL };
        public PageStatus status;
        public float angle;
        public LinkedList<LineInfo> lines;
        public List<PageError> errors;

        public PageInfo()
        {
            lines = new LinkedList<LineInfo>();
            errors = new List<PageError>();
            status = PageStatus.UNKNOWN;
        }
        public PageInfo(BuJoDetector.ManagedDetector detector)
        {
            errors = new List<PageError>();
            lines = new LinkedList<LineInfo>();
            for (uint i = 0; i < detector.GetNumLines(); i++)
                lines.AddLast(new LineInfo(detector, i));
            status = PageStatus.UNKNOWN;
            angle = detector.GetAngle();
        }
    }
}
