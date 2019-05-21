using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using BuJoDetector;

namespace PageLabeler.PageInfo
{
    public class PageError
    {
        public enum ErrorType { MISSING_WORD, MISSING_LINE, MISSING_REGION, OTHER };
        public ErrorType type;
        public string description;
        public float x, y;

        public static ErrorType CycleError(ErrorType err)
        {
            switch(err)
            {
                case ErrorType.OTHER: return ErrorType.MISSING_WORD;
                case ErrorType.MISSING_WORD: return ErrorType.MISSING_LINE;
                case ErrorType.MISSING_LINE: return ErrorType.MISSING_REGION;
                case ErrorType.MISSING_REGION: return ErrorType.OTHER;
            }
            return ErrorType.OTHER;
        }
    }
    class PageInfo
    {
        public enum PageStatus { UNKNOWN, EXCLUDE, SUCCESS, FAIL };
        public PageStatus status;
        public float angle;
        public string name;
        public LinkedList<LineInfo> lines;
        public LinkedList<PageError> errors;

        public PageInfo()
        {
            lines = new LinkedList<LineInfo>();
            errors = new LinkedList<PageError>();
            status = PageStatus.UNKNOWN;
        }
        public PageInfo(BuJoDetector.ManagedDetector detector, string path, string page_name,
            Image alignedImage)
        {
            errors = new LinkedList<PageError>();
            lines = new LinkedList<LineInfo>();
            name = page_name;

            string fpath = System.IO.Path.Combine(path, name);
            if (System.IO.Directory.Exists(fpath))
                System.IO.Directory.Delete(fpath, true);
            System.IO.Directory.CreateDirectory(fpath);

            var bmp = new Bitmap(alignedImage);
            var bmpData = bmp.LockBits(new Rectangle(0, 0, alignedImage.Width, alignedImage.Height),
                System.Drawing.Imaging.ImageLockMode.ReadOnly, bmp.PixelFormat);

            for (uint i = 0; i < detector.GetNumLines(); i++)
                lines.AddLast(new LineInfo(detector, i, fpath, bmpData,
                    alignedImage.Width, alignedImage.Height));

            bmp.UnlockBits(bmpData);
            bmp.Dispose();

            status = PageStatus.UNKNOWN;
            angle = detector.GetAngle();

            detector.GetAlignedImage().Save(System.IO.Path.Combine(fpath, "det_aligned.png"));
            detector.GetFilteredImage().Save(System.IO.Path.Combine(fpath, "det_filtered.png"));
            detector.GetMainImage().Save(System.IO.Path.Combine(fpath, "det_main.png"));
            detector.GetTextImage().Save(System.IO.Path.Combine(fpath, "det_text.png"));
        }
        private Image GetDetectorImage_(string path, string fname)
        {
            if (name == "")
                return null;
            return Image.FromFile(System.IO.Path.Combine(path, name, fname));
        }

        public Image GetDetectorAlignedImage(string path)
        {
            return GetDetectorImage_(path, "det_aligned.png");
        }
        public Image GetDetectorFilteredImage(string path)
        {
            return GetDetectorImage_(path, "det_filtered.png");
        }
        public Image GetDetectorMainImage(string path)
        {
            return GetDetectorImage_(path, "det_main.png");
        }
        public Image GetDetectorTextImage(string path)
        {
            return GetDetectorImage_(path, "det_text.png");
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

            System.IO.File.Move(System.IO.Path.Combine(oldpath_full, "det_aligned.png"),
                System.IO.Path.Combine(newpath_full, "det_aligned.png"));
            System.IO.File.Move(System.IO.Path.Combine(oldpath_full, "det_filtered.png"),
                System.IO.Path.Combine(newpath_full, "det_filtered.png"));
            System.IO.File.Move(System.IO.Path.Combine(oldpath_full, "det_main.png"),
                System.IO.Path.Combine(newpath_full, "det_main.png"));
            System.IO.File.Move(System.IO.Path.Combine(oldpath_full, "det_text.png"),
                System.IO.Path.Combine(newpath_full, "det_text.png"));

            if (deleteOld)
                System.IO.Directory.Delete(oldpath_full, true);
        }
    }
}
