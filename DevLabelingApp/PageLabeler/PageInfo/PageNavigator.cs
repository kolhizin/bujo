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
        public Image GetWordImage(string path)
        {
            if (wordIter_ == null)
                return null;
            string fname = System.IO.Path.Combine(path, curPage_.name, wordIter_.Value.filename);
            return Image.FromFile(fname);
        }
        public WordInfo.WordStatus GetWordStatus()
        {
            if (wordIter_ == null)
                return WordInfo.WordStatus.UNKNOWN;
            return wordIter_.Value.status;
        }
        public void SetWordStatus(WordInfo.WordStatus status)
        {
            if (wordIter_ == null)
                return;
            wordIter_.Value.status = status;
        }
        public string GetWordText()
        {
            if (wordIter_ == null)
                return "";
            return wordIter_.Value.text;
        }
        public void SetWordText(string str)
        {
            if (wordIter_ == null)
                return;
            wordIter_.Value.text = str;
        }
        public string GetWordComment()
        {
            if (wordIter_ == null)
                return "";
            return wordIter_.Value.comment;
        }
        public void SetWordComment(string str)
        {
            if (wordIter_ == null)
                return;
            wordIter_.Value.comment = str;
        }
    }
}
