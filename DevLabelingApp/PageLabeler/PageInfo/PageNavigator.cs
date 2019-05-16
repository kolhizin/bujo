using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;

namespace PageLabeler.PageInfo
{
    class PageNavigator
    {
        private PageInfo curPage_;
        private LinkedListNode<LineInfo> lineIter_;
        private LinkedListNode<WordInfo> wordIter_;
        private Bitmap refImage_;

        public PageNavigator()
        {
            curPage_ = null;
        }

        public void SetPage(PageInfo page, Image refImage)
        {
            curPage_ = page;
            refImage_ = (Bitmap)refImage;
            lineIter_ = curPage_.lines.First;
            wordIter_ = null;
            if(lineIter_ != null)
                wordIter_ = lineIter_.Value.words.First;
        }

        public bool NextWord()
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var word = wordIter_.Next;
            var line = lineIter_;
            while(word == null)
            {
                line = line.Next;
                if (line == null)
                    return false;
                word = line.Value.words.First;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public bool PrevWord()
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var word = wordIter_.Previous;
            var line = lineIter_;
            while (word == null)
            {
                line = line.Previous;
                if (line == null)
                    return false;
                word = line.Value.words.Last;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public bool NextLine()
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var line = lineIter_.Next;
            var word = line.Value.words.First;
            
            while (word == null)
            {
                line = line.Next;
                if (line == null)
                    return false;
                word = line.Value.words.First;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public bool PrevLine()
        {
            if (wordIter_ == null || lineIter_ == null)
                return false;

            var line = lineIter_.Previous;
            var word = line.Value.words.Last;

            while (word == null)
            {
                line = line.Next;
                if (line == null)
                    return false;
                word = line.Value.words.First;
            }
            wordIter_ = word;
            lineIter_ = line;
            return true;
        }
        public Image GetWordImage()
        {
            if (wordIter_ == null)
                return null;
            float x = Math.Min(wordIter_.Value.x0, wordIter_.Value.x1);
            float y = Math.Min(wordIter_.Value.y0, wordIter_.Value.y1);
            float w = Math.Abs(wordIter_.Value.x1 - wordIter_.Value.x0);
            float h = Math.Abs(wordIter_.Value.y1 - wordIter_.Value.y0);
            var rect = new RectangleF(x * (refImage_.Width - 1), y * (refImage_.Height - 1),
                Math.Max(1, w * refImage_.Width), Math.Max(1, h * refImage_.Height));
            return refImage_.Clone(rect, refImage_.PixelFormat);
            // refImage_.Clone(rect, refImage_.PixelFormat);
        }
    }
}
