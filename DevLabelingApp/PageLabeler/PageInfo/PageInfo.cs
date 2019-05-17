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
        public string name;
        public LinkedList<LineInfo> lines;
        public List<PageError> errors;

        public PageInfo()
        {
            lines = new LinkedList<LineInfo>();
            errors = new List<PageError>();
            status = PageStatus.UNKNOWN;
        }
        public PageInfo(BuJoDetector.ManagedDetector detector, string path, string page_name)
        {
            errors = new List<PageError>();
            lines = new LinkedList<LineInfo>();
            name = page_name;

            string fpath = System.IO.Path.Combine(path, name);
            if (System.IO.Directory.Exists(fpath))
                System.IO.Directory.Delete(fpath, true);
            System.IO.Directory.CreateDirectory(fpath);
            for (uint i = 0; i < detector.GetNumLines(); i++)
                lines.AddLast(new LineInfo(detector, i, fpath));

            status = PageStatus.UNKNOWN;
            angle = detector.GetAngle();
        }

        public void ResetPath(string oldpath, string newpath, bool deleteOld = false)
        {
            if (oldpath == "")
                return;
            string oldpath_full = System.IO.Path.Combine(oldpath, name);
            string newpath_full = System.IO.Path.Combine(newpath, name);
            if (!System.IO.Directory.Exists(oldpath_full))
                return;

            if (System.IO.Directory.Exists(newpath_full))
                System.IO.Directory.Delete(newpath_full, true);
            System.IO.Directory.CreateDirectory(newpath_full);

            foreach (var line in lines)
                line.ResetPath(oldpath_full, newpath_full, deleteOld);

            if (deleteOld)
                System.IO.Directory.Delete(oldpath_full, true);
        }
    }
}
